#pragma once

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
	static void AddUnique(std::vector<std::string>& arr, const std::string& val) noexcept
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

	static void AddUnique(std::vector<const char*>& arr, const char* val) noexcept
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