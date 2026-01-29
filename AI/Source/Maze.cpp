#include "Maze.h"
#include <iostream>
#include "MyMath.h"
#include "Application.h"

//DO NOT FOLLOW
//ugly hack: using global var to determine sleep duration (week 7)
int gSleepDuration = 10;

Maze::Maze()
	: m_grid{}, m_key{}, m_size{}, m_curr{}, m_numMove{}
{
}

Maze::~Maze()
{
}

void Maze::Generate(unsigned key, unsigned size, MazePt start, float wallLoad)
{
	m_grid.clear();
	if (size == 0)
		return;
	start.x = Math::Clamp(start.x, 0, (int)size - 1);
	start.y = Math::Clamp(start.y, 0, (int)size - 1);
	wallLoad = Math::Clamp(wallLoad, 0.f, 0.8f);

	float oreLoad = 0.015f;
	oreLoad = Math::Clamp(oreLoad, 0.f, 0.5f);

	float woodLoad = 0.015f;
	woodLoad = Math::Clamp(woodLoad, 0.f, 0.5f);

	unsigned total = size * size;
	m_grid.resize(total);
	m_gridHealth.resize(total);
	std::fill(m_grid.begin(), m_grid.end(), TILE_GRASS);
	std::fill(m_gridHealth.begin(), m_gridHealth.end(), 0);
	unsigned startId = start.y * size + start.x;
	srand(key);
	for (int i = 0; i < (int)total * wallLoad;)
	{
		unsigned chosen = rand() % total;
		if (chosen == startId)
			continue;
		if (m_grid[chosen] == TILE_GRASS)
		{
			m_grid[chosen] = TILE_WALL;
			m_gridHealth[chosen] = 100;
			++i;
		}
	}
	// 2) Place slow tiles (only on remaining EMPTY tiles)
	for (int i = 0; i < (int)(total * oreLoad);)
	{
		unsigned chosen = rand() % total;
		if (chosen == startId) continue;

		if (m_grid[chosen] == TILE_GRASS)
		{
			m_grid[chosen] = TILE_ORE;
			m_gridHealth[chosen] = 100;
			++i;
		}
	}
	for (int i = 0; i < (int)(total * woodLoad);)
	{
		unsigned chosen = rand() % total;
		if (chosen == startId) continue;

		if (m_grid[chosen] == TILE_GRASS)
		{
			m_grid[chosen] = TILE_WOODENLOG;
			m_gridHealth[chosen] = 100;
			++i;
		}
	}
	std::cout << "Maze " << key << std::endl;
	for (int row = (int)size - 1; row >= 0; --row)
	{
		for (int col = 0; col < (int)size; ++col)
		{
			int t = m_grid[row * size + col];
			if (t == TILE_WALL) std::cout << "1 ";
			else if (t == TILE_ORE) std::cout << "S ";
			else std::cout << "0 ";
		}
		std::cout << std::endl;
	}
	m_key = key;
	m_size = size;
	m_numMove = 0;
}

void Maze::ConvertWallsToResources(unsigned key, MazePt start,
	float oreFromWallsLoad, float woodFromWallsLoad)
{
	if (m_size == 0 || m_grid.empty()) return;

	start.x = Math::Clamp(start.x, 0, (int)m_size - 1);
	start.y = Math::Clamp(start.y, 0, (int)m_size - 1);
	unsigned startId = start.y * m_size + start.x;

	oreFromWallsLoad = Math::Clamp(oreFromWallsLoad, 0.f, 1.f);
	woodFromWallsLoad = Math::Clamp(woodFromWallsLoad, 0.f, 1.f);

	// Collect all wall indices (excluding start)
	std::vector<unsigned> wallIds;
	wallIds.reserve(m_grid.size());
	for (unsigned i = 0; i < (unsigned)m_grid.size(); ++i)
	{
		if (i == startId) continue;
		if (m_grid[i] == TILE_WALL)
			wallIds.push_back(i);
	}

	if (wallIds.empty()) return;

	// Shuffle walls (so the converted ones are random but no repeats)
	srand(key);
	for (int i = (int)wallIds.size() - 1; i > 0; --i)
	{
		int j = rand() % (i + 1);
		std::swap(wallIds[i], wallIds[j]);
	}

	int wallCount = (int)wallIds.size();
	int oreCount = (int)(wallCount * oreFromWallsLoad);
	int woodCount = (int)(wallCount * woodFromWallsLoad);

	// Safety: can't convert more than we have
	if (oreCount + woodCount > wallCount)
	{
		// scale down proportionally (simple + safe)
		float sum = oreFromWallsLoad + woodFromWallsLoad;
		if (sum > 0.f)
		{
			oreCount = (int)(wallCount * (oreFromWallsLoad / sum));
			woodCount = (int)(wallCount * (woodFromWallsLoad / sum));
		}
		// final clamp
		if (oreCount + woodCount > wallCount)
			woodCount = wallCount - oreCount;
	}

	// First chunk -> ore
	for (int k = 0; k < oreCount; ++k)
	{
		unsigned idx = wallIds[k];
		m_grid[idx] = TILE_ORE;
		m_gridHealth[idx] = 100;
	}

	// Next chunk -> wood
	for (int k = oreCount; k < oreCount + woodCount; ++k)
	{
		unsigned idx = wallIds[k];
		m_grid[idx] = TILE_WOODENLOG;
		m_gridHealth[idx] = 100;
	}
}

