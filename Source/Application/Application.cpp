#include "Application.h"

#include <tchar.h>

#include <wrl/client.h>

#include "../Render/Render.h"
#include "../Dx12Wrapper/Dx12Wrapper.h"

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
	auto& app = Application::Instance();
	if (!app.Init())
	{
		return -1;
	}

	app.Run();
	app.Terminate();
	return 0;
}

bool Application::Init()
{
	// メモリリークを知らせる
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// COM初期化
	if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
	{
		CoUninitialize();

		return false;
	}

	if (!CreateGameWindow())
	{
		return false;
	}

	if (!mDX12Wrapper)
	{
		mDX12Wrapper = std::make_shared<Dx12Wrapper>(mHwnd);
	}

	if (!mRender)
	{
		mRender = std::make_shared<Render>();
	}

	return true;
}

void Application::Run()
{
	MSG msg = {};

	while (true)
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

		mDX12Wrapper->Clear();
		mDX12Wrapper->Update();
		mDX12Wrapper->EndDraw();

		mRender->Frame(); // 毎フレームごとに呼ぶ
	}
}

void Application::Terminate()
{
	UnregisterClass(mWindowClass.lpszClassName, mWindowClass.hInstance);

	// COM解放
	CoUninitialize();
}

SIZE Application::GetWindowSize() const
{
	SIZE size = {window_width, window_height};
	return size;
}

bool Application::CreateGameWindow()
{
	mWindowClass.cbSize = sizeof(WNDCLASSEX);
	mWindowClass.lpfnWndProc = (WNDPROC)WindowProcedure;
	mWindowClass.lpszClassName = _T("DX12Sample");
	mWindowClass.hInstance = GetModuleHandle(nullptr);

	mhInstance = GetModuleHandle(nullptr);

	RegisterClassEx(&mWindowClass);

	RECT wrc = { 0, 0, window_width, window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	mHwnd = CreateWindow(mWindowClass.lpszClassName,
		_T("DX12test"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		mWindowClass.hInstance,
		nullptr);

	ShowWindow(mHwnd, SW_SHOW);

	return true;
}