#include "stdafx.h"
#include "FileManager.h"
#include "Log.h"

bool FileManager::ReadFile(const std::string& filepath, uint8_t*& dataPtr, uint32_t& dataSize)
{
	std::string finalPath = filepath;

#if PLATFORM_ANDROID

	AAsset* asset = AAssetManager_open(g_AndroidApp->activity->assetManager, finalPath.c_str(), AASSET_MODE_STREAMING);
	dataSize = AAsset_getLength(asset);
	dataPtr = new uint8[dataSize];
	AAsset_read(asset, dataPtr, dataSize);
	AAsset_close(asset);

#else

	errno_t err;
	FILE* file = nullptr;;
	err = fopen_s(&file, finalPath.c_str(), "rb");
	if (err!= 0 || !file)
	{
		Log::Error("File not found :" + filepath);
		return false;
	}

	fseek(file, 0, SEEK_END);
	dataSize = (uint32_t)ftell(file);
	fseek(file, 0, SEEK_SET);

	if (dataSize <= 0) {
		fclose(file);
		Log::Error("File has no data :" + filepath);
		return false;
	}

	dataPtr = new uint8_t[dataSize];
	fread(dataPtr, 1, dataSize, file);
	fclose(file);

#endif

	return true;
}
