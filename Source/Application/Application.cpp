#include "Application.h"

#include <tchar.h>

#include <wrl/client.h>

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
	// ÉÅÉÇÉäÉäÅ[ÉNÇímÇÁÇπÇÈ
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// COMèâä˙âª
	if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
	{
		CoUninitialize();

		return -1;
	}

}

void Application::Run()
{
}

void Application::Terminate()
{
	UnregisterClass(mWindowClass.lpszClassName, mWindowClass.hInstance);

	// COMâï˙
	CoUninitialize();
}

SIZE Application::GetWindowSize() const
{
	return SIZE();
}

void Application::CreateGameWindow(HWND& hwnd, WNDCLASSEX& windowClass)
{
	mWindowClass.cbSize = sizeof(WNDCLASSEX);
	mWindowClass.lpfnWndProc = (WNDPROC)WindowProcedure;
	mWindowClass.lpszClassName = _T("DX12Sample");
	mWindowClass.hInstance = GetModuleHandle(nullptr);

	mhInstance = GetModuleHandle(nullptr);

	RegisterClassEx(&mWindowClass);

	RECT wrc = { 0, 0, window_width, window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	HWND hwnd = CreateWindow(mWindowClass.lpszClassName,
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

	ShowWindow(hwnd, SW_SHOW);
}