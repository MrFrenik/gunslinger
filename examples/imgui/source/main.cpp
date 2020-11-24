#include <gs.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

/*
	Simple c++ imgui example
*/

// Forward Decls.
gs_result app_init();
gs_result app_update();		// Use to update your application

void imgui_init();
void imgui_new_frame();
void imgui_render();

// Globals
b8 g_show_demo_window = true;

int main(int argc, char** argv)
{
	// This is our app description. It gives internal hints to our engine for various things like 
	// window size, title, as well as update, init, and shutdown functions to be run. 
	gs_application_desc_t app = {0};
	app.window_title 		= "ImGui";
	app.window_width 		= 800;
	app.window_height 		= 600;
	app.init 				= &app_init;
	app.update 				= &app_update;

	// Construct internal instance of our engine
	gs_engine_t* engine = gs_engine_construct(app);

	// Run the internal engine loop until completion
	gs_result res = engine->run();

	// Check result of engine after exiting loop
	if (res != gs_result_success) 
	{
		gs_println("Error: Engine did not successfully finish running.");
		return -1;
	}

	gs_println("Gunslinger exited successfully.");

	return 0;	
}

gs_result app_init()
{
	imgui_init();

	return gs_result_success;
}

// Update your application here
gs_result app_update()
{
	// Grab global instance of engine
	gs_engine_t* engine = gs_engine_instance();

	// If we press the escape key, exit the application
	if (engine->ctx.platform->key_pressed(gs_keycode_esc))
	{
		return gs_result_success;
	}

	imgui_new_frame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    ImGui::ShowDemoWindow(&g_show_demo_window);

    // Draw all imgui data
    imgui_render();

	// Otherwise, continue
	return gs_result_in_progress;
}

void imgui_init()
{
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	// Get main window from platform
	void* win = platform->raw_window_handle(platform->main_window());

	 // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 
    (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init();
}

void imgui_new_frame()
{
	// Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void imgui_render()
{
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

    // TODO(john): abstract this all away to be rendered via command buffer system
    ImGui::Render();
    gs_vec2 fbs = platform->frame_buffer_size(platform->main_window());
    glViewport(0, 0, fbs.x, fbs.y);
    glClearColor(0.1f, 0.1f, 0.1f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}






