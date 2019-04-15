/*
* Author: LowBoyTeam (https://github.com/LowBoyTeam)
* License: Code Project Open License
* Disclaimer: The software is provided "as-is". No claim of suitability, guarantee, or any warranty whatsoever is provided.
* Copyright (c) 2016-2017.
*/

#ifndef _WINNET_RESTCLIENT_HPP_INCLUDED_
#define _WINNET_RESTCLIENT_HPP_INCLUDED_

#include <string>
#include <map>

#if defined _USE_WIHTTP_INTERFACE
#include "winhttp_helper.hpp"
using namespace winhttp;
#define HttpClient CWinHttp
#else
#include "winnet_helper.hpp"
using namespace winnet;
#define HttpClient CWinNet
#endif

namespace RestClient
{
	typedef struct {
		int code;
		std::string		body;
		HeaderFields	headers;
		std::string		cookies;
		Cookies			Cookie;
		std::string get_header(const std::string& name)
		{
			return this->headers[name];
		};
		std::string get_cookie(const std::string& name)
		{
			return this->Cookie[name];
		};
	} Response;

	typedef struct {
		int timeout;
		bool followRedirects = true;
		HeaderFields headers;
		struct {
			std::string proxy;
			std::string username;
			std::string password;
		} proxy;
		struct {
			std::string username;
			std::string password;
		} basicAuth;
		std::string set_userAgent(const std::string& value)
		{
			return this->headers["User-Agent"] = value;
		};
		std::string set_cookie(const std::string& value)
		{
			return this->headers["Cookie"] = value;
		};
		std::string set_referer(const std::string& value)
		{
			return this->headers["Referer"] = value;
		};
	} Request;

	Request default_request;

	Response get(const std::string& url, Request* request = nullptr) {
		
		Response ret;

		HttpClient http;

		if (http.Open(url, HTTP_METHOD_GET))
		{
			if (request == nullptr)
				request = &default_request;

			if (request->timeout > 0)
				http.SetTimeout(request->timeout);

			http.FollowRedirects(request->followRedirects);

			if (!request->headers.empty())
			{
				for (auto &kv : request->headers)
					http.SetRequestHeader(kv.first.c_str(), kv.second.c_str());
			}

			if (!request->proxy.proxy.empty())
				http.SetProxy(request->proxy.proxy, request->proxy.username, request->proxy.password);

			if (!request->basicAuth.username.empty() && !request->basicAuth.password.empty())
				http.SetBasicAuthenticator(request->basicAuth.username, request->basicAuth.password);

			if (http.Send())
			{
				std::string result;
				std::vector<BYTE> response = http.GetResponseBody();
				result.insert(result.begin(), response.begin(), response.end());
				ret.body = result;
				ret.code = http.GetStatusCode();
				ret.cookies = http.GetCookieStr();
				ret.Cookie = http.GetCookies();
				ret.headers = http.GetHeaderFields();
			}
		}
		return ret;
	}

	Response post(const std::string& url, const std::string& content_type, const std::string& data, Request* request = nullptr) {

		Response ret;

		HttpClient http;

		if (http.Open(url, HTTP_METHOD_POST))
		{
			if (request == nullptr)
				request = &default_request;

			http.FollowRedirects(request->followRedirects);

			http.SetRequestHeader("Content-Type", content_type);

			if (request->timeout > 0)
				http.SetTimeout(request->timeout);

			if (!request->headers.empty())
			{
				for (auto &kv : request->headers)
					http.SetRequestHeader(kv.first.c_str(), kv.second.c_str());
			}

			if (!request->proxy.proxy.empty())
				http.SetProxy(request->proxy.proxy, request->proxy.username, request->proxy.password);

			if (!request->basicAuth.username.empty() && !request->basicAuth.password.empty())
				http.SetBasicAuthenticator(request->basicAuth.username, request->basicAuth.password);

			if (http.Send((LPVOID)data.c_str(), data.length()))
			{
				std::string result;
				std::vector<BYTE> response = http.GetResponseBody();
				result.insert(result.begin(), response.begin(), response.end());
				ret.body = result;
				ret.code = http.GetStatusCode();
				ret.cookies = http.GetCookieStr();
				ret.Cookie = http.GetCookies();
				ret.headers = http.GetHeaderFields();
			}
		}
		return ret;
	}

