#pragma once

#include "Texture.h"

class Model
{
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};
public:
	bool Init(const char*);
	void Close();

	void Render();

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

private:
	bool initializeBuffers();
	void shutdownBuffers();

	bool loadTexture(const char*);
	void releaseTexture();

	ID3D11Buffer* m_vertexBuffer = nullptr, * m_indexBuffer = nullptr;
	int m_vertexCount, m_indexCount;
	Texture* m_Texture = nullptr;
};