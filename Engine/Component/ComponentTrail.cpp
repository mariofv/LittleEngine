#include "ComponentTrail.h"

#include "Main/Application.h"
#include "Module/ModuleEffects.h"
#include "Module/ModuleProgram.h"
#include "Module/ModuleResourceManager.h"
#include "Module/ModuleRender.h"
#include "Module/ModuleTime.h"
#include "GL/glew.h"

#include "ResourceManagement/ResourcesDB/CoreResources.h"

namespace { const float MAX_TRAIL_VERTICES = 1000; } //arbitrary number 

ComponentTrail::ComponentTrail() : Component(nullptr, ComponentType::TRAIL)
{
	Init();
}

ComponentTrail::ComponentTrail(GameObject * owner) : Component(owner, ComponentType::TRAIL)
{
	Init();
}
ComponentTrail::~ComponentTrail()
{
	if (trail_vbo != 0)
	{
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glDeleteBuffers(1, &trail_vbo);
		glDeleteBuffers(1, &trail_vao);
	}
	App->effects->RemoveComponentTrail(this);
}

void ComponentTrail::Init()
{
	gameobject_init_position = owner->transform.GetGlobalTranslation(); //initial GO position
	last_point = TrailPoint(gameobject_init_position, width, duration);

	//InitRenderer
	ChangeTexture(static_cast<uint32_t>(CoreResource::BILLBOARD_DEFAULT_TEXTURE));

	glGenVertexArrays(1, &trail_vao);
	glGenBuffers(1, &trail_vbo);

	glBindVertexArray(trail_vao);

	glBindBuffer(GL_ARRAY_BUFFER, trail_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MAX_TRAIL_VERTICES * 4 * 5, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void ComponentTrail::UpdateTrail()
{
	float3 gameobject_position = owner->transform.GetGlobalTranslation(); //current GO position

	if (gameobject_init_position.x != gameobject_position.x || gameobject_init_position.y != gameobject_position.y || gameobject_init_position.z != gameobject_position.z)//if user moves Trail GO
		on_transform_change = true;
	
	if (on_transform_change)//always gets in this is wrong ! TODO create an event here/ merge with event system
	{
		head_point = TrailPoint(gameobject_position, width, duration); //If there is movement, update the head point on the GO's position
		head_point.life = duration; // If the GO moves, we reset the life of the head point
		
		if (head_point.position.Distance(last_point.position) >= min_distance) //If the head point is at a greater distance than the distance we specified...
		{
			TrailPoint next_point(head_point.position, width, duration);
			test_points.push_back(next_point);	//create another Trail point and add it to the pool		
			last_point = next_point; // So we're gonna calculate, on the next iteration how far we are from the last point created, and so on
		}
	}

	GetPerpendiculars();

	for (auto it = test_points.begin(); it < test_points.end(); ++it)
	{
		if (it->life >= 0) // If life is positive, all good
		{
			it->is_rendered = true;
			it->life -= App->time->real_time_delta_time; // Update time left
		}

		else // But if not, we delete these points
		{
			it = test_points.erase(it);
		}
	}
}

void  ComponentTrail::GetPerpendiculars()
{
	unsigned int destroy_point_vertice = 0;
	TrailPoint* previous_point = nullptr;
	TrailPoint* current_point = nullptr;
	mesh_points.clear();
	float mesh_index = 0.0f;
	
	for (int i = 0; i < test_points.size(); ++i)
	{
		current_point = &test_points[i];
		if (previous_point != nullptr)
		{
			mesh_points.push_back(std::make_pair(previous_point, current_point));
		}
		previous_point = current_point;
	}

	std::vector<Vertex> vertices;
	
	unsigned int j = 0;
	mesh_index = 1 / (float)test_points.size(); // to coordinate texture
	for (auto pair = mesh_points.begin(); pair < mesh_points.end(); ++pair)
	{
		if (pair->first->life > 0 && pair->second->life > 0)
		{
			//Get the vector that links every two points
			float3 vector_adjacent = (pair->second->position - pair->first->position).Normalized();//vector between each pair -> Normalized to get vector with magnitutde = 1 but same direction
			float3 perpendicular = vector_adjacent.Cross(owner->transform.GetFrontVector()) * width; //Front is currently local

			float3 top_left, bottom_left;
			++j;
			//Commented code is for rendering the last point of path
			//if (++pair == mesh_points.end())
			//{
			//	/*--pair;
			//	top_left = pair->second->position + perpendicular;
			//	bottom_left = (pair->second->position - perpendicular);*/
			//}
			//else
			//{
			//	top_left = pair->first->position + perpendicular;
			//	bottom_left = (pair->first->position - perpendicular);
			//}
			top_left = pair->first->position + perpendicular;
			bottom_left = (pair->first->position - perpendicular);
			vertices.push_back({ top_left, float2(mesh_index * j,1.0f) }); //uv[i]
			vertices.push_back({ bottom_left, float2(mesh_index * j,0.0f) });//uv[++i]
		}
		else
		{
			mesh_points.erase(pair);
		}
	}
	Render(vertices);
}

void ComponentTrail::Render(std::vector<Vertex>& to_render)
{
	if (active)
	{

		GLuint shader_program = App->program->UseProgram("Trail");
		glUseProgram(shader_program);

		glBindVertexArray(trail_vao);

		//use glBufferMap to obtain a pointer to buffer data
		glBindBuffer(GL_ARRAY_BUFFER, trail_vbo);
		trail_renderer_vertices = (float*)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(Vertex) *  to_render.size(), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);// 6 indices
		memcpy(trail_renderer_vertices, to_render.data(), to_render.size() * sizeof(Vertex));
		glUnmapBuffer(GL_ARRAY_BUFFER);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, trail_texture->opengl_texture);
		glUniform1i(glGetUniformLocation(shader_program, "tex"), 0);

		glBindBuffer(GL_UNIFORM_BUFFER, App->program->uniform_buffer.ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, App->program->uniform_buffer.MATRICES_UNIFORMS_OFFSET, sizeof(float4x4), owner->transform.GetGlobalModelMatrix().Transposed().ptr());
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glUniform4fv(glGetUniformLocation(shader_program, "color"), 1, (float*)color);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, to_render.size());
		glBindVertexArray(0);

		glUseProgram(0);
	}
}

