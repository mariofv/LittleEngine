#ifndef _COMPONENTCANVAS_H_
#define _COMPONENTCANVAS_H_

#include "Component/ComponentCamera.h"

#include <GL/glew.h>
#include <MathGeoLib.h>

class ComponentCanvasRenderer;

class ComponentCanvas : public Component
{
public:
	ComponentCanvas();
	ComponentCanvas(GameObject * owner);
	~ComponentCanvas() = default;

	ComponentCanvas & operator=(const ComponentCanvas& component_to_copy);
	ComponentCanvas & operator=(ComponentCanvas&& component_to_move) = default;

	Component* Clone(bool original_prefab = false) const override;
	void Copy(Component* component_to_copy) const override;
	void Delete() override;

	void Render(bool scene_mode = false);

	void SpecializedSave(Config& config) const override;
	void SpecializedLoad(const Config& config) override;

private:
	std::vector<ComponentCanvasRenderer*> GetComponentCanvasRendererToRender() const;
	
};

#endif //_COMPONENTCANVAS_H_

