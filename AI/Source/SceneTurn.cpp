#include "SceneTurn.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>
#include "GridSettings.h"

SceneTurn::SceneTurn()
	: m_turn{}, m_goList{}, m_speed{}, m_objectCount{}, m_noGrid{},
	  m_gridSize{}, m_gridOffset{}, m_maze{},
	  m_start{}, m_end{}, m_myGrid{},
	  m_visited{}, m_queue{}, m_previous{},
	  m_shortestPath{}, m_mazeKey{}, m_wallLoad{}
{
}

SceneTurn::~SceneTurn()
{
}

void SceneTurn::Init()
{
	SceneBase::Init();
	bLightEnabled = true; //week 8: have to do this for material to work

	//Calculating aspect ratio
	//Physics code here
	m_speed = 1;

	Math::InitRNG();

	m_noGrid = 20;
	m_gridSize = m_worldHeight / m_noGrid;
	m_gridOffset = m_gridSize / 2;

	m_start.Set(0, 0);
	m_mazeKey = 0;
	m_wallLoad = 0.3f;
	m_maze.Generate(m_mazeKey, m_noGrid, m_start, m_wallLoad); //Generate new maze
	m_myGrid.resize(m_noGrid * m_noGrid);
	m_visited.resize(m_noGrid * m_noGrid);
	m_previous.resize(m_noGrid * m_noGrid);
	std::fill(m_myGrid.begin(), m_myGrid.end(), Maze::TILE_FOG);
	std::fill(m_visited.begin(), m_visited.end(), false);
	m_myGrid[m_start.y * m_noGrid + m_start.x] = Maze::TILE_EMPTY;
	DFS(m_start);
	//CarveUntilNoFog();
}


GameObject* SceneTurn::FetchGO(GameObject::GAMEOBJECT_TYPE type)
{
	for (std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if (!go->active)
		{
			go->active = true;
			go->type = type;
			++m_objectCount;
			return go;
		}
	}
	for (unsigned i = 0; i < 10; ++i)
	{
		GameObject *go = new GameObject(GameObject::GO_NONE);
		m_goList.push_back(go);
	}
	return FetchGO(type);
}

void SceneTurn::DFS(MazePt curr)
{
	m_visited[curr.y * m_noGrid + curr.x] = true;
	//UP
	if(curr.y < m_noGrid - 1)
	{
		MazePt next(curr.x, curr.y + 1);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_UP) == true)
			{
				Maze::TILE_CONTENT var = m_maze.See(next);
				m_myGrid[next.y * m_noGrid + next.x] = var;
				DFS(next);
				m_maze.Move(Maze::DIR_DOWN);
			}
			else
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
			}
		}
	}
	//DOWN
	if (curr.y > 0)
	{
		MazePt next(curr.x, curr.y - 1);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_DOWN) == true)
			{
				Maze::TILE_CONTENT var = m_maze.See(next);
				m_myGrid[next.y * m_noGrid + next.x] = var;
				DFS(next);
				m_maze.Move(Maze::DIR_UP);
			}
			else
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
			}
		}
	}
	//LEFT
	if (curr.x > 0)
	{
		MazePt next(curr.x - 1, curr.y);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_LEFT) == true)
			{
				Maze::TILE_CONTENT var = m_maze.See(next);
				m_myGrid[next.y * m_noGrid + next.x] = var;
				DFS(next);
				m_maze.Move(Maze::DIR_RIGHT);
			}
			else
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
			}
		}
	}
	//RIGHT
	if (curr.x < m_noGrid - 1)
	{
		MazePt next(curr.x + 1, curr.y);
		if (!m_visited[next.y * m_noGrid + next.x])
		{
			if (m_maze.Move(Maze::DIR_RIGHT) == true)
			{
				Maze::TILE_CONTENT var = m_maze.See(next);
				m_myGrid[next.y * m_noGrid + next.x] = var;
				DFS(next);
				m_maze.Move(Maze::DIR_LEFT);
			}
			else
			{
				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
			}
		}
	}
}
void SceneTurn::CarveUntilNoFog()
{
	while (true)
	{
		std::fill(m_visited.begin(), m_visited.end(), false);
		DFS(m_start);
		bool fog = false;

		for (auto t : m_myGrid)
		{
			if (t == Maze::TILE_FOG) fog = true;
		}
		if (!fog) { break; }

		bool carved = false;

		const int dx[4] = { 1,-1,0,0 };
		const int dy[4] = { 0,0,1,-1 };

		for (int y = 0; y < (int)m_noGrid && !carved; y++)
			for (int x = 0; x < (int)m_noGrid && !carved; x++)
			{
				int idx = y * m_noGrid + x;
				if (m_myGrid[idx] != Maze::TILE_FOG) continue; // we're looking at fog walls

				// if this fog tile touches ANY seen tile, break THIS tile
				for (int k = 0; k < 4; k++)
				{
					int nx = x + dx[k], ny = y + dy[k];
					if (nx < 0 || nx >= (int)m_noGrid || ny < 0 || ny >= (int)m_noGrid) continue;

					int nidx = ny * m_noGrid + nx;

					if (m_myGrid[nidx] != Maze::TILE_FOG)
					{
						//std::cout << ny << " , " << nx << std::endl;
						// carve the fog tile (idx) in the TRUE grid
						m_maze.m_grid[nidx] = Maze::TILE_EMPTY;
						m_myGrid[nidx] = Maze::TILE_EMPTY; // optional immediate reveal
						carved = true;
						break;
					}
				}
			}

		if (!carved) break; // safety
	}
}

