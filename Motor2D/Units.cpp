
#include "j1App.h"
#include "j1Input.h"
#include "j1Render.h"
#include "j1Animation.h"
#include "Units.h"
#include "p2Log.h"
#include "j1Pathfinding.h"
#include "j1Map.h"
#include "j1EntityManager.h"

Unit::Unit(UNIT_TYPE u_type, fPoint pos, int id): Entity(UNIT, pos), unit_type(u_type), direction(WEST), action_type(IDLE), id(id)
{
	switch (u_type)
	{
		//ADD UNIT: IF ANY UNIT IS ADDED ADD CODE HERE:
	case TWOHANDEDSWORDMAN:
		SetHp(60);
		attack = 12;
		SetArmor(1);
		speed = 0.9;
		rate_of_fire = 2;
		range = 1;
		unit_class = INFANTRY;
		unit_radius = 6;
		IA = false;
		state = NONE;
		break;

	case TWOHANDEDSWORDMANENEMY:
		SetHp(60);
		attack = 12;
		SetArmor(1);
		speed = 0.9;
		rate_of_fire = 2;
		range = 1;
		unit_class = INFANTRY;
		unit_radius = 6;
		IA = true;
		state = NONE;
		break;


	case CAVALRYARCHER:
		SetHp(50);
		attack = 6;
		SetArmor(1);
		speed = 1.4;
		rate_of_fire = 2;
		range = 4;
		unit_class = ARCHER;
		unit_radius = 12;
		IA = false;
		state = NONE;
		break;

	case SIEGERAM:
		SetHp(270);
		attack = 4;
		SetArmor(-5);
		speed = 0.6;
		rate_of_fire = 5;
		range = 1;
		unit_class = SIEGE;
		IA = false;
		state = NONE;
		break;

	default:
		LOG("Error UNIT TYPE STATS NULL");
		unit_class = NO_CLASS;
		break;
	}
}

void Unit::Update()
{
	if (App->input->GetMouseButtonDown(3) == KEY_DOWN && this->GetEntityStatus() == E_SELECTED)
	{
		this->path_list.clear();
		App->input->GetMousePosition(destination.x, destination.y);

		if (this->GetPath({ destination.x, destination.y }) != -1)
		{
			path_list.pop_front();
			GetNextTile();
			this->action_type = WALK;
			state = MOVING;
		}
		else
		{
			state = NONE;
		}
	}
	else if (App->input->GetKey(SDL_SCANCODE_A) == KEY_DOWN && this->GetEntityStatus() == E_SELECTED)
	{
		App->input->GetMousePosition(destination.x, destination.y);
		destination.x -= App->render->camera.x;
		destination.y -= App->render->camera.y;
		destination = App->map->WorldToMap(destination.x, destination.y);
		if (App->entity_manager->IsUnitInTile(this, destination)) {
			enemy = App->entity_manager->GetUnitInTile(destination);
			state = ATTACKING;
		}
	}

	if (IA)
	{
		AI();
	}

	if(state == MOVING) 
	{
		Move();
	}

	if (state == ATTACKING)
	{
		AttackUnit(enemy);
	}
	Draw();

	if (GetHP() <= 0) {
		state = DEAD;
		this->action_type = DIE;
		//delete this;
		//App->entity_manager->DeleteUnit(this);
	}
}

