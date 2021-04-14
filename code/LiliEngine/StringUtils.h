#pragma once

#include "Span.h"

namespace str
{

	int StringLength(const char* str);

	bool CopyString(Span<char> output, const char* source);
	bool CopyString(Span<char> output, Span<const char> source);

	bool MakeLowercase(Span<char> output, const char* source);

	bool EqualStrings(const char* lhs, const char* rhs);
	bool EqualStrings(Span<const char> lhs, Span<const char> rhs);
} // namespace str

#define STARTING_BUFFER_SIZE 512

static FORCEINLINE int32_t GetVarArgs(char* dest, SIZE_T destSize, int32_t count, const char*& fmt, va_list argPtr)
{
	int32_t Result = vsnprintf(dest, count, fmt, argPtr);
	va_end(argPtr);
	return Result;
}

#define GET_VARARGS(msg, msgsize, len, lastarg, fmt) \
	{ \
		va_list ap; \
		va_start(ap, lastarg); \
		GetVarArgs(msg, msgsize, len, fmt, ap); \
	}
#define GET_VARARGS_WIDE(msg, msgsize, len, lastarg, fmt) \
	{ \
		va_list ap; \
		va_start(ap, lastarg); \
		GetVarArgs(msg, msgsize, len, fmt, ap); \
	}
#define GET_VARARGS_ANSI(msg, msgsize, len, lastarg, fmt) \
	{ \
		va_list ap; \
		va_start(ap, lastarg); \
		GetVarArgs(msg, msgsize, len, fmt, ap); \
	}
#define GET_VARARGS_RESULT(msg, msgsize, len, lastarg, fmt, result) \
	{ \
		va_list ap; \
		va_start(ap, lastarg); \
		result = GetVarArgs(msg, msgsize, len, fmt, ap); \
		if (result >= msgsize) \
		{ \
			result = -1; \
		} \
	}
#define GET_VARARGS_RESULT_WIDE(msg, msgsize, len, lastarg, fmt, result) \
	{ \
		va_list ap; \
		va_start(ap, lastarg); \
		result = GetVarArgs(msg, msgsize, len, fmt, ap); \
		if (result >= msgsize) \
		{ \
			result = -1; \
		} \
	}
#define GET_VARARGS_RESULT_ANSI(msg, msgsize, len, lastarg, fmt, result) \
	{ \
		va_list ap; \
		va_start(ap, lastarg); \
		result = GetVarArgs(msg, msgsize, len, fmt, ap); \
		if (result >= msgsize) \
		{ \
			result = -1; \
		} \
	}


namespace Str
{




	static std::string Space(size_t Count, char Ascii = ' ')
	{
		return std::string(Count, Ascii);
	}

	/**
	Creates a string out of the given number.
	\code
	Str::Number( 5, 3); // This returns "005"
	Str::Number(16, 3); // This returns "016"
	\endcode
	*/
	static std::string Number(size_t Number, size_t DigitsCount, const char Ascii = '0')
	{
		std::string Str(std::to_string(Number));

		if (Str.size() < DigitsCount)
		{
			DigitsCount -= Str.size();
			Str = Str::Space(DigitsCount, Ascii) + Str;
		}

		return Str;
	}
} // namespace Str

namespace StringUtils
{
	inline std::string Printf(const char* fmt, ...)
	{
		int32_t bufferSize = STARTING_BUFFER_SIZE;
		char  startingBuffer[STARTING_BUFFER_SIZE];
		char* buffer = startingBuffer;
		int32_t result = -1;

		GET_VARARGS_RESULT(buffer, bufferSize, bufferSize - 1, fmt, fmt, result);

		if (result == -1)
		{
			buffer = nullptr;
			while (result == -1)
			{
				bufferSize *= 2;
				buffer = (char*)realloc(buffer, bufferSize * sizeof(char));
				GET_VARARGS_RESULT(buffer, bufferSize, bufferSize - 1, fmt, fmt, result);
			};
		}

		buffer[result] = 0;
		std::string resultString(buffer);

		if (bufferSize != STARTING_BUFFER_SIZE)
		{
			free(buffer);
		}

		return resultString;
	}

	inline void AddUnique(std::vector<std::string>& arr, const std::string& val) noexcept
	{
		bool found = false;
		for (size_t i = 0; i < arr.size(); ++i)
		{
			if (arr[i] == val)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			arr.push_back(val);
		}
	}

	inline void AddUnique(std::vector<const char*>& arr, const char* val) noexcept
	{
		bool found = false;
		for (size_t i = 0; i < arr.size(); ++i) 
		{
			if (strcmp(arr[i], val) == 0) 
			{
				found = true;
				break;
			}
		}
		if (!found) 
		{
			arr.push_back(val);
		}
	}
} // namespace StringUtils