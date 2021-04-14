#pragma once

template<typename T>
class Span
{
public:
	Span() = default;
	Span(T* nbegin, T* nend) : begin(nbegin), end(nend) {}
	Span(T* nbegin, uint32_t len) : begin(nbegin), end(nbegin + len) {}
	template <int N> explicit Span(T(&value)[N]) : begin(value), end(value + N) {}

	T& operator[](uint32_t idx) const { assert(begin + idx < end); return begin[idx]; }
	operator Span<const T>() const { return Span<const T>(begin, end); }
	Span fromLeft(uint32_t count) const { return Span(begin + count, end); }

	uint32_t length() const { return uint32_t(end - begin); }

	T* begin = nullptr;
	T* end = nullptr;
};