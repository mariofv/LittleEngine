#include "ComponentBoxPrimitive.h"
#include "Main/Application.h"
#include "Module/ModulePhysics.h"


btRigidBody* ComponentBoxPrimitive::AddBody()
{
	if (body) {
		App->physics->world->removeRigidBody(body);
	}
	
	col_shape = new btBoxShape(btVector3(box_size)); // regular box
	deviation = owner->aabb.global_bounding_box.CenterPoint() - owner->transform.GetGlobalTranslation();
	motion_state = new btDefaultMotionState(btTransform(btQuaternion(owner->transform.rotation.x, owner->transform.rotation.y, owner->transform.rotation.z, owner->transform.rotation.w), btVector3(owner->aabb.global_bounding_box.CenterPoint().x, owner->aabb.global_bounding_box.CenterPoint().y, owner->aabb.global_bounding_box.CenterPoint().z)));
		
	btVector3 localInertia(0.f, 0.f, 0.f);
	if (mass != 0.f) col_shape->calculateLocalInertia(mass, localInertia);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, col_shape, localInertia);
	body = new btRigidBody(rbInfo);
	
	App->physics->world->addRigidBody(body);
	body->applyCentralForce(btVector3(0, 1, 0));
	
	return body;
}

ComponentBoxPrimitive::ComponentBoxPrimitive(ComponentType componentType):Component(componentType)
{
	AddBody();
}

ComponentBoxPrimitive::ComponentBoxPrimitive(GameObject* owner, ComponentType componentType): Component(owner, componentType)
{
	//size will be bounding_box size already computed

	box_size = btVector3(owner->aabb.bounding_box.Size().x/2, owner->aabb.bounding_box.Size().y/2, owner->aabb.bounding_box.Size().z/2);

	AddBody();
}

Component* ComponentBoxPrimitive::Clone(bool original_prefab) const
{
	return nullptr;
}

void ComponentBoxPrimitive::Copy(Component* component_to_copy) const
{
}

void ComponentBoxPrimitive::Delete()
{

	App->physics->world->removeRigidBody(body);
	App->physics->RemoveComponentBoxPrimitive(this);
}

void ComponentBoxPrimitive::Save(Config & config) const
{
	config.AddUInt(UUID, "UUID");
	config.AddBool(active, "Active");
	config.AddUInt((unsigned int)type, "ComponentType");
	config.AddFloat(mass, "Mass");

}

void ComponentBoxPrimitive::Load(const Config & config)
{
	UUID = config.GetUInt("UUID", 0);
	active = config.GetBool("Active", true);
	mass = config.GetFloat("Mass", 1.0F);
	AddBody();
}

void ComponentBoxPrimitive::MakeBoxStatic() {

	if (is_static) 
	{
		body->setMassProps(0, body->getLocalInertia());
	}
	else {
		btVector3 localInertia(0.f, 0.f, 0.f);
		col_shape->calculateLocalInertia(1, localInertia);
		body->setMassProps(1, localInertia);
	}

}

void ComponentBoxPrimitive::MoveBody()
{
	btTransform trans;
	motion_state->getWorldTransform(trans);
	owner->transform.SetGlobalMatrixTranslation(float3(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ()) - deviation);
	owner->transform.SetRotation(Quat(trans.getRotation().x(), trans.getRotation().y(), trans.getRotation().z(), trans.getRotation().w()));
}

void ComponentBoxPrimitive::UpdateBoxDimensions()
{
	// This should only happen if the transform change
	Quat global_rotation = owner->transform.GetGlobalRotation();	
	float3 global_scale = owner->transform.GetGlobalScale();
	
	motion_state->setWorldTransform(btTransform(btQuaternion(global_rotation.x, global_rotation.y, global_rotation.z, global_rotation.w), btVector3(owner->aabb.global_bounding_box.CenterPoint().x, owner->aabb.global_bounding_box.CenterPoint().y, owner->aabb.global_bounding_box.CenterPoint().z)));
	body->setMotionState(motion_state);
	body->getCollisionShape()->setLocalScaling(btVector3(global_scale.x, global_scale.y, global_scale.z));
	deviation = owner->aabb.global_bounding_box.CenterPoint() - owner->transform.GetGlobalTranslation();
}

