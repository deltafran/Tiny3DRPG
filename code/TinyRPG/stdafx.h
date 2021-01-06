#pragma once

#pragma warning(disable : 4820) // C4820 padding added after data member
#pragma warning(disable : 5045) // C5045 Spectre mitigation warning

//=============================================================================
// Standart header
//=============================================================================
#define _STL_WARNING_LEVEL 3
#pragma warning(push, 3)
#pragma warning(disable : 5219)

#include <exception>

#include <iostream>
#include <fstream>

#include <string>

#include <winsdkver.h>
#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NODRAWTEXT
#define NOBITMAP
#define NOMCX
//#define NOGDI
#define NOSERVICE
#define NOHELP
#include <Windows.h>

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include <d3d11_1.h>
#include <d3dcompiler.h>

#if defined(NTDDI_WIN10_RS2)
#	include <dxgi1_6.h>
#else
#	include <dxgi1_5.h>
#endif

#include <DirectXMath.h>
using namespace DirectX;

#ifdef _DEBUG
#	include <dxgidebug.h>
#endif


#pragma warning(pop)

//=============================================================================
// Engine header
//=============================================================================
#include "Globals.h"
#include "Exception.h"

//=============================================================================
// Temp header
//=============================================================================
#include "ConfigExample.h"