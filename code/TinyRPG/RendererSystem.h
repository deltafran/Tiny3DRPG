#pragma once

class RendererSystem final
{
public:
	bool Init() noexcept;
	void Close() noexcept;

	void BeginFrame() noexcept;
	void EndFrame() noexcept;

	ID3D11Device* GetDevice() noexcept;
	ID3D11DeviceContext* GetDeviceContext() noexcept;

	void GetProjectionMatrix(XMMATRIX&) noexcept;
	void GetOrthoMatrix(XMMATRIX&) noexcept;

	void GetVideoCardInfo(char*, int&) noexcept;

private:
	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_deviceContext = nullptr;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11RasterizerState* m_rasterState;
	XMMATRIX m_projectionMatrix;
	XMMATRIX m_orthoMatrix;

	int m_videoCardMemory;
	char m_videoCardDescription[128];
};