unsigned Maze::GetKey()
{
	return m_key;
}

unsigned Maze::GetSize()
{
	return m_size;
}

MazePt Maze::GetCurr()
{
	return m_curr;
}

int Maze::GetNumMove()
{
	return m_numMove;
}

void Maze::SetCurr(MazePt newCurr)
{
	++m_numMove;
	m_curr = newCurr;
}

void Maze::SetNumMove(int num)
{
	m_numMove = num;
}

bool Maze::Move(DIRECTION direction)
{
	//Application::GetInstance().Iterate();
	//Sleep(gSleepDuration);

	++m_numMove;
	MazePt temp = m_curr;
	switch (direction)
	{
	case DIR_LEFTUP:
		if (temp.x == 0 || temp.y == (int)m_size - 1)
			return false;
		temp.x -= 1;
		temp.y += 1;
		break;
	case DIR_LEFTDOWN:
		if (temp.x == 0 || temp.y == 0)
			return false;
		temp.x -= 1;
		temp.y -= 1;
		break;
	case DIR_RIGHTUP:
		if (temp.x == (int)m_size - 1 || temp.y == (int)m_size - 1)
			return false;
		temp.x += 1;
		temp.y += 1;
		break;
	case DIR_RIGHTDOWN:
		if (temp.x == (int)m_size - 1 || temp.y == 0)
			return false;
		temp.x += 1;
		temp.y -= 1;
		break;
	case DIR_LEFT:
		if (temp.x == 0)
			return false;
		temp.x -= 1;
		break;
	case DIR_RIGHT:
		if (temp.x == (int)m_size - 1)
			return false;
		temp.x += 1;
		break;
	case DIR_UP:
		if (temp.y == (int)m_size - 1)
			return false;
		temp.y += 1;
		break;
	case DIR_DOWN:
		if (temp.y == 0)
			return false;
		temp.y -= 1;
		break;
	}
	int tempId = temp.y * m_size + temp.x;

	if (!IsPassable(m_grid[tempId]))
		return false;
	m_curr = temp;
	return true;
}

Maze::TILE_CONTENT Maze::See(MazePt tile)
{
	//week 8
	//this function will tell the user the content of the specified position
	
	//ensure provided position is not out of bound
	if (tile.x < 0 || tile.x >= static_cast<int>(m_size) ||
		tile.y < 0 || tile.y >= static_cast<int>(m_size))
		return TILE_CONTENT::TILE_WALL;

	return m_grid[tile.y*m_size + tile.x];
}


bool Maze::IsPassable(Maze::TILE_CONTENT tile)
{
	if (tile == Maze::TILE_WALL || tile == Maze::TILE_ORE || tile == Maze::TILE_FOG || tile == Maze::TILE_WOODENLOG || tile == Maze::TILE_WATER) return false;
	return true;
}

bool Maze::NonMats(Maze::TILE_CONTENT tile)
{
	if (tile == Maze::TILE_WALL || tile == Maze::TILE_FOG) return false;
	return true;
}

bool Maze::IsEmpty(Maze::TILE_CONTENT tile)
{
	if (tile == Maze::TILE_EMPTY || tile == Maze::TILE_GRASS || tile == Maze::TILE_SNOW || tile == Maze::TILE_SAND || tile == Maze::TILE_LAVA || tile == Maze::TILE_ICE) return true;
	return false;
}