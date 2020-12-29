#pragma once

#include "Texture.h"

class Model
{
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};

	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

public:
	bool Init(const char* modelFilename, const char* textureFilename);
	void Close();

	void Render();

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

private:
	bool initializeBuffers();
	void shutdownBuffers();

	bool loadTexture(const char*);
	void releaseTexture();

	bool loadModel(const char*);
	void releaseModel();

	ID3D11Buffer* m_vertexBuffer = nullptr, * m_indexBuffer = nullptr;
	int m_vertexCount, m_indexCount;
	Texture* m_Texture = nullptr;
	ModelType* m_model = nullptr;
};