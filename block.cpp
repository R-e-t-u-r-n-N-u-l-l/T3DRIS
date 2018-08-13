#include "block.h"

engine::Vector4f Block::COLORS[5] = {
	engine::Vector4f(1.0f, 0.0f, 0.0f, 1.0f),
	engine::Vector4f(0.0f, 0.0f, 1.0f, 1.0f),
	engine::Vector4f(0.5f, 0.0f, 0.5f, 1.0f),
	engine::Vector4f(0.0f, 1.0f, 0.0f, 1.0f),
	engine::Vector4f(1.0f, 1.0f, 0.0f, 1.0f)
};

Block::Block(Terrain& terrain, int speed, unsigned index) : terrain(terrain), m_updateTimer(speed), oValid(true), m_index(index) {
	int x = terrain.getSize().x / 2 - 1;
	int z = terrain.getSize().z / 2 - 1;

	switch (index) {
	case 0: { // Cube
		m_middle = 0;

		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 2; j++)
				for (int k = 0; k < 2; k++)
					m_blocks.push_back(engine::Vector3f(x + i, terrain.getSize().y - j - 2, z + k));
		}
		break;

	case 1: { // I
		m_middle = 2;

		for (int i = 0; i < 4; i++)
			m_blocks.push_back(engine::Vector3f(x + i, terrain.getSize().y - 2, z));
		}
		break;

	case 2: { // L
		m_middle = 0;

		for (int i = 0; i < 3; i++)
			m_blocks.push_back(engine::Vector3f(x + i, terrain.getSize().y - 2, z));

		m_blocks.push_back(engine::Vector3f(x, terrain.getSize().y - 2, z + 1));
		}
		break;

	case 3: { // S
		m_middle = 2;

		for (int i = 0; i < 2; i++) {
			m_blocks.push_back(engine::Vector3f(x + i + 0, terrain.getSize().y - 2, z + 0));
			m_blocks.push_back(engine::Vector3f(x + i + 1, terrain.getSize().y - 2, z + 1));
		}
		}
		break;

	default: { // T
		m_middle = 1;

		for (int i = 0; i < 3; i++)
			m_blocks.push_back(engine::Vector3f(x + i, terrain.getSize().y - 2, z));

		m_blocks.push_back(engine::Vector3f(x + 1, terrain.getSize().y - 2, z + 1));
		}
		break;
	}

	m_color = &COLORS[index < 4 ? index : 4];
}

void Block::draw() {
	for (const engine::Vector3f& v : m_blocks)
		terrain.set(v.x, v.y, v.z, m_color);
}

