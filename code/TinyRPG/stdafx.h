#pragma once

#pragma warning(disable : 4820)

//=============================================================================
// Standart header
//=============================================================================
#define _STL_WARNING_LEVEL 3
#pragma warning(push, 3)
#pragma warning(disable : 5219)
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;
#pragma warning(pop)

//=============================================================================
// Engine header
//=============================================================================
#include "Globals.h"

//=============================================================================
// Temp header
//=============================================================================
#include "ConfigExample.h"