bool SceneTurn::BFS(MazePt start, MazePt end)
{
	std::fill(m_visited.begin(), m_visited.end(), false);
	while (!m_queue.empty())
		m_queue.pop();
	m_shortestPath.clear();
	m_queue.push(start);
	m_maze.SetNumMove(0);
	while (!m_queue.empty())
	{
		MazePt curr = m_queue.front();
		m_maze.SetCurr(curr);
		m_queue.pop();
		if (curr.x == end.x && curr.y == end.y)
		{
			while (!(curr.x == start.x && curr.y == start.y))
			{
				m_shortestPath.insert(m_shortestPath.begin(), curr);
				curr = m_previous[curr.y * m_noGrid + curr.x];
			}
			m_shortestPath.insert(m_shortestPath.begin(), curr);
			return true;
		}
		//UP
		if (curr.y < m_noGrid - 1)
		{
			MazePt next(curr.x, curr.y + 1);
			if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
			{
				m_previous[next.y * m_noGrid + next.x] = curr;
				m_queue.push(next);
				m_visited[next.y * m_noGrid + next.x] = true;
			}
		}
		//DOWN
		if (curr.y > 0)
		{
			MazePt next(curr.x, curr.y - 1);
			if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
			{
				m_previous[next.y * m_noGrid + next.x] = curr;
				m_queue.push(next);
				m_visited[next.y * m_noGrid + next.x] = true;
			}
		}
		//LEFT
		if (curr.x > 0)
		{
			MazePt next(curr.x - 1, curr.y);
			if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
			{
				m_previous[next.y * m_noGrid + next.x] = curr;
				m_queue.push(next);
				m_visited[next.y * m_noGrid + next.x] = true;
			}
		}
		//RIGHT
		if (curr.x < m_noGrid - 1)
		{
			MazePt next(curr.x + 1, curr.y);
			if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
			{
				m_previous[next.y * m_noGrid + next.x] = curr;
				m_queue.push(next);
				m_visited[next.y * m_noGrid + next.x] = true;
			}
		}
	}
	return false;
}

