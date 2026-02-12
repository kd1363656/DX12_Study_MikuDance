#include "Windows.h"
#include <tchar.h>

#include <d3d12.h>
#include <dxgi1_6.h>

#include <wrl/client.h>

#include <cassert>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;

const int window_width = 1600;
const int window_height = 800;

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int main()
{
	// メモリリークを知らせる
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// COM初期化
	if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
	{
		CoUninitialize();

		return -1;
	}

	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;
	w.lpszClassName = _T("DX12Sample");
	w.hInstance = GetModuleHandle(nullptr);

	RegisterClassEx(&w);

	RECT wrc = { 0, 0, window_width, window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	HWND hwnd = CreateWindow(w.lpszClassName,
		_T("DX12test"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr);

	ShowWindow(hwnd, SW_SHOW);

	MSG msg = {};

	ComPtr<ID3D12Device> _dev = nullptr;

	// デバイスの初期化
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	D3D_FEATURE_LEVEL featureLevel;

	for (auto lv : levels)
	{
		if(D3D12CreateDevice(nullptr, lv , IID_PPV_ARGS(_dev.ReleaseAndGetAddressOf())) == S_OK)
		{
			featureLevel;
			break;
		}
	}

	ComPtr<IDXGIFactory6> _dxgiFactory = nullptr;

	// ファクトリーの初期化
#ifdef _DEBUG
	HRESULT result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf()));
#else 
	HRESULT result = CreateDXGIFactory1(IID_PPV_ARGS(mDxgiFactory.ReleaseAndGetAddressOf()));
#endif

	if (FAILED(result))
	{
		assert(false && "ファクトリー作成失敗");
		return -1;
	}

	ComPtr<ID3D12CommandAllocator> _cmdAllocator = nullptr;
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(_cmdAllocator.ReleaseAndGetAddressOf()));

	if (FAILED(result))
	{
		assert(false && "コマンドアロケーター作成失敗");
		return -1;
	}

	ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator.Get(), nullptr, IID_PPV_ARGS(_cmdList.ReleaseAndGetAddressOf()));

	if (FAILED(result))
	{
		assert(false && "コマンドリスト作成失敗");
		return -1;
	}

	ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};

	// タイムアウトなし
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	// アダプターを一つ使うと0で大丈夫
	cmdQueueDesc.NodeMask = 0;

	// プライオリティは指定しない
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

	// コマンドリストに合わせる
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(_cmdQueue.ReleaseAndGetAddressOf()));

	if (FAILED(result))
	{
		assert(false && "コマンドキュー作成失敗");
		return -1;
	}

	ComPtr<IDXGISwapChain4> _swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2; // ダブルバッファリングなので2

	// バックバッファは伸び縮み可能
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;

	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	// ウィンドウモードとフルスクリーンモード切替可能
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	result = _dxgiFactory->CreateSwapChainForHwnd(_cmdQueue.Get(), hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)_swapChain.ReleaseAndGetAddressOf());

	if (FAILED(result))
	{
		assert(false && "スワップチェイン作成失敗");
		return -1;
	}

	while(true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
		{
			break;
		}
	}

	UnregisterClass(w.lpszClassName, w.hInstance);

	// COM解放
	CoUninitialize();

	return 0;
}