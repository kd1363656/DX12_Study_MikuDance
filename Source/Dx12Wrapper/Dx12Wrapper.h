#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include <wrl/client.h>

#include <DirectXMath.h>

#include <string>
#include <vector>

#include <memory>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class Dx12Wrapper
{
private:

	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

public:

	Dx12Wrapper(HWND hwnd);
	~Dx12Wrapper();

	void ShowErrorMessage(HRESULT result, ID3DBlob* errorBlob);

	void Clear();
	void Update();
	void EndDraw();

	ComPtr<ID3D12Device> Device() const { return mDevice; }
	ComPtr<ID3D12GraphicsCommandList> CommandList() const { return mCmdList; }
	ComPtr<IDXGISwapChain4> SwapChain() const { return mSwapChain; }

	DirectX::XMMATRIX GetViewMatrix() const;
	DirectX::XMMATRIX GetProjectionMatrix() const;

private:

	HRESULT InitializeDXGIDevice();
	HRESULT InitializeCommand();
	HRESULT CreateSwapChain(const HWND& hwnd);

	SIZE mWindowSize;

	ComPtr<IDXGIFactory6> mDXGIFactory = nullptr;
	ComPtr<ID3D12Device> mDevice = nullptr;
	ComPtr<ID3D12CommandAllocator> mCmdAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> mCmdList = nullptr;
	ComPtr<ID3D12CommandQueue> mCmdQueue = nullptr;
	ComPtr<IDXGISwapChain4> mSwapChain = nullptr;
	ComPtr<ID3D12DescriptorHeap> mRtvHeaps = nullptr;
	std::vector<ComPtr<ID3D12Resource>> mBackBuffers;
	std::unique_ptr<D3D12_VIEWPORT> mViewport;
	std::unique_ptr<D3D12_RECT> mScissorRect;
	ComPtr<ID3D12Fence> mFence = nullptr;
	UINT64 mFenceVal = 0;
};