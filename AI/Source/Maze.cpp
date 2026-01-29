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

void Maze::Generate(unsigned key, unsigned size, MazePt start)
{
	m_grid.clear();
	if (size == 0)
		return;
	start.x = Math::Clamp(start.x, 0, (int)size - 1);
	start.y = Math::Clamp(start.y, 0, (int)size - 1);

	float wallLoad = 0.15f;
	wallLoad = Math::Clamp(wallLoad, 0.f, 0.8f);

	float waterLoad = 0.15f;
	waterLoad = Math::Clamp(waterLoad, 0.f, 0.5f);

	float oreLoad = 0.02f;
	oreLoad = Math::Clamp(oreLoad, 0.f, 0.5f);

	float woodLoad = 0.02f;
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
	for (int i = 0; i < (int)(total * waterLoad);)
	{
		unsigned chosen = rand() % total;
		if (chosen == startId) continue;

		if (m_grid[chosen] == TILE_GRASS)
		{
			m_grid[chosen] = TILE_WATER;
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

void Maze::ConvertTerrainForWinter(unsigned key, MazePt start, std::vector<int>& outChangedIdx,
	float grassToSnowLoad,
	float treeToSnowLoad,
	float treeToGrassLoad,
	float waterToIceLoad
	)
{
	outChangedIdx.clear();
	if (m_size == 0 || m_grid.empty()) return;

	start.x = Math::Clamp(start.x, 0, (int)m_size - 1);
	start.y = Math::Clamp(start.y, 0, (int)m_size - 1);
	unsigned startId = start.y * m_size + start.x;

	grassToSnowLoad = Math::Clamp(grassToSnowLoad, 0.f, 1.f);
	treeToSnowLoad = Math::Clamp(treeToSnowLoad, 0.f, 1.f);
	treeToGrassLoad = Math::Clamp(treeToGrassLoad, 0.f, 1.f);
	waterToIceLoad = Math::Clamp(waterToIceLoad, 0.f, 1.f);

	srand(key);

	// ---------------------------
	// 1) GRASS -> SNOW
	// ---------------------------
	{
		std::vector<unsigned> grassIds;
		grassIds.reserve(m_grid.size());

		for (unsigned i = 0; i < (unsigned)m_grid.size(); ++i)
		{
			if (i == startId) continue;
			if (m_grid[i] == TILE_GRASS)
				grassIds.push_back(i);
		}

		// shuffle
		for (int i = (int)grassIds.size() - 1; i > 0; --i)
		{
			int j = rand() % (i + 1);
			std::swap(grassIds[i], grassIds[j]);
		}

		int grassCount = (int)grassIds.size();
		int snowCount = (int)(grassCount * grassToSnowLoad);

		snowCount = Math::Clamp(snowCount, 0, grassCount);

		for (int k = 0; k < snowCount; ++k)
		{
			unsigned idx = grassIds[k];
			m_grid[idx] = TILE_SNOW;
			m_gridHealth[idx] = 0;
			outChangedIdx.push_back(idx);
		}
	}

	// ---------------------------
	// 2) TREES (WOODENLOG) -> SNOW / GRASS
	// ---------------------------
	{
		std::vector<unsigned> treeIds;
		treeIds.reserve(m_grid.size());

		for (unsigned i = 0; i < (unsigned)m_grid.size(); ++i)
		{
			if (i == startId) continue;
			if (m_grid[i] == TILE_WOODENLOG) // your tree tile
				treeIds.push_back(i);
		}

		// shuffle
		for (int i = (int)treeIds.size() - 1; i > 0; --i)
		{
			int j = rand() % (i + 1);
			std::swap(treeIds[i], treeIds[j]);
		}

		int treeCount = (int)treeIds.size();
		int toSnow = (int)(treeCount * treeToSnowLoad);
		int toGrass = (int)(treeCount * treeToGrassLoad);

		// Safety: can't convert more than we have
		if (toSnow + toGrass > treeCount)
		{
			float sum = treeToSnowLoad + treeToGrassLoad;
			if (sum > 0.f)
			{
				toSnow = (int)(treeCount * (treeToSnowLoad / sum));
				toGrass = (int)(treeCount * (treeToGrassLoad / sum));
			}
			if (toSnow + toGrass > treeCount)
				toGrass = treeCount - toSnow;
		}

		toSnow = Math::Clamp(toSnow, 0, treeCount);
		toGrass = Math::Clamp(toGrass, 0, treeCount - toSnow);

		// first chunk -> snow
		for (int k = 0; k < toSnow; ++k)
		{
			unsigned idx = treeIds[k];
			m_grid[idx] = TILE_SNOW;
			m_gridHealth[idx] = 0;
			outChangedIdx.push_back(idx);
		}

		// next chunk -> grass
		for (int k = toSnow; k < toSnow + toGrass; ++k)
		{
			unsigned idx = treeIds[k];
			m_grid[idx] = TILE_GRASS;
			m_gridHealth[idx] = 0;
			outChangedIdx.push_back(idx);
		}

		// remaining trees stay trees: enforce tree hp = 100
		for (int k = toSnow + toGrass; k < treeCount; ++k)
		{
			unsigned idx = treeIds[k];
			m_gridHealth[idx] = 100;
		}
	}

	// ---------------------------
	// 3) WATER -> ICE
	// ---------------------------
	{
		std::vector<unsigned> waterIds;
		waterIds.reserve(m_grid.size());

		for (unsigned i = 0; i < (unsigned)m_grid.size(); ++i)
		{
			if (i == startId) continue;
			if (m_grid[i] == TILE_WATER)
				waterIds.push_back(i);
		}

		// shuffle
		for (int i = (int)waterIds.size() - 1; i > 0; --i)
		{
			int j = rand() % (i + 1);
			std::swap(waterIds[i], waterIds[j]);
		}

		int waterCount = (int)waterIds.size();
		int iceCount = (int)(waterCount * waterToIceLoad);

		iceCount = Math::Clamp(iceCount, 0, waterCount);

		for (int k = 0; k < iceCount; ++k)
		{
			unsigned idx = waterIds[k];
			m_grid[idx] = TILE_ICE;
			m_gridHealth[idx] = 0; // ice hp = 0
			outChangedIdx.push_back(idx);
		}
	}
}

void Maze::ConvertTerrainForDesert(
	unsigned key,
	MazePt start,
	std::vector<int>& outChangedIdx,
	float grassToSandLoad,
	float treeToSandLoad,
	float treeToGrassLoad,
	float waterToSandLoad
)
{
	outChangedIdx.clear();
	if (m_size == 0 || m_grid.empty()) return;

	start.x = Math::Clamp(start.x, 0, (int)m_size - 1);
	start.y = Math::Clamp(start.y, 0, (int)m_size - 1);
	unsigned startId = start.y * m_size + start.x;

	grassToSandLoad = Math::Clamp(grassToSandLoad, 0.f, 1.f);
	treeToSandLoad = Math::Clamp(treeToSandLoad, 0.f, 1.f);
	treeToGrassLoad = Math::Clamp(treeToGrassLoad, 0.f, 1.f);
	waterToSandLoad = Math::Clamp(waterToSandLoad, 0.f, 1.f);

	srand(key);

	// ---------------------------
	// 1) GRASS -> SAND
	// ---------------------------
	{
		std::vector<unsigned> grassIds;
		grassIds.reserve(m_grid.size());

		for (unsigned i = 0; i < (unsigned)m_grid.size(); ++i)
		{
			if (i == startId) continue;
			if (m_grid[i] == TILE_GRASS)
				grassIds.push_back(i);
		}

		// shuffle
		for (int i = (int)grassIds.size() - 1; i > 0; --i)
		{
			int j = rand() % (i + 1);
			std::swap(grassIds[i], grassIds[j]);
		}

		int grassCount = (int)grassIds.size();
		int sandCount = (int)(grassCount * grassToSandLoad);
		sandCount = Math::Clamp(sandCount, 0, grassCount);

		for (int k = 0; k < sandCount; ++k)
		{
			unsigned idx = grassIds[k];
			m_grid[idx] = TILE_SAND;
			m_gridHealth[idx] = 0;
			outChangedIdx.push_back((int)idx);
		}
	}

	// ---------------------------
	// 2) TREES (WOODENLOG) -> SAND / GRASS
	// ---------------------------
	{
		std::vector<unsigned> treeIds;
		treeIds.reserve(m_grid.size());

		for (unsigned i = 0; i < (unsigned)m_grid.size(); ++i)
		{
			if (i == startId) continue;
			if (m_grid[i] == TILE_WOODENLOG)
				treeIds.push_back(i);
		}

		// shuffle
		for (int i = (int)treeIds.size() - 1; i > 0; --i)
		{
			int j = rand() % (i + 1);
			std::swap(treeIds[i], treeIds[j]);
		}

		int treeCount = (int)treeIds.size();
		int toSand = (int)(treeCount * treeToSandLoad);
		int toGrass = (int)(treeCount * treeToGrassLoad);

		// Safety: can't convert more than we have
		if (toSand + toGrass > treeCount)
		{
			float sum = treeToSandLoad + treeToGrassLoad;
			if (sum > 0.f)
			{
				toSand = (int)(treeCount * (treeToSandLoad / sum));
				toGrass = (int)(treeCount * (treeToGrassLoad / sum));
			}
			if (toSand + toGrass > treeCount)
				toGrass = treeCount - toSand;
		}

		toSand = Math::Clamp(toSand, 0, treeCount);
		toGrass = Math::Clamp(toGrass, 0, treeCount - toSand);

		// first chunk -> sand
		for (int k = 0; k < toSand; ++k)
		{
			unsigned idx = treeIds[k];
			m_grid[idx] = TILE_SAND;
			m_gridHealth[idx] = 0;
			outChangedIdx.push_back((int)idx);
		}

		// next chunk -> grass
		for (int k = toSand; k < toSand + toGrass; ++k)
		{
			unsigned idx = treeIds[k];
			m_grid[idx] = TILE_GRASS;
			m_gridHealth[idx] = 0;
			outChangedIdx.push_back((int)idx);
		}

		// remaining trees stay trees: enforce hp = 100
		for (int k = toSand + toGrass; k < treeCount; ++k)
		{
			unsigned idx = treeIds[k];
			m_gridHealth[idx] = 100;
		}
	}

	// ---------------------------
	// 3) WATER -> SAND
	// ---------------------------
	{
		std::vector<unsigned> waterIds;
		waterIds.reserve(m_grid.size());

		for (unsigned i = 0; i < (unsigned)m_grid.size(); ++i)
		{
			if (i == startId) continue;
			if (m_grid[i] == TILE_WATER)
				waterIds.push_back(i);
		}

		// shuffle
		for (int i = (int)waterIds.size() - 1; i > 0; --i)
		{
			int j = rand() % (i + 1);
			std::swap(waterIds[i], waterIds[j]);
		}

		int waterCount = (int)waterIds.size();
		int sandCount = (int)(waterCount * waterToSandLoad);
		sandCount = Math::Clamp(sandCount, 0, waterCount);

		for (int k = 0; k < sandCount; ++k)
		{
			unsigned idx = waterIds[k];
			m_grid[idx] = TILE_SAND;   // NOTE: loses "used to be water" info
			m_gridHealth[idx] = 0;
			outChangedIdx.push_back((int)idx);
		}
	}
}

void Maze::RevertBiomeOverlayToForest(
	unsigned key,
	MazePt start,
	std::vector<int>& outChangedIdx,

	// ground overlay to revert (snow or sand)
	TILE_CONTENT groundFromTile,
	float groundToGrassLoad,
	float groundToTreeLoad,

	// liquid overlay to revert (ice or sand)
	TILE_CONTENT liquidFromTile,
	float liquidToWaterLoad
)
{
	outChangedIdx.clear();
	if (m_size == 0 || m_grid.empty()) return;

	start.x = Math::Clamp(start.x, 0, (int)m_size - 1);
	start.y = Math::Clamp(start.y, 0, (int)m_size - 1);
	unsigned startId = start.y * m_size + start.x;

	groundToGrassLoad = Math::Clamp(groundToGrassLoad, 0.f, 1.f);
	groundToTreeLoad = Math::Clamp(groundToTreeLoad, 0.f, 1.f);
	liquidToWaterLoad = Math::Clamp(liquidToWaterLoad, 0.f, 1.f);

	srand(key);

	// ---------------------------
	// 1) GROUND overlay -> GRASS / TREE
	// ---------------------------
	{
		std::vector<unsigned> groundIds;
		groundIds.reserve(m_grid.size());

		for (unsigned i = 0; i < (unsigned)m_grid.size(); ++i)
		{
			if (i == startId) continue;
			if (m_grid[i] == groundFromTile)
				groundIds.push_back(i);
		}

		// shuffle
		for (int i = (int)groundIds.size() - 1; i > 0; --i)
		{
			int j = rand() % (i + 1);
			std::swap(groundIds[i], groundIds[j]);
		}

		int groundCount = (int)groundIds.size();
		int toGrass = (int)(groundCount * groundToGrassLoad);
		int toTree = (int)(groundCount * groundToTreeLoad);

		// Safety: don't exceed available
		if (toGrass + toTree > groundCount)
		{
			float sum = groundToGrassLoad + groundToTreeLoad;
			if (sum > 0.f)
			{
				toGrass = (int)(groundCount * (groundToGrassLoad / sum));
				toTree = (int)(groundCount * (groundToTreeLoad / sum));
			}
			if (toGrass + toTree > groundCount)
				toTree = groundCount - toGrass;
		}

		toGrass = Math::Clamp(toGrass, 0, groundCount);
		toTree = Math::Clamp(toTree, 0, groundCount - toGrass);

		// first chunk -> grass (hp 0)
		for (int k = 0; k < toGrass; ++k)
		{
			unsigned idx = groundIds[k];
			m_grid[idx] = TILE_GRASS;
			m_gridHealth[idx] = 0;
			outChangedIdx.push_back((int)idx);
		}

		// next chunk -> tree (WOODENLOG) (hp 100)
		for (int k = toGrass; k < toGrass + toTree; ++k)
		{
			unsigned idx = groundIds[k];
			m_grid[idx] = TILE_WOODENLOG;
			m_gridHealth[idx] = 100;
			outChangedIdx.push_back((int)idx);
		}

		// remaining groundFromTile stays as-is (hp probably 0) – optional enforce:
		// for (int k = toGrass + toTree; k < groundCount; ++k)
		//     m_gridHealth[groundIds[k]] = 0;
	}

	// ---------------------------
	// 2) LIQUID overlay -> WATER
	// ---------------------------
	{
		std::vector<unsigned> liquidIds;
		liquidIds.reserve(m_grid.size());

		for (unsigned i = 0; i < (unsigned)m_grid.size(); ++i)
		{
			if (i == startId) continue;
			if (m_grid[i] == liquidFromTile)
				liquidIds.push_back(i);
		}

		// shuffle
		for (int i = (int)liquidIds.size() - 1; i > 0; --i)
		{
			int j = rand() % (i + 1);
			std::swap(liquidIds[i], liquidIds[j]);
		}

		int liquidCount = (int)liquidIds.size();
		int toWater = (int)(liquidCount * liquidToWaterLoad);
		toWater = Math::Clamp(toWater, 0, liquidCount);

		for (int k = 0; k < toWater; ++k)
		{
			unsigned idx = liquidIds[k];
			m_grid[idx] = TILE_WATER;

			// you didn't define water HP; keeping 0 like grass/snow/ice.
			// change if you want.
			m_gridHealth[idx] = 100;

			outChangedIdx.push_back((int)idx);
		}
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