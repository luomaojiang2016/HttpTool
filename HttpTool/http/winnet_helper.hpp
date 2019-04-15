/*
* Author: LowBoyTeam (https://github.com/LowBoyTeam)
* License: Code Project Open License
* Disclaimer: The software is provided "as-is". No claim of suitability, guarantee, or any warranty whatsoever is provided.
* Copyright (c) 2016-2017.
*/

#ifndef _WINNET_HPP_INCLUDED_
#define _WINNET_HPP_INCLUDED_

#include <string>
#include <tchar.h>
#include <vector>
#include <map>
#include <regex>
#include <assert.h>

#include <windows.h>
#include <strsafe.h>

#include <wininet.h>
#pragma comment(lib,"Wininet.lib")

namespace winnet
{
	/// <summary>
	/// Common HTTP methods.
	/// </summary>		
	const std::string HTTP_METHOD_GET		= "GET";
	const std::string HTTP_METHOD_POST		= "POST";
	const std::string HTTP_METHOD_PUT		= "PUT";
	const std::string HTTP_METHOD_HEAD		= "HEAD";
	const std::string HTTP_METHOD_DELETE	= "DELETE";

	typedef std::map<std::string, std::string> HeaderFields;
	typedef std::map<std::string, std::string> Cookies;

	class CWinNet
	{
	private:

		HINTERNET			m_hSession;
		HINTERNET			m_hConnect;
		HINTERNET			m_hRequest;

		std::string			m_strHost;
		std::string			m_strPath;
		std::string			m_strExt;

		INTERNET_SCHEME		m_nScheme;
		INTERNET_PORT		m_nPort;

		BOOL				m_followRedirects;
		HeaderFields		m_RequestHeaders;
		std::string			m_request_method;

		std::string			m_proxy_info;
		std::string			m_proxy_user;
		std::string			m_proxy_pass;

		std::string			m_basic_auth_user;
		std::string			m_basic_auth_pass;

		Cookies ParseCookies(const std::string& cookies)
		{
			Cookies result;
			
			std::string cookies_pattern("([\\S]+?)=([^;|^\\r|^\\n]+)");
			std::regex cookies_regex(cookies_pattern);
			std::smatch results;

			auto cookies_begin = std::sregex_iterator(cookies.begin(), cookies.end(), cookies_regex);
			auto cookies_end = std::sregex_iterator();

			for (auto i = cookies_begin; i != cookies_end; ++i)
			{
				std::smatch match = *i;
				if (match.size() == 3)
					result[match[1].str()] = match[2].str();
			}

			return result;
		}

	public:
		
		CWinNet(void) : m_hSession(NULL)
			, m_hConnect(NULL)
			, m_hRequest(NULL)
			, m_followRedirects(FALSE)
		{

		}

		~CWinNet()
		{
			if (m_hRequest)
			{
				InternetCloseHandle(m_hRequest);
				m_hRequest = NULL;
			}
			if (m_hConnect)
			{
				InternetCloseHandle(m_hConnect);
				m_hConnect = NULL;
			}
			if (m_hSession)
			{
				InternetCloseHandle(m_hSession);
				m_hSession = NULL;
			}
		}