void SceneTurn::Update(double dt)
{
	SceneBase::Update(dt);

	//Calculating aspect ratio

	if (Application::IsKeyPressed(VK_OEM_MINUS))
	{
		m_speed = Math::Max(0.f, m_speed - 0.1f);
	}
	if (Application::IsKeyPressed(VK_OEM_PLUS))
	{
		m_speed += 0.1f;
	}
	if (Application::IsKeyPressed('N'))
	{
	}
	if (Application::IsKeyPressed('M'))
	{
	}
	if (Application::IsKeyPressed(VK_RETURN))
	{
	}
	if (Application::IsKeyPressed('R'))
	{
		//Exercise: Implement Reset button
	}

	//Input Section
	static bool bLButtonState = false;
	if (!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;
		double x, y;
		Application::GetCursorPos(&x, &y);
		int w = Application::GetWindowWidth();
		int h = Application::GetWindowHeight();
		float posX = static_cast<float>(x) / w * m_worldWidth;
		float posY = (h - static_cast<float>(y)) / h * m_worldHeight;
		if (posX < m_noGrid * m_gridSize && posY < m_noGrid * m_gridSize) //ensure we're clicking within the board
		{
			m_end.Set(static_cast<int>(posX / m_gridSize), static_cast<int>(posY / m_gridSize));
			for (GameObject* go : m_goList)
				if (go->active && go->type == GameObject::GO_NPC)
					PathFind(go, m_end, 1);
		}
	}
	else if (bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		std::cout << "LBUTTON UP" << std::endl;
	}
	static bool bRButtonState = false;
	if (!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		std::cout << "RBUTTON DOWN" << std::endl;
	}
	else if (bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;
	}
	static bool bSpaceState = false;
	if (!bSpaceState && Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = true;

		// Exercise Week 08
		//7.	SceneTurn::Update, spawn a GO_NPC with spacebar at any empty tile
		GameObject* go = FetchGO(GameObject::GAMEOBJECT_TYPE::GO_NPC);
		go->grid.resize(m_noGrid * m_noGrid);
		go->visited.resize(m_noGrid * m_noGrid);
		std::fill(go->grid.begin(), go->grid.end(), Maze::TILE_FOG);
		std::fill(go->visited.begin(), go->visited.end(), false);
		//set position to a random EMPTY tile
		do
		{
			go->curr.Set(Math::RandIntMinMax(0, m_noGrid - 1),
				Math::RandIntMinMax(0, m_noGrid - 1));
		} while (m_maze.See(go->curr) != Maze::TILE_EMPTY);
		go->grid[Get1DIndex(go->curr.x, go->curr.y)] = Maze::TILE_EMPTY;
		RevealAround(go);
		go->stack.push_back(go->curr); //triggers dfs


	}
	else if (bSpaceState && !Application::IsKeyPressed(VK_SPACE))
	{
		bSpaceState = false;
	}

	// Exercise Week 08
	static constexpr float TURN_TIME = 0.5f;
	static float timer = 0.f;
	// 6.	SceneTurn::Update - add code to support a turn-based system. Read this pseudo codes and try to implement on your own. The solution will be provided during lesson time.
	//remember to declare and initialize m_turn
	timer += m_speed * dt;
		if (timer > 1.0f)
		{
			timer = 0.f;
			++m_turn;
			//for each GameObject in m_goList
			for (GameObject* go : m_goList)
			{
				if (go->active)
				{
					switch (go->type)
					{
					case GameObject::GO_NPC:
						DFSOnce(go);
						break;
					}
				}
			}
		}


}

void SceneTurn::RenderGO(GameObject *go)
{
	// Exercise Week 8
	// d.	Render each go based on its curr position - edit RenderGO(GameObject::GO_NPC)
	switch (go->type)
	{
		case GameObject::GO_NPC: //Render GO_NPC
		{
			modelStack.PushMatrix();
			modelStack.Translate(m_gridOffset + m_gridSize* go->curr.x,
				m_gridOffset + m_gridSize *
				go->curr.y,
				0.5f);
			modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
			RenderMesh(meshList[GEO_AGENT], false);
			modelStack.PopMatrix();
		}
		break;
	}
}