void Unit::Move()
{
		this->SetPosition(GetX() + move_vector.x*speed, GetY() + move_vector.y*speed);

		iPoint unit_world;
		unit_world.x = GetX();
		unit_world.y = GetY();
		if (path_objective.DistanceTo(unit_world) < 3)
		{
			//center the unit to the tile
			this->SetPosition(path_objective.x, path_objective.y);
			if (!GetNextTile())
			{
				state = NONE;
				this->action_type = IDLE;
			}
		}

		/*if (App->input->GetKey(SDL_SCANCODE_A) == KEY_DOWN && this->GetEntityStatus() == E_SELECTED)
		{
		//if(App->entity_manager->GetUnitInTile())
		this->path_list.clear();
		App->input->GetMousePosition(destination.x, destination.y);
		destination.x -= App->render->camera.x;
		destination.y -= App->render->camera.y;
		if (this->GetPath({ destination.x, destination.y }) != -1)
		{
		attacking = true;
		path_list.pop_front();
		GetNextTile();
		this->action_type = WALK;
		this->moving = true;
		}
		else
		{
		CheckSurroundings();
		attacking = true;
		this->moving = false;
		this->action_type = ATTACK;
		}
		}

		App->render->DrawLine(0, 0, 1000, 0, 255, 255, 255);
		App->render->DrawLine(0, 0, 0, 1000, 255, 255, 255);

		/*if (attacking == true)
		{
		CheckSurroundings();
		}*/
}

void Unit::AI()
{
	if (state != ATTACKING) {
		if (CheckSurroundings())
		{
			if (state == MOVING)
			{
				this->SetPosition(GetX() + move_vector.x*speed, GetY() + move_vector.y*speed);

				iPoint unit_world;
				unit_world.x = GetX();
				unit_world.y = GetY();
				if (path_objective.DistanceTo(unit_world) < 3)
				{
					//center the unit to the tile
					this->SetPosition(path_objective.x, path_objective.y);
					if (!GetNextTile())
					{
						//state == ATTACKING;
						this->action_type = ATTACK;
					}

				}
			}
		}
	}
}

void Unit::Draw()
{
	SDL_Texture* tex = App->anim->GetTexture(unit_type);
	SDL_Rect rect;
	iPoint pivot;

	App->anim->GetAnimationFrame(rect, pivot, this);

	if (direction == NORTH_EAST || direction == EAST || direction == SOUTH_EAST)
		App->render->Blit(tex, GetX(), GetY(), &rect, SDL_FLIP_HORIZONTAL, pivot.x, pivot.y);
	else
		App->render->Blit(tex, GetX() - pivot.x, GetY() - pivot.y, &rect);

	if (this->GetEntityStatus() == E_SELECTED)
		App->render->DrawCircle(this->GetX() + App->render->camera.x, this->GetY() + App->render->camera.y, this->unit_radius, 255, 255, 255);
}

const DIRECTION Unit::GetDir() const
{
	return direction;
}

const UNIT_TYPE Unit::GetUnitType() const
{
	return unit_type;
}

const ACTION_TYPE Unit::GetActionType() const
{
	return action_type;
}

void Unit::SetAction(const ACTION_TYPE action)
{
	action_type = action;
}

int Unit::GetPath(iPoint dest)
{
	iPoint ori = App->map->WorldToMap(GetX(), GetY());
	iPoint destinat = App->map->WorldToMap(dest.x, dest.y);
	return App->pathfinding->CreatePath(ori, destinat, path_list);
}

void Unit::PopFirstPath()
{
	path_list.pop_front();
}

void Unit::AddPath(iPoint new_goal)
{
	path_list.push_back(new_goal);
}

