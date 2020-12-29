#include "stdafx.h"
#include "Model.h"
#include "RendererSystem.h"

bool Model::Init(const char* modelFilename, const char* textureFilename)
{
	if (!loadModel(modelFilename))
		return false;

	if (!initializeBuffers())
		return false;

	if (!loadTexture(textureFilename))
		return false;

	return true;
}

void Model::Close()
{
	releaseTexture();
	shutdownBuffers();
	releaseModel();
}

void Model::Render()
{
	ID3D11DeviceContext* deviceContext = Globals::RendererSystem().GetDeviceContext();

	// Set vertex buffer stride and offset.
	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
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

#ifdef _TRIANGLE
	m_vertexCount = 4;
	m_indexCount = 6;
#endif

	VertexType* vertices = new VertexType[m_vertexCount];
	if (!vertices)
		return false;

	unsigned long* indices = new unsigned long[m_indexCount];
	if (!indices)
		return false;

#ifdef _TRIANGLE
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

#else
	// Load the vertex array and index array with data.
	for (int i = 0; i < m_vertexCount; i++)
	{
		vertices[i].position = XMFLOAT3(m_model[i].x, m_model[i].y, m_model[i].z);
		vertices[i].texture = XMFLOAT2(m_model[i].tu, m_model[i].tv);
		vertices[i].normal = XMFLOAT3(m_model[i].nx, m_model[i].ny, m_model[i].nz);

		indices[i] = i;
	}
#endif

	// Set up the description of the static vertex buffer.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth           = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags      = 0;
	vertexBufferDesc.MiscFlags           = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem          = vertices;
	vertexData.SysMemPitch      = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
		return false;

	// Set up the description of the static index buffer.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth           = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags      = 0;
	indexBufferDesc.MiscFlags           = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem          = indices;
	indexData.SysMemPitch      = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
		return false;

	// Release the arrays now that the vertex and index buffers have been created and loaded.
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

bool Model::loadModel(const char* filename)
{
	std::ifstream fin;
	fin.open(filename);
	if (fin.fail())
		return false;

	// Read up to the value of vertex count.
	char input;
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// Read in the vertex count.
	fin >> m_vertexCount;

	// Set the number of indices to be the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Create the model using the vertex count that was read in.
	m_model = new ModelType[m_vertexCount];
	if (!m_model)
	{
		return false;
	}

	// Read up to the beginning of the data.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	// Read in the vertex data.
	for (int i = 0; i < m_vertexCount; i++)
	{
		fin >> m_model[i].x >> m_model[i].y >> m_model[i].z;
		fin >> m_model[i].tu >> m_model[i].tv;
		fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
	}

	fin.close();

	return true;
}

void Model::releaseModel()
{
	if (m_model)
	{
		delete[] m_model;
		m_model = 0;
	}
}