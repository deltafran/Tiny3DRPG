#pragma once

class FileManager
{
public:
	static bool ReadFile(const std::string& filepath, uint8_t*& dataPtr, uint32_t& dataSize);
};