bool Unit::GetNextTile()
{
	bool ret = true;

	if (path_list.size() == 0)
		return false;

	path_objective = App->map->MapToWorld(path_list.front().x, path_list.front().y);
	path_list.pop_front();

	move_vector.x = (float)path_objective.x - GetX();
	move_vector.y = (float)path_objective.y - GetY();
	float module = (sqrt(move_vector.x*move_vector.x + move_vector.y * move_vector.y));
	move_vector.x = move_vector.x / module;
	move_vector.y = move_vector.y / module;
	float ang_test = (float)RAD_TO_DEG * atan2(-move_vector.y, move_vector.x);
	LOG("ang_test: %f", ang_test);

	iPoint direction_vec;
	direction_vec.x = path_objective.x - GetX();
	direction_vec.y = GetY() - path_objective.y;
	angle = (float)RAD_TO_DEG * atan2(direction_vec.y, direction_vec.x);

	if (angle < 0)
		angle += 360;


	if ((0 <= angle &&  angle <= 22.5) || (337.5 <= angle&& angle <= 360))
		this->direction = EAST;

	else if (22.5 <= angle &&  angle <= 67.5)
		this->direction = NORTH_EAST;

	else if (67.5 <= angle &&  angle <= 112.5)
		this->direction = NORTH;

	else if (112.5 <= angle &&  angle <= 157.5)
		this->direction = NORTH_WEST;

	else if (157.5 <= angle &&  angle <= 202.5)
		this->direction = WEST;

	else if (202.5 <= angle &&  angle <= 247.5)
		this->direction = SOUTH_WEST;

	else if (247.5 <= angle &&  angle <= 292.5)
		this->direction = SOUTH;

	else if (292.5 <= angle &&  angle <= 337.5)
		this->direction = SOUTH_EAST;

	else
		this->direction = NO_DIRECTION;

	return ret;
}

bool Unit::CheckSurroundings()
{
	iPoint tile;
	iPoint ori = App->map->WorldToMap(GetX(), GetY());
	tile.x = ori.x - unit_radius;
	tile.y = ori.y - unit_radius;
	for (tile.x; tile.x < ori.x + 12; tile.x++) {
		for (tile.y = ori.y - unit_radius; tile.y < ori.y + 12; tile.y++) {
			if (App->entity_manager->IsUnitInTile(this, tile) )
			{
				if (tile.x == ori.x + 1 == false) {
					iPoint destination = App->map->MapToWorld(tile.x - 1, tile.y );
					if (this->GetPath({ destination.x, destination.y }) != -1)
					{
						path_list.pop_front();
						GetNextTile();
						this->action_type = WALK;
						state = MOVING;
					}
				}
				else
				{
					state = NONE;
					GetNextTile();
					this->SetPosition(path_objective.x, path_objective.y);
					this->action_type = ATTACK;
					this->direction = SOUTH;
					//state = ATTACKING;
				}
				return true;
			}
		}
	}

	return false;
}

bool Unit::AttackUnit(Unit* victim)
{
	bool ret = false;

	if (victim->GetHP() > 0) {

		iPoint entityPos = App->map->WorldToMap(victim->GetX(), victim->GetY());
		iPoint Pos = App->map->WorldToMap(GetX(), GetY());

		if (Pos.x == entityPos.x - 1 && Pos.y == entityPos.y)//Enemy is south
		{
			victim->SetHp(victim->GetHP() - attack);
			this->direction = SOUTH;
			this->action_type = ATTACK;
			ret = true;
		}

		else if (Pos.x == entityPos.x && Pos.y == entityPos.y - 1)//Enemy is west
		{
			victim->SetHp(victim->GetHP() - attack);
			this->direction = WEST;
			this->action_type = ATTACK;
			ret = true;
		}

		else if (Pos.x == entityPos.x + 1 && Pos.y == entityPos.y)//Enemy is north
		{
			victim->SetHp(victim->GetHP() - attack);
			this->direction = NORTH;
			this->action_type = ATTACK;
			ret = true;
		}

		else if (Pos.x == entityPos.x && Pos.y == entityPos.y + 1)//Enemy is east
		{
			victim->SetHp(victim->GetHP() - attack);
			this->direction = EAST;
			this->action_type = ATTACK;
			ret = true;
		}

		else {
			this->path_list.clear();
			iPoint enemy;
			enemy.x = victim->GetX();
			enemy.y = victim->GetY();
			if (this->GetPath(enemy) != -1)
			{
				path_list.pop_front();
				GetNextTile();
				this->action_type = WALK;
				state = MOVING;
				ret = true;
			}
			else
			{
				state = NONE;
			}
		}
	}
	else {
		return false;
	}
	
	return ret;
}