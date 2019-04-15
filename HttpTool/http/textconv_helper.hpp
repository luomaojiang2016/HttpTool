/*
* Author: LowBoyTeam (https://github.com/LowBoyTeam)
* License: Code Project Open License
* Disclaimer: The software is provided "as-is". No claim of suitability, guarantee, or any warranty whatsoever is provided.
* Copyright (c) 2016-2017.
*/

#ifndef _TEXTCONV_HELPER_HPP_INCLUDED_
#define _TEXTCONV_HELPER_HPP_INCLUDED_

#include <windows.h>
#include <vector>

////////////////////////////////////////////////////////
// Classes and functions (typedefs) for text conversions
//
//  This section defines the following text conversions:
//  A2BSTR		ANSI  to BSTR
//  A2OLE		ANSI  to OLE
//	A2T			ANSI  to TCHAR
//	A2W			ANSI  to WCHAR
//  OLE2A		OLE   to ANSI
//  OLE2T		OLE   to TCHAR
//  OLE2W		OLE   to WCHAR
//  T2A			TCHAR to ANSI
//  T2BSTR		TCHAR to BSTR
//  T2OLE       TCHAR to OLE
//  T2W			TCHAR to WCHAR
//  W2A			WCHAR to ANSI
//  W2BSTR		WCHAR to BSTR
//  W2OLE		WCHAR to OLE
//  W2T			WCHAR to TCHAR

// About different character and string types:
// ------------------------------------------
// char (or CHAR) character types are ANSI (8 bits).
// wchar_t (or WCHAR) character types are Unicode (16 bits).
// TCHAR characters are Unicode if the _UNICODE macro is defined, otherwise they are ANSI.
// BSTR (Basic String) is a type of string used in Visual Basic and COM programming.
// OLE is the same as WCHAR. It is used in Visual Basic and COM programming.

namespace textconv_helper
{
	// Forward declarations of our classes. They are defined later.
	class CA2A_;
	class CA2W_;
	class CW2A_;
	class CW2W_;
	class CA2BSTR_;
	class CW2BSTR_;

	// typedefs for the well known text conversions
	typedef CA2W_ A2W_;
	typedef CW2A_ W2A_;
	typedef CW2BSTR_ W2BSTR_;
	typedef CA2BSTR_ A2BSTR_;
	typedef CW2A_ BSTR2A_;
	typedef CW2W_ BSTR2W_;

#ifdef _UNICODE
	typedef CA2W_ A2T_;
	typedef CW2A_ T2A_;
	typedef CW2W_ T2W_;
	typedef CW2W_ W2T_;
	typedef CW2BSTR_ T2BSTR_;
	typedef BSTR2W_ BSTR2T_;
#else
	typedef CA2A_ A2T_;
	typedef CA2A_ T2A_;
	typedef CA2W_ T2W_;
	typedef CW2A_ W2T_;
	typedef CA2BSTR_ T2BSTR_;
	typedef BSTR2A_ BSTR2T_;
#endif

	typedef A2W_  A2OLE_;
	typedef T2W_  T2OLE_;
	typedef CW2W_ W2OLE_;
	typedef W2A_  OLE2A_;
	typedef W2T_  OLE2T_;
	typedef CW2W_ OLE2W_;

	class CA2W_
	{
	public:
		CA2W_(LPCSTR pStr, UINT codePage = CP_ACP) : m_pStr(pStr)
		{
			if (pStr)
			{
				// Resize the vector and assign null WCHAR to each element
				int length = MultiByteToWideChar(codePage, 0, pStr, -1, NULL, 0) + 1;
				m_vWideArray.assign(length, L'\0');

				// Fill our vector with the converted WCHAR array
				MultiByteToWideChar(codePage, 0, pStr, -1, &m_vWideArray[0], length);
			}
		}
		~CA2W_() {}
		operator LPCWSTR() { return m_pStr ? &m_vWideArray[0] : NULL; }
		operator LPOLESTR() { return m_pStr ? (LPOLESTR)&m_vWideArray[0] : (LPOLESTR)NULL; }

	private:
		CA2W_(const CA2W_&);
		CA2W_& operator= (const CA2W_&);
		std::vector<wchar_t> m_vWideArray;
		LPCSTR m_pStr;
	};

	class CW2A_
	{
	public:
		CW2A_(LPCWSTR pWStr, UINT codePage = CP_ACP) : m_pWStr(pWStr)
			// Usage:
			//   CW2A_ ansiString(L"Some Text");
			//   CW2A_ utf8String(L"Some Text", CP_UTF8);
			//
			// or
			//   SetWindowTextA( W2A(L"Some Text") ); The ANSI version of SetWindowText
		{
			// Resize the vector and assign null char to each element
			int length = WideCharToMultiByte(codePage, 0, pWStr, -1, NULL, 0, NULL, NULL) + 1;
			m_vAnsiArray.assign(length, '\0');

			// Fill our vector with the converted char array
			WideCharToMultiByte(codePage, 0, pWStr, -1, &m_vAnsiArray[0], length, NULL, NULL);
		}

		~CW2A_()
		{
			m_pWStr = 0;
		}
		operator LPCSTR() { return m_pWStr ? &m_vAnsiArray[0] : NULL; }

	private:
		CW2A_(const CW2A_&);
		CW2A_& operator= (const CW2A_&);
		std::vector<char> m_vAnsiArray;
		LPCWSTR m_pWStr;
	};

	class CW2W_
	{
	public:
		CW2W_(LPCWSTR pWStr) : m_pWStr(pWStr) {}
		operator LPCWSTR() { return const_cast<LPWSTR>(m_pWStr); }
		operator LPOLESTR() { return const_cast<LPOLESTR>(m_pWStr); }

	private:
		CW2W_(const CW2W_&);
		CW2W_& operator= (const CW2W_&);

		LPCWSTR m_pWStr;
	};

	class CA2A_
	{
	public:
		CA2A_(LPCSTR pStr) : m_pStr(pStr) {}
		operator LPCSTR() { return (LPSTR)m_pStr; }

	private:
		CA2A_(const CA2A_&);
		CA2A_& operator= (const CA2A_&);

		LPCSTR m_pStr;
	};

	class CW2BSTR_
	{
	public:
		CW2BSTR_(LPCWSTR pWStr) { m_bstrString = ::SysAllocString(pWStr); }
		~CW2BSTR_() { ::SysFreeString(m_bstrString); }
		operator BSTR() { return m_bstrString; }

	private:
		CW2BSTR_(const CW2BSTR_&);
		CW2BSTR_& operator= (const CW2BSTR_&);
		BSTR m_bstrString;
	};

	class CA2BSTR_
	{
	public:
		CA2BSTR_(LPCSTR pStr) { m_bstrString = ::SysAllocString(textconv_helper::CA2W_(pStr)); }
		~CA2BSTR_() { ::SysFreeString(m_bstrString); }
		operator BSTR() { return m_bstrString; }

	private:
		CA2BSTR_(const CA2BSTR_&);
		CA2BSTR_& operator= (const CA2BSTR_&);
		BSTR m_bstrString;
	};
}

#endif // _TEXTCONV_HELPER_HPP_INCLUDED_