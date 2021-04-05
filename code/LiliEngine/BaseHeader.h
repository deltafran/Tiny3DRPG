#pragma once

//=============================================================================
// Base header
//=============================================================================
#include "EngineConfig.h"
#include "BaseMacros.h"

//=============================================================================
// Disable warning
//=============================================================================
#pragma warning(disable : 4820) // C4820 padding added after data member
#pragma warning(disable : 5045) // C5045 Spectre mitigation warning

//=============================================================================
// Standart header
//=============================================================================
#define _STL_WARNING_LEVEL 3
#pragma warning(push, 3)
#pragma warning(disable : 5219)

#include <cassert>
#include <cmath>

#include <iostream>
#include <fstream>

#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#pragma warning(pop)

//=============================================================================
// Platform header
//=============================================================================

#pragma warning(push, 3)

#include <winsdkver.h>
#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX // Don't let Windows define min() or max()
#define NODRAWTEXT
#define NOBITMAP
#define NOMCX
//#define NOGDI
#define NOSERVICE
#define NOHELP
#include <Windows.h>
#include <xmmintrin.h>

#ifndef GET_X_LPARAM
#	define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#	define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

#if LILI_DIRECT3D11
#	include <wrl/client.h>
using Microsoft::WRL::ComPtr;
#	include <d3d11_1.h>
#	include <d3dcompiler.h>
#	if defined(NTDDI_WIN10_RS2)
#		include <dxgi1_6.h>
#	else
#		include <dxgi1_5.h>
#	endif
#	include <DirectXMath.h>
using namespace DirectX;
#	ifdef _DEBUG
#		include <dxgidebug.h>
#	endif
#endif

#if LILI_VULKAN
#	define VK_USE_PLATFORM_WIN32_KHR 1
#	include <vulkan/vulkan.h>
#endif

#pragma warning(pop)

//=============================================================================
// Engine header
//=============================================================================

//=============================================================================
// Temp header
//=============================================================================
