/*
* Author: LowBoyTeam (https://github.com/LowBoyTeam)
* License: Code Project Open License
* Disclaimer: The software is provided "as-is". No claim of suitability, guarantee, or any warranty whatsoever is provided.
* Copyright (c) 2016-2017.
*/

#ifndef _WINHTTP_HPP_INCLUDED_
#define _WINHTTP_HPP_INCLUDED_

#include <string>
#include <tchar.h>
#include <vector>
#include <map>
#include <regex>
#include <assert.h>

#include <windows.h>

#include <winhttp.h>
#pragma comment(lib, "winhttp")

#include "textconv_helper.hpp"

namespace winhttp
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

	class CWinHttp
	{
	private:

		HINTERNET			m_hSession;
		HINTERNET			m_hConnect;
		HINTERNET			m_hRequest;

		std::wstring		m_strHost;
		std::wstring		m_strPath;
		std::wstring		m_strExt;

		INTERNET_SCHEME		m_nScheme;
		INTERNET_PORT		m_nPort;

		BOOL				m_followRedirects;
		HeaderFields		m_RequestHeaders;
		std::wstring		m_request_method;

		std::wstring		m_proxy_info;
		std::wstring		m_proxy_user;
		std::wstring		m_proxy_pass;

		std::wstring		m_basic_auth_user;
		std::wstring		m_basic_auth_pass;

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

		CWinHttp()
		{

		}

		virtual ~CWinHttp()
		{
			if (m_hRequest)
			{
				WinHttpCloseHandle(m_hRequest);
				m_hRequest = NULL;
			}
			if (m_hConnect)
			{
				WinHttpCloseHandle(m_hConnect);
				m_hConnect = NULL;
			}
			if (m_hSession)
			{
				WinHttpCloseHandle(m_hSession);
				m_hSession = NULL;
			}
		}

		BOOL Open(const std::string& Url, const std::string& method)
		{
			m_hSession = WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

			if (!m_hSession)
			{
				printf("Error %u in WinHttpOpen.\n", GetLastError());
				return FALSE;
			}

			if (WinHttpCheckPlatform())
			{
				URL_COMPONENTSW urlComp;
				ZeroMemory(&urlComp, sizeof(urlComp));
				urlComp.dwStructSize = sizeof(urlComp);
				urlComp.dwHostNameLength = 1;
				urlComp.dwUserNameLength = 1;
				urlComp.dwPasswordLength = 1;
				urlComp.dwUrlPathLength = 1;
				urlComp.dwExtraInfoLength = 1;

				std::wstring wstrUrl = textconv_helper::A2W_(Url.c_str());

				if (!WinHttpCrackUrl(wstrUrl.c_str(), 0, 0, &urlComp))
				{
					printf("Error %u in WinHttpCrackUrl.\n", GetLastError());
					return FALSE;
				}
				
				m_strHost = std::wstring(urlComp.lpszHostName, urlComp.dwHostNameLength);
				m_strPath = std::wstring(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
				m_strExt = std::wstring(urlComp.lpszExtraInfo, urlComp.dwExtraInfoLength);
				m_nScheme = urlComp.nScheme;
				m_nPort = urlComp.nPort;
				m_strPath.append(m_strExt);
				m_request_method = textconv_helper::A2W_(method.c_str());;
				return TRUE;
			}
			return FALSE;
		}

		BOOL SetTimeout(int nTimeOut)
		{
			assert(m_hSession != NULL);
			return WinHttpSetTimeouts(m_hSession, nTimeOut, nTimeOut, nTimeOut, nTimeOut);
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
			assert(m_hSession!=NULL);

			BOOL  bResults = FALSE;

			m_hConnect = WinHttpConnect(m_hSession, m_strHost.c_str(), m_nPort, 0);	
			
			if (!m_hConnect)
			{
				printf("Error %u in WinHttpConnect.\n", GetLastError());
				return FALSE;
			}

			if (!m_proxy_info.empty())
			{
				WINHTTP_PROXY_INFO proxyInfo;
				proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
				proxyInfo.lpszProxy = const_cast<LPWSTR>(m_proxy_info.c_str());
				proxyInfo.lpszProxyBypass = NULL;

				if (!WinHttpSetOption(m_hSession, WINHTTP_OPTION_PROXY, (LPVOID)&proxyInfo, sizeof(proxyInfo)))
				{
					printf("Error %u in WinHttpSetOption. WINHTTP_OPTION_PROXY \n", GetLastError());
					return FALSE;
				}

				if (!m_proxy_user.empty() && !m_proxy_pass.empty())
				{
					if (!WinHttpSetOption(m_hConnect, WINHTTP_OPTION_PROXY_USERNAME, (LPVOID)m_proxy_user.c_str(), m_proxy_user.size() + 1))
					{
						printf("Error %u in WinHttpSetOption. WINHTTP_OPTION_PROXY_USERNAME \n", GetLastError());
						return FALSE;
					}
					if (!WinHttpSetOption(m_hConnect, WINHTTP_OPTION_PROXY_PASSWORD, (LPVOID)m_proxy_pass.c_str(), m_proxy_pass.size() + 1))
					{
						printf("Error %u in WinHttpSetOption. WINHTTP_OPTION_PROXY_PASSWORD \n", GetLastError());
						return FALSE;
					}
				}
			}

			m_hRequest = WinHttpOpenRequest(
				m_hConnect,
				m_request_method.c_str(),
				m_strPath.c_str(),
				nullptr,
				WINHTTP_NO_REFERER,
				WINHTTP_DEFAULT_ACCEPT_TYPES,
				m_nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0);

			if (!m_hRequest)
			{
				printf("Error %u in WinHttpOpenRequest.\n", GetLastError());
				return FALSE;
			}

			if (!m_basic_auth_user.empty() && !m_basic_auth_pass.empty())
			{
				if (!WinHttpSetCredentials(m_hRequest,
					WINHTTP_AUTH_TARGET_SERVER,
					WINHTTP_AUTH_SCHEME_BASIC,
					m_basic_auth_user.c_str(),
					m_basic_auth_pass.c_str(),
					NULL))
				{
					printf("Error %u in WinHttpSetCredentials \n", GetLastError());
					return FALSE;
				}
			}

			if (!m_followRedirects)
			{
				DWORD dwOption = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
				if (!WinHttpSetOption(m_hRequest, WINHTTP_OPTION_REDIRECT_POLICY, (LPVOID)&dwOption, sizeof(dwOption)))
				{
					printf("Error %u in WinHttpSetOption WINHTTP_OPTION_REDIRECT_POLICY .\n", GetLastError());
					return FALSE;
				}
			}

			//build http headers
			std::string strHeaders;
			for (auto &k : m_RequestHeaders)
				strHeaders.append(k.first).append(": ").append(k.second).append("\n");
			m_RequestHeaders.clear();

			std::wstring wstrHeaders = textconv_helper::A2W_(strHeaders.c_str());

			if (!WinHttpSendRequest(m_hRequest, wstrHeaders.c_str(), wstrHeaders.length(), lpPostData, dwsize, dwsize, 0))
			{
				printf("Error %u in WinHttpSendRequest.\n", GetLastError());
				return FALSE;
			}

			return TRUE;
		}

		std::vector<BYTE> GetResponseBody()
		{
			DWORD dwReadSize = 0;
			DWORD dwDownloaded = 0;

			std::vector<BYTE> list;

			if (WinHttpReceiveResponse(m_hRequest, NULL))
			{
				do
				{
					// Check for available data.
					dwReadSize = 0;
					if (!WinHttpQueryDataAvailable(m_hRequest, &dwReadSize))
					{
						printf("Error %u in WinHttpQueryDataAvailable.\n", GetLastError());
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

						if (!WinHttpReadData(m_hRequest, lpReceivedData, dwReadSize, &dwDownloaded)) {
							printf("Error %u in WinHttpReadData.\n", GetLastError());
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
			}
			return list;
		}

		std::string GetResponseHeaderValue(int dwInfoLevel, DWORD dwIndex = WINHTTP_NO_HEADER_INDEX)
		{
			std::wstring		result;
			LPVOID				lpOutBuffer = NULL;
			DWORD				dwSize = 0;

		retry:

			if (!WinHttpQueryHeaders(m_hRequest, dwInfoLevel, WINHTTP_HEADER_NAME_BY_INDEX, lpOutBuffer, &dwSize, &dwIndex))
			{
				if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				{
					// Allocate the necessary buffer.
					lpOutBuffer = new WCHAR[dwSize / sizeof(WCHAR)];
					// Retry the call.
					goto retry;
				}
			}

			if (lpOutBuffer)
				result.assign(reinterpret_cast<LPWSTR>(lpOutBuffer));

			if (lpOutBuffer)
				delete[] lpOutBuffer;

			return std::string(textconv_helper::W2A_(result.c_str()));
		}
		
		std::string GetResponseHeaders()
		{
			return GetResponseHeaderValue(WINHTTP_QUERY_RAW_HEADERS_CRLF);
		}

		std::string GetResponseHeaderValue(const std::string& name)
		{
			std::wstring		result;

			LPVOID				lpOutBuffer = NULL;
			DWORD				dwSize = 0;
			DWORD				dwIndex = 0;

		retry:

			if (!WinHttpQueryHeaders(m_hRequest, WINHTTP_QUERY_CUSTOM, textconv_helper::A2W_(name.c_str()), lpOutBuffer, &dwSize, &dwIndex))
			{
				if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				{
					// Allocate the necessary buffer.
					lpOutBuffer = new WCHAR[dwSize / sizeof(WCHAR)];
					// Retry the call.
					goto retry;
				}
			}

			if (lpOutBuffer)
				result.assign(reinterpret_cast<LPWSTR>(lpOutBuffer));

			if (lpOutBuffer)
				delete[] lpOutBuffer;

			return std::string(textconv_helper::W2A_(result.c_str()));
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
			return GetResponseHeaderValue(WINHTTP_QUERY_LOCATION);
		}

		DWORD GetStatusCode()
		{
			DWORD dwStatusCode;
			DWORD dwSize = sizeof(DWORD);
			BOOL bRet = WinHttpQueryHeaders(m_hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwStatusCode, &dwSize, NULL);
			return bRet ? dwStatusCode : 0;
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
			this->m_basic_auth_user = textconv_helper::A2W_(username.c_str());
			this->m_basic_auth_pass = textconv_helper::A2W_(password.c_str());
		}

		void SetProxy(const std::string& proxy, const std::string& proxyName = "", const std::string& proxyPass = "")
		{
			this->m_proxy_info = textconv_helper::A2W_(proxy.c_str());
			this->m_proxy_user = textconv_helper::A2W_(proxyName.c_str());
			this->m_proxy_pass = textconv_helper::A2W_(proxyPass.c_str());
		}
	};
}

#endif // _WINHTTP_HPP_INCLUDED_


