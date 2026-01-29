#ifndef MAZE_H
#define MAZE_H

#include <vector>

struct MazePt
{
	int x, y;
	MazePt(int _x = 0, int _y = 0) : x(_x), y(_y) {}
	void Set(int _x = 0, int _y = 0) {x = _x; y = _y;}
};

class Maze
{
public:
	enum TILE_CONTENT
	{
		TILE_WALL = -1,
		TILE_FOG = 0,
		TILE_EMPTY = 1,
		TILE_ORE = 2,
		TILE_WOODENLOG = 3,
		TILE_GRASS,
		TILE_SNOW,
		TILE_SAND,
		TILE_WATER,
		TILE_ICE,
		TILE_LAVA,
		TILE_NULL,
	};
	enum DIRECTION
	{
		//week 13 - for hexgrid
		DIR_LEFTUP,
		DIR_LEFTDOWN,
		DIR_RIGHTUP,
		DIR_RIGHTDOWN,

		DIR_UP,
		DIR_DOWN,
		DIR_LEFT,
		DIR_RIGHT,
	};
	Maze();
	~Maze();
	void Generate(unsigned key, unsigned size, MazePt start, float wallLoad);
	void ConvertWallsToResources(unsigned key, MazePt start, float oreFromWallsLoad, float woodFromWallsLoad);
	bool Move(DIRECTION direction);
	unsigned GetKey();
	unsigned GetSize();
	MazePt GetCurr();
	int GetNumMove();
	void SetCurr(MazePt newCurr);
	void SetNumMove(int num);

	TILE_CONTENT See(MazePt tile);
	bool IsPassable(Maze::TILE_CONTENT tile);
	bool NonMats(Maze::TILE_CONTENT tile);
	bool IsEmpty(Maze::TILE_CONTENT tile);
	std::vector<TILE_CONTENT> m_grid;
	std::vector<int> m_gridHealth;

private:
	//Do not make these public
	unsigned m_key;
	unsigned m_size;
	MazePt m_curr;
	int m_numMove;
};

#endif