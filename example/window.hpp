#include "../src/menu.hpp"

class DemoWindow {
public:
	Menu menu;

public:
	DemoWindow();
	~DemoWindow();

	void process();

private:
	bool create_window();
	void init_backends();
	void setup_imgui();

	void new_frame();
	void end_frame();

	void shutdown_imgui();
	void destroy_window();
};