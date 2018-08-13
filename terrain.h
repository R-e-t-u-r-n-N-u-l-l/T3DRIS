#pragma once

#include <vector>

#include "maths/maths.h"
#include "graphics/shader.h"
#include "graphics/render.h"
#include "graphics/instancedRender.h"
#include "entities/light.h"
#include "models/model.h"
#include "utilities/primitives.h"
#include "utilities/timer.h"

class Terrain {

private:
	std::vector<engine::Vector4f*> m_blocks;
	GLfloat* m_vectors, *m_colors;
	engine::InstancedRender m_instancedRender;
	const engine::Model m_blockModel;
	const engine::Vector3f m_size;
	engine::Vector4f* m_removeColor;
	engine::Shader m_shader;
	engine::Timer m_removeTimer;
	bool m_removeRow;
	int m_blockCount;
	int m_score;

	void remove();
	int removeRow(int x, int y, int z);
	void updateInstances(GLfloat*& vectors, GLfloat*& colors);

public:
	Terrain(engine::Vector3f size);

	void update();
	void render(engine::Shader* shader, const engine::Matrix4f& projection, const engine::Matrix4f& view, const engine::Light& light, bool shadow = false);

	void set(int x, int y, int z, engine::Vector4f* color);
	bool check(int x, int y, int z);

	bool isReady() const;
	const std::vector<engine::Vector4f*>& getBlocks() const;
	engine::Shader& getShader();
	const engine::Vector3f& getSize() const;
	void addScore(int score);
	int getScore() const;

};
