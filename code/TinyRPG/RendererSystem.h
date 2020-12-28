#pragma once

class RendererSystem final
{
public:
	bool Init() noexcept;
	void Close() noexcept;

	void BeginFrame() noexcept;
	void EndFrame() noexcept;

private:
	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_deviceContext = nullptr;

	int m_videoCardMemory;
	char m_videoCardDescription[128];
	bool m_vsync_enabled = false;
};