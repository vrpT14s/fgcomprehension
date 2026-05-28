#pragma once

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include <stdio.h>
#include <utility>
#include <cstdlib>


class Application {
protected:
	static constexpr char *glsl_version = "#version 130";
	SDL_Window *window;
	SDL_GLContext gl_context;

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
public:
	void initSDL();
	void makeWindow(const char *title);
	void setupImGui();

	virtual void draw() = 0;

	int run();
};
