#pragma once
#include "imgui.h"

#include <windows.h>
#include <stdio.h>


#define LOG(format, ...) log(__FILE__, __LINE__, format, __VA_ARGS__);

void log(const char file[], int line, const char* format, ...);

static void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

enum update_status
{
	UPDATE_CONTINUE = 1,
	UPDATE_STOP,
	UPDATE_ERROR
};

enum class Fonts
{
	FONT_FA = 0,
	FONT_FAR = 1,
	FONT_FAB = 2
};

// Paths -------
# define SHADERS_PATH "./shaders/"
# define TEXTURES_PATH "./resources/textures/"

# define DEFAULT_VERTEX_SHADER_PATH "./shaders/default.vs"
# define DEFAULT_FRAGMENT_SHADER_PATH "./shaders/default.fs"

# define PRIMITIVE_VERTEX_SHADER_PATH "./shaders/primitive.vs"
# define PRIMITIVE_FRAGMENT_SHADER_PATH "./shaders/primitive.fs"

# define TEXTURE_FRAGMENT_SHADER_PATH "./shaders/texture.fs"

# define WOBBLE_VERTEX_SHADER_PATH "./shaders/wobble.vs"
# define WOBBLE_FRAGMENT_SHADER_PATH "./shaders/wobble.fs"

# define LENA_TEXTURE_PATH "./resources/textures/lena.png"
# define HOUSE_MODEL_PATH "./resources/models/baker_house/BakerHouse.fbx"
# define UMBREON_MODEL_PATH "./resources/models/Umbreon/Umbreon.fbx"

#define FULLSCREEN false
#define RESIZABLE true
#define BORDERED true

#define VSYNC true
#define TITLE "Orion Engine"