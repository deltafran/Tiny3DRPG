#pragma once

class NonCopyable
{
public:
private:
	NonCopyable() = delete;
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable(NonCopyable&&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;
	NonCopyable& operator=(NonCopyable&&) = delete;
};