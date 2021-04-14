#include "stdafx.h"
#include "Path.h"
#include "StringUtils.h"
#include "crc32.h"

PathInfo::PathInfo(const char* path)
{
	char tmp[255];
	Path::Normalize(path, Span(tmp));
	str::CopyString(Span(extension), Path::GetExtension(Span(tmp, str::StringLength(tmp))));
	str::CopyString(Span(basename), Path::GetBasename(tmp));
	str::CopyString(Span(dir), Path::GetDir(tmp));
}

Path::Path(const char* path)
{
	Normalize(path, Span(m_path));
	m_hash = crc32(m_path);
}

uint32_t Path::Length() const 
{
	return str::StringLength(m_path);
}

void Path::operator =(const char* rhs)
{
	Normalize(rhs, Span(m_path));
	m_hash = crc32(m_path);
}

bool Path::operator==(const Path& rhs) const
{
	return m_hash == rhs.m_hash;
}

bool Path::operator!=(const Path& rhs) const
{
	return m_hash != rhs.m_hash;
}

void Path::Normalize(const char* path, Span<char> output)
{
	char* out = output.begin;
	uint32_t max_size = output.length();
	assert(max_size > 0);
	uint32_t i = 0;

	bool is_prev_slash = false;

	if (path[0] == '.' && (path[1] == '\\' || path[1] == '/'))
		path += 2;
#ifdef _WIN32
	if (path[0] == '\\' || path[0] == '/')
		++path;
#endif
	while (*path != '\0' && i < max_size)
	{
		bool is_current_slash = *path == '\\' || *path == '/';

		if (is_current_slash && is_prev_slash)
		{
			++path;
			continue;
		}

		*out = *path == '\\' ? '/' : *path;
#ifdef _WIN32
		* out = *path >= 'A' && *path <= 'Z' ? *path - 'A' + 'a' : *out;
#endif

		path++;
		out++;
		i++;

		is_prev_slash = is_current_slash;
	}
	(i < max_size ? *out : *(out - 1)) = '\0';
}

Span<const char> Path::GetDir(const char* src)
{
	if (!src[0]) return { nullptr, nullptr };

	Span<const char> dir;
	dir.begin = src;
	dir.end = src + str::StringLength(src) - 1;
	while (dir.end != dir.begin && *dir.end != '\\' && *dir.end != '/')
	{
		--dir.end;
	}
	if (dir.end != dir.begin) ++dir.end;
	return dir;
}

Span<const char> Path::GetBasename(const char* src)
{
	if (!src[0]) return Span<const char>(src, src);

	Span<const char> res;
	const char* end = src + str::StringLength(src);
	res.end = end;
	res.begin = end;
	while (res.begin != src && *res.begin != '\\' && *res.begin != '/')
	{
		--res.begin;
	}

	if (*res.begin == '\\' || *res.begin == '/') ++res.begin;
	res.end = res.begin;

	while (res.end != end && *res.end != '.') ++res.end;

	return res;
}


Span<const char> Path::GetExtension(Span<const char> src)
{
	if (src.length() == 0) return src;

	Span<const char> res;
	res.end = src.end;
	res.begin = src.end - 1;

	while (res.begin != src.begin && *res.begin != '.')
	{
		--res.begin;
	}
	if (*res.begin == '.') 
	{
		++res.begin;
	}

	return res;
}


bool Path::ReplaceExtension(char* path, const char* ext)
{
	char* end = path + str::StringLength(path);
	while (end > path && *end != '.')
	{
		--end;
	}
	if (*end != '.') return false;

	++end;
	const char* src = ext;
	while (*src != '\0' && *end != '\0')
	{
		*end = *src;
		++end;
		++src;
	}
	bool copied_whole_ext = *src == '\0';
	if (!copied_whole_ext) return false;

	*end = '\0';
	return true;
}


bool Path::HasExtension(const char* filename, const char* ext)
{
	char tmp[20];
	str::CopyString(Span(tmp), GetExtension(Span(filename, str::StringLength(filename))));
	str::MakeLowercase(Span(tmp), tmp);

	return str::EqualStrings(tmp, ext);
}