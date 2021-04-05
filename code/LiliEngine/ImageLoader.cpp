#include "stdafx.h"
#include "ImageLoader.h"
#pragma warning(push, 0)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#pragma warning(pop)

uint8_t* ImageLoader::LoadFromMemory(const uint8_t* inBuffer, int32_t inSize, int32_t* outWidth, int32_t* outHeight, int32_t* outComp, int32_t reqComp)
{
	return stbi_load_from_memory(inBuffer, inSize, outWidth, outHeight, outComp, reqComp);
}

float* ImageLoader::LoadFloatFromMemory(const uint8_t* inBuffer, int32_t inSize, int32_t* outWidth, int32_t* outHeight, int32_t* outComp, int32_t reqComp)
{
	return stbi_loadf_from_memory(inBuffer, inSize, outWidth, outHeight, outComp, reqComp);
}

void ImageLoader::Free(uint8_t* data)
{
	stbi_image_free(data);
}