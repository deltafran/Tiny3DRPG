#include "stdafx.h"
#include "Model.h"
#include "RendererSystem.h"

bool Model::Init(const char* textureFilename)
{
	bool result = initializeBuffers();
	if (!result)
		return false;

	result = loadTexture(textureFilename);
	if (!result)
		return false;

	return true;
}

void Model::Close()
{
	releaseTexture();
	shutdownBuffers();
}

void Model::Render()
{
	ID3D11DeviceContext* deviceContext = Globals::RendererSystem().GetDeviceContext();

	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

int Model::GetIndexCount()
{
	return m_indexCount;
}

ID3D11ShaderResourceView* Model::GetTexture()
{
	return m_Texture->GetTexture();
}

bool Model::initializeBuffers()
{
	ID3D11Device* device = Globals::RendererSystem().GetDevice();	
	
	HRESULT result;

	m_vertexCount = 4;
	m_indexCount = 6;

	VertexType* vertices = new VertexType[m_vertexCount];
	if (!vertices)
		return false;

	unsigned long* indices = new unsigned long[m_indexCount];
	if (!indices)
		return false;

	vertices[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	vertices[0].texture = XMFLOAT2(0.0f, 0.0f);
	vertices[1].position = XMFLOAT3(1.0f, -1.0f, 0.0f);
	vertices[1].texture = XMFLOAT2(1.0f, 0.0f);
	vertices[2].position = XMFLOAT3(-1.0f, 1.0f, 0.0f);
	vertices[2].texture = XMFLOAT2(0.0f, 1.0f);
	vertices[3].position = XMFLOAT3(1.0f, 1.0f, 0.0f);
	vertices[3].texture = XMFLOAT2(1.0f, 1.0f);

	indices[0] = 2;
	indices[1] = 1;
	indices[2] = 0; 
	indices[3] = 1;
	indices[4] = 2;
	indices[5] = 3;


	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	D3D11_BUFFER_DESC  indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	delete[] vertices;
	vertices = 0;
	delete[] indices;
	indices = 0;

	return true;
}

void Model::shutdownBuffers()
{
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}
}

bool Model::loadTexture(const char* filename)
{
	m_Texture = new Texture();
	if (!m_Texture->Create(filename))
		return false;

	return true;
}

void Model::releaseTexture()
{
	if (m_Texture)
	{
		m_Texture->Close();
		delete m_Texture;
		m_Texture = 0;
	}
}