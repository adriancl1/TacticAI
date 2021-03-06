
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
		//TODO 6 (EXTRA): Play around with the different properties 
	case TWOHANDEDSWORDMAN:
		SetHp(60);
		attack = 12;
		SetArmor(1);
		speed = 0.9;
		rate_of_fire = 2;
		range = 1;
		unit_class = INFANTRY;
		unit_radius = 5;
		AI = false;
		state = NONE;
		break;

	case TWOHANDEDSWORDMANENEMY:
		SetHp(80);
		attack = 12;
		SetArmor(1);
		speed = 0.9;
		rate_of_fire = 2;
		range = 1;
		unit_class = INFANTRY;
		unit_radius = 5;
		AI = true;
		state = NONE;
		break;


	case CAVALRYARCHER:
		SetHp(50);
		attack = 8;
		SetArmor(1);
		speed = 1.8;
		rate_of_fire = 2;
		range = 4;
		unit_class = RANGED;
		unit_radius = 8;
		AI = false;
		state = NONE;
		break;

	
	case ARCHER:
		SetHp(40);
		attack = 10;
		SetArmor(1);
		speed = 0.9;
		rate_of_fire = 2;
		range = 4;
		unit_class = RANGED;
		unit_radius = 8;
		AI = false;
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
	if (!AI) {
		if (state == NONE) {
			CheckSurroundings();
		}

		if (App->input->GetMouseButtonDown(3) == KEY_DOWN && this->GetEntityStatus() == E_SELECTED)
		{
			this->path_list.clear();
			App->input->GetMousePosition(destination.x, destination.y);
			destination.x -= App->render->camera.x;
			destination.y -= App->render->camera.y;
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
				this->action_type = IDLE;
			}
		}

		if (state == MOVING)
		{
			Move();
			CheckSurroundings();
		}

		if (state == MOVING_TO_ATTACK)
		{
			Move();
		}

		if (state == ATTACKING)
		{
			AttackUnit();
		}
	}

	else if (AI)
	{
		DoAI();
	}

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN && this->GetEntityStatus() == E_SELECTED)
	{
		debug = !debug;
	}

	DrawDebugRadius();
	Draw();

}

void Unit::PostUpdate()
{
	if (enemy != nullptr && enemy->state == DEAD)
	{
		enemy = nullptr;
	}

	//TODO 5: If your HP is below zero, die and delete the unit (entity_manager->DeleteUnit)
	if (GetHP() <= 0) {
		state = DEAD;
		this->action_type = DIE;
		App->entity_manager->DeleteUnit(this);
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
				if (state == MOVING_TO_ATTACK)
				{
					state = ATTACKING;
					this->action_type = IDLE;
				}
				else {
					state = NONE;
					this->action_type = IDLE;
				}
			}
		}
}