void SceneTurn::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection matrix : Orthographic Projection
	Mtx44 projection;
	projection.SetToOrtho(0, m_worldWidth, 0, m_worldHeight, -10, 10);
	projectionStack.LoadMatrix(projection);

	// Camera matrix
	viewStack.LoadIdentity();
	viewStack.LookAt(
		camera.position.x, camera.position.y, camera.position.z,
		camera.target.x, camera.target.y, camera.target.z,
		camera.up.x, camera.up.y, camera.up.z
		);
	// Model matrix : an identity matrix (model will be at the origin)
	modelStack.LoadIdentity();

	RenderMesh(meshList[GEO_AXES], false);

	modelStack.PushMatrix();
	modelStack.Translate(m_worldHeight * 0.5f, m_worldHeight * 0.5f, -1.f);
	modelStack.Scale(m_worldHeight, m_worldHeight, m_worldHeight);
	RenderMesh(meshList[GEO_WHITEQUAD], false);
	modelStack.PopMatrix();

	//Render tiles 
	for (int row = 0; row < m_noGrid; ++row)
	{
		for (int col = 0; col < m_noGrid; ++col)
		{
			modelStack.PushMatrix();
			modelStack.Translate(m_gridOffset + m_gridSize * col, m_gridOffset + m_gridSize * row, 0);
			modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
			// Exercise Week 08
			// b.Render the tiles using m_maze.m_grid instead of m_myGrid
			switch (m_myGrid[row * m_noGrid + col])
			{
			case Maze::TILE_WALL:
				RenderMesh(meshList[GEO_WALL], false);
				break;
			case Maze::TILE_FOG:
				meshList[GEO_WHITEQUAD]->material.kAmbient.Set(0.f, 0.f, 0.f);
				RenderMesh(meshList[GEO_WHITEQUAD], true);
				break;
			case Maze::TILE_SLOW:
				meshList[GEO_WHITEQUAD]->material.kAmbient.Set(1.f, 0.f, 0.f);
				RenderMesh(meshList[GEO_WHITEQUAD], true);
				break;
			case Maze::TILE_EMPTY:
				RenderMesh(meshList[GEO_FLOOR], false);
				break;
			}
			modelStack.PopMatrix();
		}
	}

	float x0 = 120;
	float y0 = 70;
	float miniTileSize = 2;
	for (int row = 0; row < m_noGrid; ++row)
	{
		for (int col = 0; col < m_noGrid; ++col)
		{
			modelStack.PushMatrix();

			// bottom-left anchor (x0,y0), then draw grid upward/rightward
			modelStack.Translate(x0 + miniTileSize * col,
				y0 + miniTileSize * row,
				0.2f);

			modelStack.Scale(miniTileSize, miniTileSize, miniTileSize);

			switch (m_maze.m_grid[row * m_noGrid + col])
			{
			case Maze::TILE_WALL:
				RenderMesh(meshList[GEO_WALL], false);
				break;

			case Maze::TILE_FOG:
				meshList[GEO_WHITEQUAD]->material.kAmbient.Set(0.f, 0.f, 0.f);
				RenderMesh(meshList[GEO_WHITEQUAD], true);
				break;

			case Maze::TILE_SLOW:
				meshList[GEO_WHITEQUAD]->material.kAmbient.Set(1.f, 0.f, 0.f);
				RenderMesh(meshList[GEO_WHITEQUAD], true);
				break;

			case Maze::TILE_EMPTY:
				RenderMesh(meshList[GEO_FLOOR], false);
				break;
			}

			modelStack.PopMatrix();
		}
	}

	for (GameObject* go : m_goList)
	{
		if (!go->active || go->type != GameObject::GO_NPC)
			continue;

		// Safety: make sure grid is the right size
		if ((int)go->grid.size() != m_noGrid * m_noGrid)
			continue;

		for (int i = 0; i < (int)go->grid.size(); ++i)
		{
			// "has vision/knowledge" = not fog
			if (go->grid[i] == Maze::TILE_FOG)
				continue;

			int cellX = i % m_noGrid;
			int cellY = i / m_noGrid;

			modelStack.PushMatrix();
			modelStack.Translate(m_gridOffset + m_gridSize * cellX,
				m_gridOffset + m_gridSize * cellY,
				0.06f); // slightly above floor
			modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);

			meshList[GEO_FLOOR]->material.kAmbient.Set(1.0f, 0.25f, 1.0f);
			RenderMesh(meshList[GEO_FLOOR], true);

			modelStack.PopMatrix();
		}
	}

	int i = 0;
	for (bool b : m_visited)
	{
		if (b)
		{
			int cellX = i % m_noGrid;
			int cellY = i / m_noGrid;
			modelStack.PushMatrix();
			modelStack.Translate(m_gridOffset + m_gridSize * cellX, m_gridOffset + m_gridSize * cellY, 0.05f);
			modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
			meshList[GEO_FLOOR]->material.kAmbient.Set(1.0f, 0.25f, 1);
			RenderMesh(meshList[GEO_FLOOR], true);
			modelStack.PopMatrix();
		}
		i++;
	}

	// Exercise Week 8
	//c.	Render each go based on its curr position - edit RenderGO(GameObject::GO_NPC)
	for (GameObject* go : m_goList)
	{
		if (go->active)
			RenderGO(go);
	}
	

	//On screen text
	std::ostringstream ss;
	ss.precision(3);
	ss << "Speed:" << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 6);

	ss.str("");
	ss.precision(5);
	ss << "FPS:" << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 3);

	ss.str("");
	ss << "Turn:" << m_turn;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 9);

	ss.str("");
	ss << "Turn Maze " << m_mazeKey;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 0);

	// Exercise Week 08
	// 10.	Print out the GameObject's ID and its target position
	int j = 0;
	for (GameObject* go : m_goList)
	{
		if (go->active)
		{
			ss.str("");
			ss << go->id << ": BFS to pos " << m_end.x << ", " <<
				m_end.y;
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0,
				1, 0), 3, 50, 57 - j * 3);
			j++;
		}
	}


}

