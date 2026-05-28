#pragma once

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include <stdio.h>
#include <utility>
#include <cstdlib>

#include "app/application.h"

class DemoApplication: public Application {
	bool show_demo_window = true;
	bool show_another_window = false;
	bool show_star = true;
public:
	void draw() override;
	void demoWindow();
	void starWindow();
};


//--------------------------------------------------impl--------------------------------------------------

void DemoApplication::draw() {
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);
	demoWindow();
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}
	if (show_star)
		starWindow();
}

void DemoApplication::demoWindow() {
	ImGui::Begin("lol");                          // Create a window called "Hello, world!" and append into it.

	static float f = 0.0f;
	static int counter = 0;

	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
	ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
	ImGui::Checkbox("Another Window", &show_another_window);

	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		counter++;
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);

	//ImGuiIO &io = ImGui::GetIO();
	//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::Text("I just lost ma dawg !! %f", f * f);
	ImGui::End();
}

void DemoApplication::starWindow() {
	ImGui::Begin("Star", &show_star);
	ImVec2 avail_size = ImGui::GetContentRegionAvail();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImDrawList *drawlist = ImGui::GetWindowDrawList();

	//IMGUI_API void  AddLine(const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness = 1.0f);
	drawlist->AddLine(
			{pos.x, pos.y}, 
			{pos.x + avail_size.x, pos.y + avail_size.y},
			IM_COL32(10, 100, 100, 255), 1.0f);
	drawlist->AddLine(
			{pos.x, 		pos.y + avail_size.y}, 
			{pos.x + avail_size.x, pos.y},
			IM_COL32(10, 100, 100, 255), 1.0f);
	ImGui::End();
}
