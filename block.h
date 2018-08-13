#pragma once

#include <vector>
#include <algorithm>

#include "utilities/timer.h"
#include "maths/random/random.h"
#include "maths/maths.h"
#include "window/input.h"

#include "terrain.h"

class Block {

private:
	std::vector<engine::Vector3f> m_blocks;
	engine::Vector4f* m_color;
	engine::Timer m_updateTimer;
	int m_middle;
	unsigned m_index;
	Terrain& terrain;
	bool oValid;

public:
	static engine::Vector4f COLORS[5];
	bool gameOver;

	Block(Terrain& terrain, int speed, unsigned index);

	void draw();
	bool update();

	std::vector<engine::Vector3f> getBlocks() const;
	unsigned getIndex() const;

};