void SceneTurn::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	while (m_goList.size() > 0)
	{
		GameObject *go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
}


// Exercise Week 08
// 8.	Implement DFSOnce() - this method will not be recursive. We will use a go->stack to help us with the depth first search
void SceneTurn::DFSOnce(GameObject* go)
{
	MazePt curr{ go->curr.x, go->curr.y };
	std::vector<MazePt>& stack = go->stack; //get a short-hand to stack instead of constant use of indirection later
		if (stack.empty() ||
			stack.back().x != curr.x || stack.back().y != curr.y)
			//make sure we don't push node into stack when backtracking
			go->stack.push_back(curr);
	go->visited[Get1DIndex(curr.x, curr.y)] = true;
	//mark current node as visited
	//offsets: UP, DOWN, LEFT, RIGHT
	static int offsets[][2] = { { 0, 1 }, { 0, -1 }, { -1, 0 },
	{ 1, 0 } };
	//check each direction
	//int (& offset)[2] ---> offset is a reference to int[2]
	MazePt next{};
	for (int(&offset)[2] : offsets)
	{
		next.Set(curr.x + offset[0], curr.y + offset[1]);
		//only consider next node if it's unvisited
		if (IsWithinBoundary(next.x) && IsWithinBoundary(next.y)
			&& !go->visited[Get1DIndex(next.x, next.y)])
		{
			//update agent's mental record of the maze
			int idx = Get1DIndex(next.x, next.y);
			go->grid[idx] = m_maze.See(next);
			if (go->grid[idx] == Maze::TILE_CONTENT::TILE_EMPTY)
			{
				go->curr = next; //let's move to the next cell
				return;
			}
		}
	}
	//already fully explored all surrounding neighbours. time tobacktrack
		stack.pop_back();
	if (!stack.empty())
		go->curr = stack.back();
}


// Exercise Week 08
// 9.	Implement BFSLimit() - BFS but with limit (e.g. 20 tiles to visit). If end is found, return path to end, else return path to tile that is closest with end
bool SceneTurn::BFSLimit(GameObject* go, MazePt end, int limit)
{
	//reset containers/variables before performing search
	std::fill(m_visited.begin(), m_visited.end(), false);
	std::fill(m_previous.begin(), m_previous.end(), -1); //may not actually be necessary to reset this
		go->path.clear();
	m_queue = {};
	//push go's starting position into queue
	m_queue.push(go->curr);
	//mark current node as visited
	m_visited[Get1DIndex(go->curr.x, go->curr.y)] = true;
	int nearestDistance = INT_MAX;
	MazePt nearestTile = go->curr;
	int loop = 0;
	MazePt curr{};
	while (!m_queue.empty() && loop < limit)
	{
		++loop;
		//set current active node in the search
		curr = m_queue.front();
		m_queue.pop();
		//calculate mahantten distance from curr to end pt.
		//the goal here is to keep track of which node is closestto the end pt(naive tracking.doesn't take into account surroundingobstacles)
			//in case we exceeded search limit
			int distance = abs(m_end.x - curr.x) + abs(m_end.y -
				curr.y);
		if (distance < nearestDistance)
		{
			nearestDistance = distance;
			nearestTile = curr;
		}
		//construct shortest path
		if (curr.x == m_end.x && curr.y == m_end.y)
		{
			while (curr.x != go->curr.x || curr.y != go->curr.y)
			{
				go->path.insert(go->path.begin(), curr);
				curr = m_previous[Get1DIndex(curr.x, curr.y)];
			}go->path.insert(go->path.begin(), curr);
			// Print out the shortest path found
			std::cout << go->id << " found the shortest path: ";
			for (int i = 0; i < go->path.size(); i++)
			{
				std::cout << "[" << go->path[i].x << ", " <<
					go->path[i].y << "]";
			}
			std::cout << std::endl;
			return true;
		}
		//up, down, left, right
		static int offsets[][2] = { {0, 1}, {0, -1}, {-1, 0}, {1,
		0} };
		MazePt next;
		int nextIndex{};
		for (int(&offset)[2] : offsets)
		{
			next.Set(curr.x + offset[0], curr.y + offset[1]);
			nextIndex = Get1DIndex(next.x, next.y);
			if (IsWithinBoundary(next.x) &&
				IsWithinBoundary(next.y) && //test next pos to see if it'swithin the map
				!m_visited[nextIndex] && go->grid[nextIndex] ==
				Maze::TILE_EMPTY) //visit next tile if it's empty and unvisited
			{
				m_previous[nextIndex] = curr;
				m_queue.push(next);
				m_visited[nextIndex] = true;
			}
		}
	}
	//exceeded search limit without finding destination.
	//construct shortest path to nearestTile instead of targeted end tile
		curr = nearestTile;
	while (curr.x != go->curr.x || curr.y != go->curr.y)
	{
		go->path.insert(go->path.begin(), curr);
		curr = m_previous[Get1DIndex(curr.x, curr.y)];
	}
	// Print out the shortest path to nearestTile
	std::cout << go->id << " received the shortest path to nearestTile: ";
		for (int i = 0; i < go->path.size(); i++)
		{
			std::cout << "[" << go->path[i].x << ", " << go->path[i].y << "]";
		}
	std::cout << std::endl;
	return false;
}

