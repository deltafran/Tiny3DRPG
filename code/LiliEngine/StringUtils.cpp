#include "stdafx.h"
#include "StringUtils.h"

namespace str
{
	int StringLength(const char* str)
	{
		return (int)strlen(str);
	}

	bool CopyString(Span<char> dst, Span<const char> src)
	{
		if (dst.length() < 1) return false;
		if (src.length() < 1) {
			*dst.begin = 0;
			return true;
		}

		uint32_t length = dst.length();
		char* tmp = dst.begin;
		const char* srcp = src.begin;
		while (srcp != src.end && length > 1)
		{
			*tmp = *srcp;
			--length;
			++tmp;
			++srcp;
		}
		*tmp = 0;
		return srcp == src.end;
	}

	bool CopyString(Span<char> dst, const char* src)
	{
		if (!src || dst.length() < 1) return false;

		uint32_t length = dst.length();
		char* tmp = dst.begin;
		while (*src && length > 1) 
		{
			*tmp = *src;
			--length;
			++tmp;
			++src;
		}
		*tmp = 0;
		return *src == '\0';
	}

	inline char makeLowercase(char c)
	{
		return c >= 'A' && c <= 'Z' ? c - ('A' - 'a') : c;
	}

	bool MakeLowercase(Span<char> dst, const char* source)
	{
		char* destination = dst.begin;
		uint32_t length = dst.length();
		if (!source)
		{
			return false;
		}

		while (*source && length)
		{
			*destination = makeLowercase(*source);
			--length;
			++destination;
			++source;
		}
		if (length > 0)
		{
			*destination = 0;
			return true;
		}
		return false;
	}

	bool EqualStrings(const char* lhs, const char* rhs)
	{
		return strcmp(lhs, rhs) == 0;
	}

	bool EqualStrings(Span<const char> lhs, Span<const char> rhs)
	{
		if (rhs.length() != lhs.length()) return false;
		return strncmp(lhs.begin, rhs.begin, lhs.length()) == 0;
	}
} // namespace str