#include "EnemyController.h"

#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleAI.h"

#include "EditorUI/Panel/InspectorSubpanel/PanelComponent.h"

#include "imgui.h"

EnemyController* EnemyControllerDLL()
{
	EnemyController* instance = new EnemyController();
	return instance;
}

EnemyController::EnemyController()
{
	panel = new PanelComponent();
}

// Use this for initialization before Start()
void EnemyController::Awake()
{
	//EnemyManager::GetInstance()->AddEnemy(this);
	animation = (ComponentAnimation*)owner->children[0]->children[0]->GetComponent(Component::ComponentType::ANIMATION);
}

// Use this for initialization
void EnemyController::Start()
{

}

// Update is called once per frame
void EnemyController::Update()
{
	Move();
}

// Use this for showing variables on inspector
void EnemyController::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

	ImGui::Text("Player Movement Script Inspector");
	//Example Showing variables and being able to modify it on Runtime.
	ImGui::DragFloat("Speed", &move_speed, 0.1f, 0.f, 0.5f);
}

//Use this for linking JUST GO automatically 
void EnemyController::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size
	public_gameobjects.push_back(&player);
	variable_names.push_back(GET_VARIABLE_NAME(player));

	for (int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}

void EnemyController::TakeDamage(float damage)
{
	health_points -= damage;

	if (health_points <= 0)
	{
		Die();
	}
}

void EnemyController::Move()
{
	const float3 player_transform = player->transform.GetTranslation();
	float3 transform = owner->transform.GetTranslation();

	float3 direction = player_transform - transform;

	if (player_transform.Distance(transform) > stopping_distance)
	{
		float3 position = transform + (direction.Normalized() * move_speed);

		float3 next_position;
		bool valid_position = App->artificial_intelligence->FindNextPolyByDirection(position, next_position);

		if (valid_position)
		{
			position.y = next_position.y;
		}

		if (App->artificial_intelligence->IsPointWalkable(position))
		{
			owner->transform.LookAt(position);
			owner->transform.SetTranslation(position);
		}
	}
}

void EnemyController::Die()
{
	//TODO spawn particles, loot, etc.
	is_dead = true;
}
