#pragma once
	static const int m_noGrid = 20;
	static const float m_worldHeight = 100.f; //starting to realise this was originally done in aspect ratio
	static const float m_worldWidth = m_worldHeight;// i changed it fixed ratio, now it screws up multiple calculations
	static const float m_gridSize = m_worldHeight / m_noGrid; //oh well too late the whole scene is already done