void ComponentTrail::SetTrailTexture(uint32_t texture_uuid)
{
	this->texture_uuid = texture_uuid;
	ChangeTexture(texture_uuid);
}

void ComponentTrail::ChangeTexture(uint32_t texture_uuid)
{
	if (texture_uuid != 0)
	{
		this->texture_uuid = texture_uuid;
		trail_texture = App->resources->Load<Texture>(texture_uuid);
	}
}

ComponentTrail& ComponentTrail::operator=(const ComponentTrail& component_to_copy)
{
	Component::operator = (component_to_copy);
	return *this;
}

Component* ComponentTrail::Clone(bool original_prefab) const
{
	ComponentTrail* created_component;
	if (original_prefab)
	{
		created_component = new ComponentTrail();
	}
	else
	{
		created_component = App->effects->CreateComponentTrail(owner);
	}
	*created_component = *this;
	CloneBase(static_cast<Component*>(created_component));
	return created_component;
};

void ComponentTrail::Copy(Component* component_to_copy) const
{
	*component_to_copy = *this;
	*static_cast<ComponentTrail*>(component_to_copy) = *this;
}

void ComponentTrail::Delete()
{
	App->effects->RemoveComponentTrail(this);
}


void ComponentTrail::SpecializedSave(Config& config) const
{
	config.AddUInt(texture_uuid, "TextureUUID");
}
void ComponentTrail::SpecializedLoad(const Config& config)
{
	UUID = config.GetUInt("UUID", 0);
	active = config.GetBool("Active", true);
	texture_uuid = config.GetUInt("TextureUUID", 0);

	ChangeTexture(texture_uuid);
}