//helper function to check if given index is within boundary
int SceneTurn::IsWithinBoundary(int x) const
{
	return x >= 0 && x < m_noGrid;
}

//helper function to convert 2d indices to 1d index
int SceneTurn::Get1DIndex(int x, int y) const
{
	return y * m_noGrid + x;
}


static int HeuristicManhattan(const MazePt& a, const MazePt& b)
{
	return abs(a.x - b.x) + abs(a.y - b.y) * 1;
}

bool SceneTurn::AStar(GameObject* go, MazePt start, MazePt end)
{
	const int N = m_noGrid * m_noGrid;

	go->path.clear();


	// A* arrays

	const int INF = INT_MAX / 4;

	std::vector<int> gCost(N, INF);
	std::vector<int> fCost(N, INF);
	std::vector<int> prev(N, -1);

	//open to hold what to check, closed to hold what is already in
	std::vector<int> open;
	std::vector<bool> closed(N, false);

	int startIdx = Get1DIndex(start.x, start.y);
	int endIdx = Get1DIndex(end.x, end.y);

	// if end is not walkable (fog/wall), immediately fail
	if (GetTileCost(go->grid[endIdx]) == INT_MAX)
		return false;

	gCost[startIdx] = 0;
	fCost[startIdx] = HeuristicManhattan(start, end);
	open.push_back(startIdx);

	static int offsets[][2] = { {0,1}, {0,-1}, {-1,0}, {1,0} };

	while (!open.empty())
	{
		// find node in open with lowest fCost (tie-breaker: lower gCost)
		int bestPos = 0;
		for (int i = 1; i < (int)open.size(); ++i)
		{
			int a = open[i];
			int b = open[bestPos];

			if (fCost[a] < fCost[b] || (fCost[a] == fCost[b] && gCost[a] < gCost[b]))
				bestPos = i;
		}

		int currIdx = open[bestPos];
		open.erase(open.begin() + bestPos);

		if (closed[currIdx]) // already closed
			continue;

		closed[currIdx] = true; // close it

		if (currIdx == endIdx)
		{
			// reconstruct path end -> start using prev[]
			int walk = endIdx;
			while (walk != -1)
			{
				go->path.insert(go->path.begin(), MazePt(walk % m_noGrid, walk / m_noGrid));
				if (walk == startIdx) break;
				walk = prev[walk];
			}

			// if we didn't reach start, then reconstruction failed
			if (go->path.empty() || !(go->path.front().x == start.x && go->path.front().y == start.y))
				return false;

			return true;
		}

		MazePt curr = MazePt(currIdx % m_noGrid, currIdx / m_noGrid);

		// explore neighbours
		for (int(&off)[2] : offsets)
		{
			MazePt next(curr.x + off[0], curr.y + off[1]);
			if (!IsWithinBoundary(next.x) || !IsWithinBoundary(next.y))
				continue;

			int nextIdx = Get1DIndex(next.x, next.y);

			if (closed[nextIdx]) // already closed
				continue;

			int tileCost = GetTileCost(go->grid[nextIdx]);
			if (tileCost == INT_MAX) continue;

			int tentativeG = gCost[currIdx] + tileCost;

			if (tentativeG < gCost[nextIdx])
			{
				prev[nextIdx] = currIdx;
				gCost[nextIdx] = tentativeG;
				fCost[nextIdx] = tentativeG + HeuristicManhattan(next, end);

				// add to open if not already there
				if (std::find(open.begin(), open.end(), nextIdx) == open.end())
					open.push_back(nextIdx);
			}
		}
	}

	// no path found
	return false;
}

