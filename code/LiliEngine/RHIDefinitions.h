#pragma once

#include "PixelFormat.h"

enum ResLimit
{
	MAX_TEXTURE_MIP_COUNT = 14,
	MaxImmutableSamplers = 2,
	MaxVertexElementCount = 16,
	MaxVertexElementCount_NumBits = 4,
	MaxSimultaneousRenderTargets = 8,
	MaxSimultaneousRenderTargets_NumBits = 3,
	ShaderArrayElementAlignBytes = 16,
	MaxSimultaneousUAVs = 8
};

static_assert(MaxVertexElementCount <= (1 << MaxVertexElementCount_NumBits), "MaxVertexElementCount will not fit on MaxVertexElementCount_NumBits");
static_assert(MaxSimultaneousRenderTargets <= (1 << MaxSimultaneousRenderTargets_NumBits), "MaxSimultaneousRenderTargets will not fit on MaxSimultaneousRenderTargets_NumBits");

enum class ImageLayoutBarrier
{
	Undefined,
	TransferDest,
	ColorAttachment,
	DepthStencilAttachment,
	TransferSource,
	Present,
	PixelShaderRead,
	PixelDepthStencilRead,
	ComputeGeneralRW,
	PixelGeneralRW,
};

enum VertexAttribute
{
	VA_None = 0,
	VA_Position,
	VA_UV0,
	VA_UV1,
	VA_Normal,
	VA_Tangent,
	VA_Color,
	VA_SkinWeight,
	VA_SkinIndex,
	VA_SkinPack,
	VA_InstanceFloat1,
	VA_InstanceFloat2,
	VA_InstanceFloat3,
	VA_InstanceFloat4,
	VA_Custom0,
	VA_Custom1,
	VA_Custom2,
	VA_Custom3,
	VA_Count,
};

enum VertexElementType
{
	VET_None,
	VET_Float1,
	VET_Float2,
	VET_Float3,
	VET_Float4,
	VET_PackedNormal,
	VET_UByte4,
	VET_UByte4N,
	VET_Color,
	VET_Short2,
	VET_Short4,
	VET_Short2N,
	VET_Half2,
	VET_Half4,
	VET_Short4N,
	VET_UShort2,
	VET_UShort4,
	VET_UShort2N,
	VET_UShort4N,
	VET_URGB10A2N,
	VET_MAX,

	VET_NumBits = 5,
};

static_assert(VET_MAX <= (1 << VET_NumBits), "VET_MAX will not fit on VET_NumBits");

enum CubeFace
{
	CubeFace_PosX = 0,
	CubeFace_NegX,
	CubeFace_PosY,
	CubeFace_NegY,
	CubeFace_PosZ,
	CubeFace_NegZ,
	CubeFace_MAX
};

FORCEINLINE VertexAttribute StringToVertexAttribute(const char* name)
{
	if (strcmp(name, "inPosition") == 0) {
		return VertexAttribute::VA_Position;
	}
	else if (strcmp(name, "inUV0") == 0) {
		return VertexAttribute::VA_UV0;
	}
	else if (strcmp(name, "inUV1") == 0) {
		return VertexAttribute::VA_UV1;
	}
	else if (strcmp(name, "inNormal") == 0) {
		return VertexAttribute::VA_Normal;
	}
	else if (strcmp(name, "inTangent") == 0) {
		return VertexAttribute::VA_Tangent;
	}
	else if (strcmp(name, "inColor") == 0) {
		return VertexAttribute::VA_Color;
	}
	else if (strcmp(name, "inSkinWeight") == 0) {
		return VertexAttribute::VA_SkinWeight;
	}
	else if (strcmp(name, "inSkinIndex") == 0) {
		return VertexAttribute::VA_SkinIndex;
	}
	else if (strcmp(name, "inSkinPack") == 0) {
		return VertexAttribute::VA_SkinPack;
	}
	else if (strcmp(name, "inCustom0") == 0) {
		return VertexAttribute::VA_Custom0;
	}
	else if (strcmp(name, "inCustom1") == 0) {
		return VertexAttribute::VA_Custom1;
	}
	else if (strcmp(name, "inCustom2") == 0) {
		return VertexAttribute::VA_Custom2;
	}
	else if (strcmp(name, "inCustom3") == 0) {
		return VertexAttribute::VA_Custom3;
	}

	return VertexAttribute::VA_None;
}

static inline VkFormat VEToVkFormat(VertexElementType Type)
{
	switch (Type)
	{
	case VET_Float1:
		return VK_FORMAT_R32_SFLOAT;
	case VET_Float2:
		return VK_FORMAT_R32G32_SFLOAT;
	case VET_Float3:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case VET_PackedNormal:
		return VK_FORMAT_R8G8B8A8_SNORM;
	case VET_UByte4:
		return VK_FORMAT_R8G8B8A8_UINT;
	case VET_UByte4N:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case VET_Color:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case VET_Short2:
		return VK_FORMAT_R16G16_SINT;
	case VET_Short4:
		return VK_FORMAT_R16G16B16A16_SINT;
	case VET_Short2N:
		return VK_FORMAT_R16G16_SNORM;
	case VET_Half2:
		return VK_FORMAT_R16G16_SFLOAT;
	case VET_Half4:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case VET_Short4N:
		return VK_FORMAT_R16G16B16A16_SNORM;
	case VET_UShort2:
		return VK_FORMAT_R16G16_UINT;
	case VET_UShort4:
		return VK_FORMAT_R16G16B16A16_UINT;
	case VET_UShort2N:
		return VK_FORMAT_R16G16_UNORM;
	case VET_UShort4N:
		return VK_FORMAT_R16G16B16A16_UNORM;
	case VET_Float4:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case VET_URGB10A2N:
		return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
	default:
		break;
	}

	return VK_FORMAT_UNDEFINED;
}

static inline uint32_t ElementTypeToSize(VertexElementType type)
{
	switch (type)
	{
	case VET_Float1:
		return 4;
	case VET_Float2:
		return 8;
	case VET_Float3:
		return 12;
	case VET_Float4:
		return 16;
	case VET_PackedNormal:
		return 4;
	case VET_UByte4:
		return 4;
	case VET_UByte4N:
		return 4;
	case VET_Color:
		return 4;
	case VET_Short2:
		return 4;
	case VET_Short4:
		return 8;
	case VET_UShort2:
		return 4;
	case VET_UShort4:
		return 8;
	case VET_Short2N:
		return 4;
	case VET_UShort2N:
		return 4;
	case VET_Half2:
		return 4;
	case VET_Half4:
		return 8;
	case VET_Short4N:
		return 8;
	case VET_UShort4N:
		return 8;
	case VET_URGB10A2N:
		return 4;
	default:
		return 0;
	};
}

static inline uint32_t IndexTypeToSize(VkIndexType type)
{
	switch (type)
	{
	case VK_INDEX_TYPE_UINT16:
		return 2;
		break;
	case VK_INDEX_TYPE_UINT32:
		return 4;
		break;
	default:
		return 0;
		break;
	}
}
