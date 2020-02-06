#ifndef _BILLBOARD_H_
#define _BILLBOARD_H_

#include "Resource/Mesh.h"
#include "Resource/Texture.h"

#include <string>
#include <MathGeoLib.h>
#include <GL/glew.h>

class Billboard
{
public:
	Billboard(const std::string& texture_path, float width, float height);
	~Billboard();

	void Render(const float3& position) const;

private:
	Mesh* billboard_quad;

	float width = 5.f;
	float height = 5.f;
	Texture* billboard_texture = nullptr;

};

#endif //_BILLBOARD_H_