void Unit::DoAI()
{

	if (state == NONE)
	{
		CheckSurroundings();
	}

	if (state == ATTACKING) 
	{
		AttackUnit();
	}

	if (state == MOVING || state == MOVING_TO_ATTACK)
	{
		Move();
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

bool Unit::AttackUnit()
{
	/*TODO 4: Check if your enemy is still next to you if you're melee. Reduce the HP of your enemy if he's still alive (use a timer).
	Reset to an idle position if he's dead.
	USEFUL METHODS:
	LookAtEnemy
	SetFightingArea (if he's not next to you)
	*/
	bool ret = false;
	
	return ret;
}

bool Unit::CheckSurroundings()
{
	//TODO 1: Change BFS to work as intended in the picture. Hint: it's a couple of fors, but some sizes may change along the way so be careful. Don't forget the timer.
		frontier.clear();
		visited.clear();//Reset frontier and visited
		logicTimer.Start();//Start timer again

		frontier.push_back(App->map->WorldToMap(GetX(), GetY()));
		visited.push_back(App->map->WorldToMap(GetX(), GetY()));

		iPoint current;
		current = *frontier.begin();
		frontier.pop_front();
		
		iPoint neighbors[4];
		neighbors[0].x = current.x + 1;
		neighbors[0].y = current.y;
		neighbors[1].x = current.x - 1;
		neighbors[1].y = current.y;
		neighbors[2].x = current.x;
		neighbors[2].y = current.y - 1;
		neighbors[3].x = current.x;
		neighbors[3].y = current.y + 1;

		//TODO 2: Check if a unit is in any of the neighbors, and if someone's there and his AI is different than yours, make him your enemy and procede to the next todo.

		bool alreadyVisited = false;
		for (int i = 0; i <= 3; i++) {
			if (frontier.size() == 1)
			{
				frontier.push_back(iPoint(neighbors[i]));
				visited.push_back(iPoint(neighbors[i]));
			}

			else {
				if (App->pathfinding->IsWalkable(neighbors[i])) {
					std::list<iPoint>::iterator tmp = visited.begin();
					int visitedNum = visited.size();
					alreadyVisited = false;
					for (int p = 0; p < visitedNum; tmp++, p++) {
						if ((*tmp).x == neighbors[i].x && (*tmp).y == neighbors[i].y)
						{
							alreadyVisited = true;
						}
					}
					if (alreadyVisited == false)
					{
						frontier.push_back(iPoint(neighbors[i]));
						visited.push_back(iPoint(neighbors[i]));
					}
				}
			}
		}
	return true;
}

bool Unit::SetFightingArea()
{
	/*TODO 3: This will be a long one, so let's go step by step. Now we'll find the tiles where the unit and his enemy will fight.
	First, you should check if the enemy is already fighting someone else.
	Next check your and his class, every combination needs to be resolved in a different way:
	Ranged vs Ranged: Both attack from where they stand.
	Ranged vs Melee: Ranged stays put and attacks, melee moves next to him.
	Melee vs Melee: Find the tile halfway to the enemy, make it your destination and give the enemy a tile next to it as his destination.
	If the enemy is in combat with someone else, just go fight him! Don't change anything about him.

	USEFUL METHODS:
	GetFreeAdjacent
	GetAdjacentTile
	To know how to move units take a look at Update()
	*/
	if (enemy != nullptr)
	{
		bool ret;

		iPoint enemyPos = App->map->WorldToMap(enemy->GetX(), enemy->GetY());
		iPoint Pos = App->map->WorldToMap(GetX(), GetY());

		iPoint distance = Pos - enemyPos;

		if (enemy->enemy == nullptr || enemy->enemy == this) {

			if (unit_class == RANGED && enemy->unit_class == RANGED)
			{

			}
			else if (unit_class == RANGED && enemy->unit_class != RANGED) {

			}
			else if (unit_class != RANGED && enemy->unit_class != RANGED) 
			{

			}
		}
		else if (enemy->enemy != nullptr && enemy->enemy != this)
		{
			if (unit_class == RANGED)
			{
			
			}
			else
			{
				if (enemy->state == ATTACKING) 
				{
				
				}

				else if (enemy->state == MOVING_TO_ATTACK)
				{

				}
			}
		}
	}
	return true;
}

//Returns the direction to face the enemy 
void Unit::LookAtEnemy()
{
	if (enemy != nullptr)
	{

		iPoint direction_vec;
		direction_vec.x = enemy->GetX() - GetX();
		direction_vec.y = GetY() - enemy->GetY();

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
	}
}

//Returns a tile unnocupied next to the unit.
bool Unit::GetFreeAdjacent(iPoint& Adjacent) const
{
	iPoint ret = App->map->WorldToMap(GetX(), GetY());

	if ((!App->entity_manager->IsUnitInTile(this, { ret.x + 1, ret.y })) && App->pathfinding->IsWalkable({ ret.x + 1, ret.y }))
	{
		Adjacent = { ret.x + 1, ret.y };
		return true;
	}
	else if ((!App->entity_manager->IsUnitInTile(this, { ret.x - 1, ret.y })) && App->pathfinding->IsWalkable({ ret.x - 1, ret.y }))
	{
		Adjacent = { ret.x - 1, ret.y };
		return true;
	}
	else if ((!App->entity_manager->IsUnitInTile(this, { ret.x, ret.y + 1 })) && App->pathfinding->IsWalkable({ ret.x, ret.y + 1 }))
	{
		Adjacent = { ret.x, ret.y + 1};
		return true;
	}
	else if ((!App->entity_manager->IsUnitInTile(this, { ret.x, ret.y - 1 })) && App->pathfinding->IsWalkable({ ret.x, ret.y - 1 }))
	{
		Adjacent = { ret.x, ret.y - 1};
		return true;
	}

	else
	{
		return false;
	}
}

//Gets an unnocupied tile next to the "tile"
bool Unit::GetAdjacentTile(iPoint tile, iPoint& Adjacent) const
{

	if ((!App->entity_manager->IsUnitInTile(this, { tile.x + 1, tile.y })) && App->pathfinding->IsWalkable({ tile.x + 1, tile.y }))
	{
		Adjacent = { tile.x + 1, tile.y };
		return true;
	}
	else if ((!App->entity_manager->IsUnitInTile(this, { tile.x - 1, tile.y })) && App->pathfinding->IsWalkable({ tile.x - 1, tile.y }))
	{
		Adjacent = { tile.x - 1, tile.y };
		return true;
	}
	else if ((!App->entity_manager->IsUnitInTile(this, { tile.x, tile.y + 1 })) && App->pathfinding->IsWalkable({ tile.x, tile.y + 1 }))
	{
		Adjacent = { tile.x, tile.y + 1 };
		return true;
	}
	else if ((!App->entity_manager->IsUnitInTile(this, { tile.x, tile.y - 1 })) && App->pathfinding->IsWalkable({ tile.x, tile.y - 1 }))
	{
		Adjacent = { tile.x, tile.y - 1 };
		return true;
	}

	else
	{
		return false;
	}
}

void Unit::DrawDebugRadius()
{
	// Print DEBUG 
	if (debug)
	{
		// Draw Visited
		iPoint point;
		std::list<iPoint>::iterator item = visited.begin();

		while (item != visited.end())
		{
			point = (*item);
			TileSet* tileset = App->map->GetTilesetFromTileId(25);

			SDL_Rect r = tileset->GetTileRect(25);
			iPoint pos = App->map->MapToWorld(point.x, point.y);

			App->render->Blit(App->tex->debugTex, pos.x - 32, pos.y - 32, &r);

			item++;
		}

		// Draw frontier
		for (std::list<iPoint>::iterator item2 = frontier.begin(); item2 != frontier.end(); item2++)
		{
			point = (*item2);
			TileSet* tileset = App->map->GetTilesetFromTileId(26);

			SDL_Rect r = tileset->GetTileRect(26);
			iPoint pos = App->map->MapToWorld(point.x, point.y);

			App->render->Blit(App->tex->debugTex, pos.x - 32, pos.y - 32, &r);
		}
	}
}