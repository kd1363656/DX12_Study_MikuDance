#pragma once
#include "Windows.h"

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

	WNDCLASSEX mWindowClass;
	HWND mHwnd;
	HINSTANCE mhInstance;

	void CreateGameWindow(HWND& hwnd, WNDCLASSEX& windowClass);

	Application(const Application&) = delete;
	void operator=(const Application&) = delete;
};