		BOOL Open(const std::string& Url, const std::string& method)
		{
			m_hSession = InternetOpenA("winnet http client v1.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
			
			if (!m_hSession)
			{
				printf("Error %u in InternetOpenA.\n", GetLastError());
				return FALSE;
			}

			URL_COMPONENTSA urlComp;
			ZeroMemory(&urlComp, sizeof(urlComp));
			urlComp.dwStructSize			= sizeof(urlComp);
			urlComp.dwHostNameLength		= 1;
			urlComp.dwUserNameLength		= 1;
			urlComp.dwPasswordLength		= 1;
			urlComp.dwUrlPathLength			= 1;
			urlComp.dwExtraInfoLength		= 1;

			if (!InternetCrackUrlA(Url.c_str(), 0, 0, &urlComp))
			{
				printf("Error %u in InternetCrackUrlA.\n", GetLastError());
				return FALSE;
			}

			m_strHost = std::string(urlComp.lpszHostName, urlComp.dwHostNameLength);
			m_strPath = std::string(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
			m_strExt = std::string(urlComp.lpszExtraInfo, urlComp.dwExtraInfoLength);
			m_nScheme = urlComp.nScheme;
			m_nPort = urlComp.nPort;
			m_strPath.append(m_strExt);
			m_request_method = method;

			return TRUE;
		}

		BOOL SetTimeout(DWORD dwTimeOut)
		{
			assert(m_hSession != NULL);

			if (InternetSetOptionA(m_hSession, INTERNET_OPTION_CONNECT_TIMEOUT, &dwTimeOut, sizeof(dwTimeOut))
				&& InternetSetOptionA(m_hSession, INTERNET_OPTION_SEND_TIMEOUT, &dwTimeOut, sizeof(dwTimeOut))
				&& InternetSetOptionA(m_hSession, INTERNET_OPTION_RECEIVE_TIMEOUT, &dwTimeOut, sizeof(dwTimeOut)))
			{
				return TRUE;
			}
			return FALSE;
		}

		void SetRequestHeader(const std::string& key, const std::string& value)
		{
			m_RequestHeaders[key] = value;
		}

		void SetHeaders(HeaderFields headers)
		{
			m_RequestHeaders = headers;
		}

		void SetUserAgent(const std::string& userAgent) {
			SetRequestHeader("User-Agent", userAgent);
		}

		void SetCookie(const std::string& cookies)
		{
			SetRequestHeader("Cookie", cookies);
		}

		void SetReferer(const std::string& referer)
		{
			SetRequestHeader("Referer", referer);
		}

		void FollowRedirects(BOOL follow)
		{
			m_followRedirects = follow;
		}

		BOOL Send(LPVOID lpPostData = NULL, DWORD dwsize = 0)
		{
			assert(m_hSession != NULL);

			m_hConnect = InternetConnectA(m_hSession, m_strHost.c_str(), m_nPort, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);

			if (!m_hConnect)
			{
				printf("Error %u in InternetConnectA.\n", GetLastError());
				return FALSE;
			}

			if (!m_basic_auth_user.empty() && !m_basic_auth_pass.empty())
			{
				if (!InternetSetOptionA(m_hConnect, INTERNET_OPTION_USERNAME, (LPVOID)m_basic_auth_user.c_str(), m_basic_auth_user.size() + 1))
				{
					printf("Error %u in InternetSetOptionA. INTERNET_OPTION_USERNAME \n", GetLastError());
					return FALSE;
				}
				if (!InternetSetOptionA(m_hConnect, INTERNET_OPTION_PASSWORD, (LPVOID)m_basic_auth_pass.c_str(), m_basic_auth_pass.size() + 1))
				{
					printf("Error %u in InternetSetOptionA. INTERNET_OPTION_PASSWORD \n", GetLastError());
					return FALSE;
				}
			}

			if (!m_proxy_info.empty())
			{
				INTERNET_PROXY_INFO proxyInfo;
				proxyInfo.dwAccessType = INTERNET_OPEN_TYPE_PROXY;
				proxyInfo.lpszProxy = reinterpret_cast<LPCTSTR>(m_proxy_info.c_str());
				proxyInfo.lpszProxyBypass = NULL;

				if (!InternetSetOptionA(m_hSession, INTERNET_OPTION_PROXY, (LPVOID)&proxyInfo, sizeof(proxyInfo)))
				{
					printf("Error %u in InternetSetOptionA. INTERNET_OPTION_PROXY \n", GetLastError());
					return FALSE;
				}

				if (!m_proxy_user.empty() && !m_proxy_pass.empty())
				{
					if (!InternetSetOptionA(m_hConnect, INTERNET_OPTION_PROXY_USERNAME, (LPVOID)m_proxy_user.c_str(), m_proxy_user.size() + 1))
					{
						printf("Error %u in InternetSetOptionA. INTERNET_OPTION_PROXY_USERNAME \n", GetLastError());
						return FALSE;
					}
					if (!InternetSetOptionA(m_hConnect, INTERNET_OPTION_PROXY_PASSWORD, (LPVOID)m_proxy_pass.c_str(), m_proxy_pass.size() + 1))
					{
						printf("Error %u in InternetSetOptionA. INTERNET_OPTION_PROXY_PASSWORD \n", GetLastError());
						return FALSE;
					}
				}
			}

			DWORD dwFlags = INTERNET_FLAG_HYPERLINK
				| INTERNET_FLAG_KEEP_CONNECTION//Uses keep-alive semantics, if available, for the connection.
				| INTERNET_FLAG_NO_UI//Disables the cookie dialog box.
				| INTERNET_FLAG_RESYNCHRONIZE//Reloads HTTP resources if the resource has been modified since the last time it was downloaded.
				| INTERNET_FLAG_NO_CACHE_WRITE//Does not add the returned entity to the cache.
				| INTERNET_FLAG_PRAGMA_NOCACHE//Forces the request to be resolved by the origin server, even if a cached copy exists on the proxy. 
				| INTERNET_FLAG_RELOAD//Forces a download of the requested file, object, or directory listing from the origin server, not from the cache. 
				| INTERNET_COOKIE_THIRD_PARTY//Indicates that a third-party cookie is being set or retrieved.
				| INTERNET_FLAG_NO_COOKIES//Does not use local cookies.
				| INTERNET_FLAG_IGNORE_CERT_CN_INVALID//Disable checking of SSL certificates errors.
				| INTERNET_FLAG_IGNORE_CERT_DATE_INVALID//Disable checking of SSL certificates errors.
				| INTERNET_FLAG_DONT_CACHE;

			if (m_nScheme == INTERNET_SCHEME_HTTPS)
				dwFlags = dwFlags | INTERNET_FLAG_SECURE;//SSL
			else
				dwFlags = dwFlags | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS;

			if (m_followRedirects == FALSE)
				dwFlags = dwFlags | INTERNET_FLAG_NO_AUTO_REDIRECT;//Does not automatically handle redirection in HttpSendRequest.

			m_hRequest = HttpOpenRequestA(m_hConnect, m_request_method.c_str(), m_strPath.c_str(), "HTTP/1.1", NULL, NULL, dwFlags, NULL);

			if (!m_hRequest)
			{
				printf("Error %u in HttpOpenRequestA.\n", GetLastError());
				return FALSE;
			}

			//Enables WinINet to perform decoding for the gzip and deflate encoding schemes. For more information
			DWORD dwGzip = 1;
			InternetSetOptionW(m_hRequest, INTERNET_OPTION_HTTP_DECODING, &dwGzip, sizeof(dwGzip));

			std::string strHeaders;
			for (auto &k : m_RequestHeaders)
				strHeaders.append(k.first).append(": ").append(k.second).append("\n");

			if (!HttpAddRequestHeadersA(m_hRequest, strHeaders.c_str(), -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE))
			{
				printf("Error %u in HttpAddRequestHeadersA.\n", GetLastError());
				return FALSE;
			}

			m_RequestHeaders.clear();

			if (!HttpSendRequestA(m_hRequest, NULL, 0, lpPostData, dwsize))
			{
				printf("Error %u in HttpSendRequestA.\n", GetLastError());
				return FALSE;
			}
			return TRUE;
		}

		std::vector<BYTE> GetResponseBody()
		{
			assert(m_hRequest != NULL);

			DWORD dwReadSize = 0;
			DWORD dwDownloaded = 0;

			std::vector<BYTE> list;

			do
			{
				// Check for available data.
				dwReadSize = 0;
				if (!InternetQueryDataAvailable(m_hRequest, &dwReadSize, 0, 0))
				{
					printf("Error %u in InternetQueryDataAvailable.\n", GetLastError());
					break;
				}

				// No more available data.
				if (!dwReadSize)
					break;

				// Allocate space for the buffer.
				//pszOutBuffer = new char[dwSize + 1];
				BYTE* lpReceivedData = new BYTE[dwReadSize];
				if (!lpReceivedData)
				{
					printf("Out of memory\n");
					break;
				}
				else
				{
					// Read the Data.
					ZeroMemory(lpReceivedData, dwReadSize);

					if (!InternetReadFile(m_hRequest, lpReceivedData, dwReadSize, &dwDownloaded)) {
						printf("Error %u in InternetReadFile.\n", GetLastError());
					}
					else {
						for (size_t i = 0; i < dwReadSize; i++)
							list.push_back(lpReceivedData[i]);
					}

					// Free the memory allocated to the buffer.
					delete[] lpReceivedData;

					// This condition should never be reached since WinHttpQueryDataAvailable
					// reported that there are bits to read.
					if (!dwDownloaded)
						break;
				}
			} while (dwReadSize > 0);
			return list;
		}

		std::string GetResponseHeaders()
		{
			return GetResponseHeaderValue(HTTP_QUERY_RAW_HEADERS_CRLF);
		}

		std::string GetResponseHeaderValue(const std::string& name)
		{
			assert(m_hRequest != NULL);

			std::string	result;

			DWORD			dwSize			= name.length() + 1;
			LPVOID			lpOutBuffer		= new char[dwSize];

			StringCchPrintfA((LPSTR)lpOutBuffer, dwSize, name.c_str());

		retry:

			if (!HttpQueryInfoA(m_hRequest, HTTP_QUERY_CUSTOM, (LPVOID)lpOutBuffer, &dwSize, NULL))
			{
				// Check for an insufficient buffer.
				if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				{
					// Allocate the necessary buffer.
					delete[] lpOutBuffer;
					lpOutBuffer = NULL;

					lpOutBuffer = new char[dwSize];

					// Rewrite the header name in the buffer.
					StringCchPrintfA((LPSTR)lpOutBuffer, dwSize, name.c_str());

					// Retry the call.
					goto retry;
				}
				else
				{
					// Error handling code.
					delete[] lpOutBuffer;
					lpOutBuffer = NULL;
				}
			}

			if (lpOutBuffer)
				result.assign(reinterpret_cast<LPCSTR>(lpOutBuffer));

			return result;
		}

		std::string GetResponseHeaderValue(int dwInfoLevel, DWORD dwIndex = NULL)
		{
			assert(m_hRequest != NULL);

			std::string		result;

			LPVOID			lpOutBuffer = NULL;
			DWORD			dwSize = 0;

		retry:

			if (!HttpQueryInfoA(m_hRequest, dwInfoLevel, (LPVOID)lpOutBuffer, &dwSize, &dwIndex))
			{
				if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				{
					// Allocate the necessary buffer.
					lpOutBuffer = new char[dwSize];
					// Retry the call.
					goto retry;
				}
			}

			if (lpOutBuffer)
				result.assign(reinterpret_cast<LPCSTR>(lpOutBuffer));

			if (lpOutBuffer)
				delete[] lpOutBuffer;

			return result;
		}

		HeaderFields GetHeaderFields()
		{
			std::string strHeaders = GetResponseHeaders();
			
			HeaderFields Headers;

			std::string pattern("(.+?): ([^;|^\\r|^\\n]+)");
			std::regex headers_regex(pattern);
			std::smatch results;

			auto _begin = std::sregex_iterator(strHeaders.begin(), strHeaders.end(), headers_regex);
			auto _end = std::sregex_iterator();
			
			for (auto i = _begin; i != _end; ++i)
			{
				std::smatch match = *i;
				if (match.size() == 3)
					Headers[match[1].str()] = match[2].str();
			}

			Headers["Set-Cookies"] = GetCookieStr();

			return Headers;
		}

		std::string GetLocal()
		{
			return GetResponseHeaderValue(HTTP_QUERY_LOCATION);
		}

		DWORD GetStatusCode()
		{
			assert(m_hRequest != NULL);

			DWORD dwStatusCode;
			DWORD dwSize = sizeof(DWORD);
			HttpQueryInfoA(m_hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatusCode, &dwSize, NULL);
			return dwStatusCode;
		}

		std::string GetCookieStr()
		{
			Cookies result = GetCookies();

			std::string cookies;
			for (auto &kv : result)
			{
				if (kv.second == "-" || kv.second == "''")
					continue;
				if (cookies.empty())
					cookies.append(kv.first).append("=").append(kv.second);
				else
					cookies.append("; ").append(kv.first).append("=").append(kv.second);
			}

			return cookies;
		}

		Cookies GetCookies()
		{
			Cookies result;

			std::string strHeaders = GetResponseHeaders();
			std::string cookies_pattern("Set-Cookie: ([^\\r|^\\n]+)");

			std::regex cookies_regex(cookies_pattern);
			std::smatch results;

			auto cookies_begin = std::sregex_iterator(strHeaders.begin(), strHeaders.end(), cookies_regex);
			auto cookies_end = std::sregex_iterator();

			for (auto i = cookies_begin; i != cookies_end; ++i)
			{
				std::smatch match = *i;
				Cookies cookies = ParseCookies(match.str());
				for (auto &kv : cookies)
					result[kv.first] = kv.second;
			}

			return result;
		}

		void SetBasicAuthenticator(const std::string& username, const std::string& password)
		{
			this->m_basic_auth_user = username;
			this->m_basic_auth_pass = password;
		}

		void SetProxy(const std::string& proxy, const std::string& proxyName = "", const std::string& proxyPass = "")
		{
			this->m_proxy_info		= proxy;
			this->m_proxy_user		= proxyName;
			this->m_proxy_pass		= proxyPass;
		}
	};
}

#endif // _WINNET_HPP_INCLUDED_
