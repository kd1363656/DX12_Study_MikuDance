#include "Dx12Wrapper.h"

#include "../Application/Application.h"

Dx12Wrapper::Dx12Wrapper(HWND hwnd)
{
#ifdef _DEBUG
	ID3D12Debug* debugLayer = nullptr;
	auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));

	debugLayer->EnableDebugLayer();
	debugLayer->Release();
#endif 

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
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(mDevice.ReleaseAndGetAddressOf())) == S_OK)
		{
			featureLevel;
			break;
		}
	}

	// ファクトリーの初期化
#ifdef _DEBUG
	result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(mDXGIFactory.ReleaseAndGetAddressOf()));
#else 
	result = CreateDXGIFactory1(IID_PPV_ARGS(mDxgiFactory.ReleaseAndGetAddressOf()));
#endif

	if (FAILED(result))
	{
		assert(false && "ファクトリー作成失敗");
		return;
	}

	// コマンドアロケータの作成
	result = mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mCmdAllocator.ReleaseAndGetAddressOf()));

	if (FAILED(result))
	{
		assert(false && "コマンドアロケーター作成失敗");
		return;
	}

	// コマンドリストを作製
	result = mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCmdAllocator.Get(), nullptr, IID_PPV_ARGS(mCmdList.ReleaseAndGetAddressOf()));

	if (FAILED(result))
	{
		assert(false && "コマンドリスト作成失敗");
		return;
	}

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};

	// タイムアウトなし
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	// アダプターを一つ使うと0で大丈夫
	cmdQueueDesc.NodeMask = 0;

	// プライオリティは指定しない
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

	// コマンドリストに合わせる
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	result = mDevice->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(mCmdQueue.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		assert(false && "コマンドキュー作成失敗");
		return;
	}

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

	swapchainDesc.Width = Application::Instance().GetWindowSize().cx;
	swapchainDesc.Height = Application::Instance().GetWindowSize().cy;
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

	result = mDXGIFactory->CreateSwapChainForHwnd(mCmdQueue.Get(), hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)mSwapChain.ReleaseAndGetAddressOf());

	if (FAILED(result))
	{
		assert(false && "スワップチェイン作成失敗");
		return;
	}

	// デスクリプタヒープの作成(レンダーターゲット用)
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	result = mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mRtvHeaps.ReleaseAndGetAddressOf()));

	if (FAILED(result))
	{
		assert(false && "ディスクリプタヒープ作成失敗");
		return;
	}

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = mSwapChain->GetDesc(&swcDesc);

	if (FAILED(result))
	{
		assert(false && "スワップチェーンパラメータ取得失敗");
		return;
	}

	mBackBuffers.resize(swcDesc.BufferCount);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	D3D12_CPU_DESCRIPTOR_HANDLE handle = mRtvHeaps->GetCPUDescriptorHandleForHeapStart();

	for (int idx = 0; idx < swcDesc.BufferCount; ++idx)
	{
		result = mSwapChain->GetBuffer(idx, IID_PPV_ARGS(&mBackBuffers[idx]));

		if (FAILED(result))
		{
			assert(false && "レンダーターゲットとスワップチェーンの紐づけ失敗");
			return;
		}

		mDevice->CreateRenderTargetView(mBackBuffers[idx].Get(), &rtvDesc, handle);
		handle.ptr += mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}


	if (!mViewport)
	{
		mViewport = std::make_unique<D3D12_VIEWPORT>();
	}

	// ビューポートとシザー矩形
	*mViewport = {};
	mViewport->Width = Application::Instance().GetWindowSize().cx;
	mViewport->Height = Application::Instance().GetWindowSize().cy;
	mViewport->TopLeftX = 0;
	mViewport->TopLeftY = 0;
	mViewport->MaxDepth = 1.0f;
	mViewport->MinDepth = 0.0f;

	if (!mScissorRect)
	{
		mScissorRect = std::make_unique<D3D12_RECT>();
	}

	mScissorRect->top = 0;
	mScissorRect->left = 0;
	mScissorRect->right = mScissorRect->left + Application::Instance().GetWindowSize().cx;
	mScissorRect->bottom = mScissorRect->top + Application::Instance().GetWindowSize().cy;

	result = mDevice->CreateFence(mFenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(mFence.ReleaseAndGetAddressOf()));

	if (FAILED(result))
	{
		assert(false && "フェンス作成失敗");
		return;
	}
}

Dx12Wrapper::~Dx12Wrapper()
{
}

void Dx12Wrapper::ShowErrorMessage(HRESULT result, ID3DBlob* errorBlob)
{
	if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
	{
		OutputDebugStringA("ファイルが見つかりません");
		return;
	}
	else
	{
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
		errstr += "\n";

		OutputDebugStringA(errstr.c_str());
	}
}

void Dx12Wrapper::Clear()
{
	int bbIdx = mSwapChain->GetCurrentBackBufferIndex();

	D3D12_CPU_DESCRIPTOR_HANDLE rtvH = mRtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIdx * mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = mBackBuffers[bbIdx].Get();

	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	mCmdList->ResourceBarrier(1, &BarrierDesc);

	mCmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

	float clearColor[] = { 0.0F, 0.0F, 0.0F, 1.0F };
	mCmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
}

void Dx12Wrapper::Update()
{
	int bbIdx = mSwapChain->GetCurrentBackBufferIndex();

	mCmdList->RSSetViewports(1, mViewport.get());
	mCmdList->RSSetScissorRects(1, mScissorRect.get());
}

void Dx12Wrapper::EndDraw()
{
	int bbIdx = mSwapChain->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = mBackBuffers[bbIdx].Get();

	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	mCmdList->Close();

	ID3D12CommandList* cmdlists[] = { mCmdList.Get() };
	mCmdQueue->ExecuteCommandLists(1, cmdlists);

	mCmdList->ResourceBarrier(1, &BarrierDesc);

	mCmdQueue->Signal(mFence.Get(), ++mFenceVal);

	if (mFence->GetCompletedValue() != mFenceVal)
	{
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);

		mFence->SetEventOnCompletion(mFenceVal, event);

		WaitForSingleObject(event, INFINITE);

		CloseHandle(event);
	}

	mCmdAllocator->Reset();
	mCmdList->Reset(mCmdAllocator.Get(), nullptr);

	mSwapChain->Present(1, 0);
}