#pragma once

#include "Span.h"

class PathInfo
{
public:
	PathInfo(const char* path);

	char extension[10];
	char basename[255];
	char dir[255];
};

class Path
{
public:
	Path() = default;
	explicit Path(const char* path);

	void operator=(const char* rhs);
	bool operator==(const Path& rhs) const;
	bool operator!=(const Path& rhs) const;

	static void Normalize(const char* path, Span<char> out);
	static Span<const char> GetDir(const char* src);
	static Span<const char> GetBasename(const char* src);
	static Span<const char> GetExtension(Span<const char> src);
	static bool HasExtension(const char* filename, const char* ext);
	static bool ReplaceExtension(char* path, const char* ext);


	uint32_t Length() const;
	uint32_t GetHash() const { return m_hash; }
	const char* c_str() const { return m_path; }
	bool IsEmpty() const { return m_path[0] == '\0'; }
private:
	char m_path[255] = {};
	uint32_t m_hash = 0;
};