bool Block::update() {
	bool valid = true;
	for (const engine::Vector3f& v : m_blocks)
		terrain.set(v.x, v.y, v.z, nullptr);

	for (const engine::Vector3f& v : m_blocks)
		if (terrain.getBlocks()[(v.z * terrain.getSize().y + (v.y - 1)) * terrain.getSize().x + v.x])
			valid = false;

	bool ready = m_updateTimer.ready();

	if (ready && valid)
		for (engine::Vector3f& v : m_blocks)
			v.y -= 1;

	if (valid && engine::Input::keyPressed(GLFW_KEY_SPACE)) {
		bool valid = true;
		m_updateTimer.reset();
		while (valid) {
			for (const engine::Vector3f& v : m_blocks)
				if (terrain.getBlocks()[(v.z * terrain.getSize().y + (v.y - 1)) * terrain.getSize().x + v.x])
					valid = false;

			if (valid)
				for (engine::Vector3f& v : m_blocks)
					v.y -= 1;
		}
	}

	oValid = valid;
	valid = false;

	if (m_index > 0 && engine::Input::keyPressed(GLFW_KEY_E)) {
		engine::Vector3f middle = m_blocks[m_middle];
		std::vector<engine::Vector3f> copy = m_blocks;
		for (int i = 0; i < m_blocks.size(); i++) {
			engine::Vector3f difference = middle - m_blocks[i];
			float temp = difference.x;
			difference.x = -difference.z;
			difference.z = temp;
			float y = m_blocks[i].y;
			m_blocks[i] = middle + difference;
			m_blocks[i].y = y;
		}

		for (engine::Vector3f& v : m_blocks) {
			if ((v.z * terrain.getSize().y + v.y) * terrain.getSize().x + v.x < 0 || (v.z * terrain.getSize().y + v.y) * terrain.getSize().x + v.x > terrain.getBlocks().size() || 
				terrain.getBlocks()[(v.z * terrain.getSize().y + v.y) * terrain.getSize().x + v.x]) {
				m_blocks = copy;
				break;
			}
		}
	}

	if (m_index > 0 && engine::Input::keyPressed(GLFW_KEY_Q)) {
		engine::Vector3f middle = m_blocks[m_middle];
		std::vector<engine::Vector3f> copy = m_blocks;
		for (int i = 0; i < m_blocks.size(); i++) {
			engine::Vector3f difference = middle - m_blocks[i];
			float temp = difference.x;
			difference.x = -difference.y;
			difference.y = temp;
			float z = m_blocks[i].z;
			m_blocks[i] = middle + difference;
			m_blocks[i].z = z;
		}

		for (engine::Vector3f& v : m_blocks) {
			if ((v.z * terrain.getSize().y + v.y) * terrain.getSize().x + v.x < 0 || (v.z * terrain.getSize().y + v.y) * terrain.getSize().x + v.x > terrain.getBlocks().size() ||
				terrain.getBlocks()[(v.z * terrain.getSize().y + v.y) * terrain.getSize().x + v.x]) {
				m_blocks = copy;
				break;
			}
		}
	}

	if (engine::Input::keyPressed(GLFW_KEY_D)) {
		valid = true;
		for (engine::Vector3f& v : m_blocks) {
			if (v.x - 1 <= 0 || terrain.getBlocks()[(v.z * terrain.getSize().y + v.y) * terrain.getSize().x + (v.x - 1)]) {
				valid = false;
				break;
			}
		}
		if (valid)
			for (engine::Vector3f& v : m_blocks)
				v.x -= 1;
	}

	if (engine::Input::keyPressed(GLFW_KEY_A)) {
		valid = true;
		for (engine::Vector3f& v : m_blocks) {
			if (v.x + 1 >= terrain.getSize().x - 1 || terrain.getBlocks()[(v.z * terrain.getSize().y + v.y) * terrain.getSize().x + (v.x + 1)]) {
				valid = false;
				break;
			}
		}
		if (valid)
			for (engine::Vector3f& v : m_blocks)
				v.x += 1;
	}

	if (engine::Input::keyPressed(GLFW_KEY_S)) {
		valid = true;
		for (engine::Vector3f& v : m_blocks) {
			if (v.z - 1 <= 0 || terrain.getBlocks()[((v.z - 1) * terrain.getSize().y + v.y) * terrain.getSize().x + v.x]) {
				valid = false;
				break;
			}
		}
		if (valid)
			for (engine::Vector3f& v : m_blocks)
				v.z -= 1;
	}

	if (engine::Input::keyPressed(GLFW_KEY_W)) {
		valid = true;
		for (engine::Vector3f& v : m_blocks) {
			if (v.z + 1 >= terrain.getSize().z - 1 || terrain.getBlocks()[((v.z + 1) * terrain.getSize().y + v.y) * terrain.getSize().x + v.x]) {
				valid = false;
				break;
			}
		}
		if (valid)
			for (engine::Vector3f& v : m_blocks)
				v.z += 1;
	}

	draw();

	if (!oValid) {
		for (const engine::Vector3f& v : m_blocks) {
			if (v.y == terrain.getSize().y - 2)
				gameOver = true;
			if (terrain.check(v.x, v.y, v.z))
				ready = true;
		}
	}

	if (ready && !oValid)
		return false;
	else
		return true;
}

std::vector<engine::Vector3f> Block::getBlocks() const {
	return m_blocks;
}

unsigned Block::getIndex() const {
	return m_index;
}