	Response put(const std::string& url, const std::string& content_type, const std::string& data, Request* request = nullptr) {

		Response ret;

		HttpClient http;

		if (http.Open(url, HTTP_METHOD_PUT))
		{
			if (request == nullptr)
				request = &default_request;

			http.FollowRedirects(request->followRedirects);

			http.SetRequestHeader("Content-Type", content_type);

			if (request->timeout > 0)
				http.SetTimeout(request->timeout);

			if (!request->headers.empty())
			{
				for (auto &kv : request->headers)
					http.SetRequestHeader(kv.first.c_str(), kv.second.c_str());
			}

			if (!request->proxy.proxy.empty())
				http.SetProxy(request->proxy.proxy, request->proxy.username, request->proxy.password);

			if (!request->basicAuth.username.empty() && !request->basicAuth.password.empty())
				http.SetBasicAuthenticator(request->basicAuth.username, request->basicAuth.password);

			if (http.Send((LPVOID)data.c_str(), data.length()))
			{
				std::string result;
				std::vector<BYTE> response = http.GetResponseBody();
				result.insert(result.begin(), response.begin(), response.end());
				ret.body = result;
				ret.code = http.GetStatusCode();
				ret.cookies = http.GetCookieStr();
				ret.Cookie = http.GetCookies();
				ret.headers = http.GetHeaderFields();
			}
		}
		return ret;
	}

	Response del(const std::string& url, Request* request = nullptr) {
		
		Response ret;

		HttpClient http;

		if (http.Open(url, HTTP_METHOD_DELETE))
		{
			if (request == nullptr)
				request = &default_request;

			if (request->timeout > 0)
				http.SetTimeout(request->timeout);

			http.FollowRedirects(request->followRedirects);

			if (!request->headers.empty())
			{
				for (auto &kv : request->headers)
					http.SetRequestHeader(kv.first.c_str(), kv.second.c_str());
			}

			if (!request->proxy.proxy.empty())
				http.SetProxy(request->proxy.proxy, request->proxy.username, request->proxy.password);

			if (!request->basicAuth.username.empty() && !request->basicAuth.password.empty())
				http.SetBasicAuthenticator(request->basicAuth.username, request->basicAuth.password);

			if (http.Send())
			{
				std::string result;
				std::vector<BYTE> response = http.GetResponseBody();
				result.insert(result.begin(), response.begin(), response.end());
				ret.body = result;
				ret.code = http.GetStatusCode();
				ret.cookies = http.GetCookieStr();
				ret.Cookie = http.GetCookies();
				ret.headers = http.GetHeaderFields();
			}
		}
		return ret;
	}

	Response head(const std::string& url, Request* request = nullptr) {
		
		Response ret;

		HttpClient http;

		if (http.Open(url, HTTP_METHOD_HEAD))
		{
			if (request == nullptr)
				request = &default_request;

			if (request->timeout > 0)
				http.SetTimeout(request->timeout);

			http.FollowRedirects(request->followRedirects);

			if (!request->headers.empty())
			{
				for (auto &kv : request->headers)
					http.SetRequestHeader(kv.first.c_str(), kv.second.c_str());
			}

			if (!request->proxy.proxy.empty())
				http.SetProxy(request->proxy.proxy, request->proxy.username, request->proxy.password);

			if (!request->basicAuth.username.empty() && !request->basicAuth.password.empty())
				http.SetBasicAuthenticator(request->basicAuth.username, request->basicAuth.password);

			if (http.Send())
			{
				std::string result;
				std::vector<BYTE> response = http.GetResponseBody();
				result.insert(result.begin(), response.begin(), response.end());
				ret.body = result;
				ret.code = http.GetStatusCode();
				ret.cookies = http.GetCookieStr();
				ret.Cookie = http.GetCookies();
				ret.headers = http.GetHeaderFields();
			}
		}
		return ret;
	}
}

#endif // _WINNET_RESTCLIENT_HPP_INCLUDED_
