#pragma once
#include "Windows.h"
#include <memory>

class Render;
class Dx12Wrapper;

class Application
{
public:

	Application () = default;
	~Application() = default;

	bool Init();
	void Run();
	void Terminate();
	SIZE GetWindowSize() const;

	static Application& Instance()
	{
		static Application instance = {};
		return instance;
	}

private:

	static const int window_width = 1600;
	static const int window_height = 800;

	std::shared_ptr<Dx12Wrapper> mDX12Wrapper = nullptr;
	std::shared_ptr<Render> mRender = nullptr;

	WNDCLASSEX mWindowClass;
	HWND mHwnd;
	HINSTANCE mhInstance;

	bool CreateGameWindow();

	Application(const Application&) = delete;
	void operator=(const Application&) = delete;
};