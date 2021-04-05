#pragma once

class ImageLoader
{
public:
	static uint8_t* LoadFromMemory(const uint8_t* inBuffer, int32_t inSize, int32_t* outWidth, int32_t* outHeight, int32_t* outComp, int32_t reqComp);
	static float* LoadFloatFromMemory(const uint8_t* inBuffer, int32_t inSize, int32_t* outWidth, int32_t* outHeight, int32_t* outComp, int32_t reqComp);
	static void Free(uint8_t* data);
};