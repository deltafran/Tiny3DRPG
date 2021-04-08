#pragma once

#include "NonCopyable.h"

template <class T>
class AutoDelete : NonCopyable
{
public:
	AutoDelete() : m_val(T()) {}
	~AutoDelete() { if (m_val) delete m_val; }

	void Assign(T val) { Release(); m_val = val; }
	void Release() { if (m_val) delete m_val; m_val = nullptr; }

	T Get() { return m_val; }
private:
	T m_val;
};