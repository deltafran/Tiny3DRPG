#pragma once

class GraphicShaders
{
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};
public:
	bool Init();
	void Close();
	bool Render(int, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*);
private:
	bool initializeShader(const wchar_t*, const wchar_t*);
	void outputShaderErrorMessage(ID3D10Blob*, const wchar_t*);

	bool setShaderParameters(XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*);
	void renderShader(int);

	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout* m_layout = nullptr;
	ID3D11Buffer* m_matrixBuffer = nullptr;
	ID3D11SamplerState* m_sampleState = nullptr;
};