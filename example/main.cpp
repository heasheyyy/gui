#include "window.hpp"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	DemoWindow demo;
	demo.process();

	return 0;
}