int SceneTurn::GetTileCost(Maze::TILE_CONTENT tile)
{
	switch (tile)
	{
	case Maze::TILE_EMPTY: return 1;
	case Maze::TILE_SLOW: return INT_MAX;
	case Maze::TILE_FOG:   return INT_MAX;
	case Maze::TILE_WALL:  return INT_MAX; // not walkable
	}
	return 1;
}

bool SceneTurn::TryFindFrontierTarget(GameObject* go, const MazePt& goal, MazePt& outTarget)
{
	bool found = false;
	int bestScore = INT_MAX;

	static int offsets[][2] = { {0,1}, {0,-1}, {-1,0}, {1,0} };

	for (int y = 0; y < m_noGrid; ++y)
		for (int x = 0; x < m_noGrid; ++x)
		{
			if (x == go->curr.x && y == go->curr.y)
				continue;
			int idx = Get1DIndex(x, y);
			if (go->grid[idx] != Maze::TILE_EMPTY) continue; // must be known walkable

			// Frontier = known empty tile that touches fog
			bool touchesFog = false;
			for (int(&off)[2] : offsets)
			{
				int nx = x + off[0], ny = y + off[1];
				if (!IsWithinBoundary(nx) || !IsWithinBoundary(ny)) continue;

				int nIdx = Get1DIndex(nx, ny);
				if (go->grid[nIdx] == Maze::TILE_FOG)
				{
					touchesFog = true;
					break;
				}
			}
			if (!touchesFog) continue;

			// Pick frontier that is closest to the final goal
			MazePt p(x, y);
			int score = HeuristicManhattan(p, goal);
			if (score < bestScore)
			{
				bestScore = score;
				outTarget = p;
				found = true;
			}
		}

	return found;
}

void SceneTurn::RevealAround(GameObject* go)
{
	MazePt curr = go->curr;
	static int offsets[][2] = { {0,1},{0,-1},{-1,0},{1,0} };

	// always mark current as empty
	go->grid[Get1DIndex(curr.x, curr.y)] = Maze::TILE_EMPTY;

	for (int(&off)[2] : offsets)
	{
		MazePt next(curr.x + off[0], curr.y + off[1]);
		if (!IsWithinBoundary(next.x) || !IsWithinBoundary(next.y)) continue;

		int idx = Get1DIndex(next.x, next.y);
		go->grid[idx] = m_maze.See(next); // writes into NPC memory
	}
}

void SceneTurn::PathFind(GameObject* go, const MazePt& goal, int moveBudget)
{
	if (moveBudget <= 0) return;

	// loop until you run out of movement
	while (moveBudget > 0)
	{
		// Always reveal from current position (so grid knowledge grows)
		RevealAround(go);

		// Decide whether we should try goal, or explore frontier
		bool goalIsKnown = (go->grid[Get1DIndex(goal.x, goal.y)] != Maze::TILE_FOG);

		bool havePlan = false;

		if (goalIsKnown)
		{
			// Try to plan directly to goal (known tiles only)
			havePlan = AStar(go, go->curr, goal) && go->path.size() > 1;
		}

		if (!havePlan)
		{
			// Goal unknown OR unreachable -> go to frontier
			MazePt frontier;
			if (!TryFindFrontierTarget(go, goal, frontier))
				return; // nowhere new to explore

			havePlan = AStar(go, go->curr, frontier) && go->path.size() > 1;
			if (!havePlan)
				return;
		}

		// Next step is path[1]
		MazePt next = go->path[1];

		// Check movement cost for stepping into that tile
		int stepCost = GetTileCost(go->grid[Get1DIndex(next.x, next.y)]);
		if (stepCost > moveBudget)
			return; // can't afford to move further this turn

		// Spend movement + move
		moveBudget -= stepCost;
		go->curr = next;

		// Learn from the new tile immediately (recommended)
		RevealAround(go);

		// loop continues: with new info, it may switch from frontier mode to goal mode automatically
	}
}



