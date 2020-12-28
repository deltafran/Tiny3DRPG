#pragma once

class Texture
{
public:
	bool Create(const char*);
	void Close();

	ID3D11ShaderResourceView* GetTexture();

private:
	bool loadTGA(const char*, int&, int&);

	unsigned char* m_targaData = nullptr;
	ID3D11Texture2D* m_texture = nullptr;
	ID3D11ShaderResourceView* m_textureView = nullptr;
};