#include "Billboard.h"

#include "Main/Application.h"
#include "Module/ModuleProgram.h"
#include "Module/ModuleTexture.h"
#include "Importer/MeshImporter.h"

Billboard::Billboard(const std::string& texture_path, float width, float height) : width(width), height(height)
{
	billboard_texture = App->texture->LoadTexture(texture_path.c_str()).get();
	billboard_quad = App->mesh_importer->Load(PRIMITIVE_QUAD_PATH).get();
}


Billboard::~Billboard()
{
	// TODO: Check if not deleting billboard_quad causes a memory leak.
}

void Billboard::Render(const float3& position) const
{
	GLuint shader_program = App->program->GetShaderProgramId("Billboard");
	glUseProgram(shader_program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, billboard_texture->opengl_texture);
	glUniform1i(glGetUniformLocation(shader_program, "billboard.texture"), 0);
	glUniform1f(glGetUniformLocation(shader_program, "billboard.width"), width);
	glUniform1f(glGetUniformLocation(shader_program, "billboard.height"), height);
	glUniform3fv(glGetUniformLocation(shader_program, "billboard.center_pos"), 1, position.ptr());

	glBindVertexArray(billboard_quad->GetVAO());
	glDrawElements(GL_TRIANGLES, billboard_quad->indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glUseProgram(0);
}

