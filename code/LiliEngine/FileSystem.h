#pragma once

#include "Delegate.h"
#include "NonCopyable.h"

template <typename T> struct Delegate;
template <typename T> struct UniquePtr;

class FileSystem final : NonCopyable
{
public:
};