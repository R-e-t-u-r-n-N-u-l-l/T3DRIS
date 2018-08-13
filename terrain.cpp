#include "terrain.h"

Terrain::Terrain(engine::Vector3f size) : m_shader("resources/terrain.vs", "resources/terrain.fs"), m_size(size), m_blockModel(engine::Shape3D::cube(0.5f).createModel()), 
	m_removeTimer(300), m_removeColor(new engine::Vector4f(0.2f, 0.2f, 0.2f, 1.0f)), m_instancedRender(m_blockModel.getVAO()) {

	for (int i = 0; i < m_size.x * m_size.y * m_size.z; i++)
		m_blocks.push_back(nullptr);

	m_instancedRender.addInstancedAttribute(3, 3, m_blocks.size());
	m_instancedRender.addInstancedAttribute(4, 4, m_blocks.size());
}

void Terrain::updateInstances(GLfloat*& vectors, GLfloat*& colors) {
	std::vector<GLfloat> rawVectors;
	std::vector<GLfloat> rawColors;

	rawVectors.reserve(m_blockCount * 3);
	rawColors.reserve(m_blockCount * 4);

	m_blockCount = 0;

	for (int i = 0; i < m_size.x; i++) {
		for (int j = 0; j < m_size.y; j++) {
			for (int k = 0; k < m_size.z; k++) {
				if (!m_blocks[(k * m_size.y + j) * m_size.x + i])
					continue;

				rawVectors.push_back(i - m_size.x / 2.0f);
				rawVectors.push_back(j - m_size.y / 2.0f);
				rawVectors.push_back(k - m_size.z / 2.0f);

				rawColors.push_back(m_blocks[(k * m_size.y + j) * m_size.x + i]->x);
				rawColors.push_back(m_blocks[(k * m_size.y + j) * m_size.x + i]->y);
				rawColors.push_back(m_blocks[(k * m_size.y + j) * m_size.x + i]->z);
				rawColors.push_back(m_blocks[(k * m_size.y + j) * m_size.x + i]->w);

				m_blockCount++;
			}
		}
	}

	vectors = new GLfloat[m_blockCount * 3];
	for (int i = 0; i < rawVectors.size(); i++)
		vectors[i] = rawVectors[i];

	colors = new GLfloat[m_blockCount * 4];
	for (int i = 0; i < rawColors.size(); i++)
		colors[i] = rawColors[i];
}

void Terrain::update() {
	if (m_removeRow)
		if (m_removeTimer.ready())
			remove();
}

void Terrain::render(engine::Shader* shader, const engine::Matrix4f& projection, const engine::Matrix4f& view, const engine::Light& light, bool shadow) {
	if (!shader)
		shader = &m_shader;

	shader->enable();
	shader->setUniformMatrix4f(shader->getUniformLocation("projection"), projection);
	shader->setUniformMatrix4f(shader->getUniformLocation("view"), view);
	if (!shadow) {
		shader->setUniform3f(shader->getUniformLocation("lightPosition"), light.getPosition());
		shader->setUniform4f(shader->getUniformLocation("lightColor"), light.getColor());
	}

	//updateInstances(m_vectors, m_colors);

	//m_instancedRender.updateAttribute(0, 3, m_blockCount, m_vectors);
	//m_instancedRender.updateAttribute(1, 4, m_blockCount, m_colors);

	//engine::Render::renderBatch(m_instancedRender, m_blockModel, m_blockCount);

	m_blockModel.bind();

	for (int i = 0; i < m_size.x; i++) {
		for (int j = 0; j < m_size.y; j++) {
			for (int k = 0; k < m_size.z; k++) {
				if (!m_blocks[(k * m_size.y + j) * m_size.x + i])
					continue;
				shader->setUniform3f(shader->getUniformLocation("blockPosition"), (i - m_size.x / 2.0f), (j - m_size.y / 2.0f), (k - m_size.z / 2.0f));
				if (!shadow)
					shader->setUniform4f(shader->getUniformLocation("blockColor"), *m_blocks[(k * m_size.y + j) * m_size.x + i]);
				engine::Render::renderNoBind(m_blockModel.getIndexLength());
			}
		}
	}

	m_blockModel.unbind();

	shader->disable();
}

void Terrain::set(int x, int y, int z, engine::Vector4f* color) {
	m_blocks[(z * m_size.y + y) * m_size.x + x] = color;
}

void Terrain::remove() {
	int count = 0;
	for (int i = 1; i < m_size.x - 1; i++)
		for (int j = 1; j < m_size.y - 1; j++)
			for (int k = 1; k < m_size.z - 1; k++)
				if (m_blocks[(k * m_size.y + j) * m_size.x + i] == m_removeColor)
					count += removeRow(i, j, k);

	addScore(count * count);
}

int Terrain::removeRow(int x, int y, int z) {
	int count = 1;
	for (int l = y; l < m_size.y - 1; l++)
		m_blocks[(z * m_size.y + l) * m_size.x + x] = m_blocks[(z * m_size.y + l + 1) * m_size.x + x];

		if (m_blocks[(z * m_size.y + y) * m_size.x + x] == m_removeColor)
		count += removeRow(x, y, z);

	return count;
}

bool Terrain::check(int x, int y, int z) {
	bool ready = false;
	bool valid = true;
	for (int i = 1; i < m_size.z - 1; i++)
		if (!m_blocks[(i * m_size.y + y) * m_size.x + x])
			valid = false;

	if (valid) {
		m_removeRow = true;
		m_removeTimer.reset();
		for (int i = 1; i < m_size.z - 1; i++)
			m_blocks[(i * m_size.y + y) * m_size.x + x] = m_removeColor;
	}

	if (valid)
		ready = true;

	valid = true;
	for (int i = 1; i < m_size.x - 1; i++)
		if (!m_blocks[(z * m_size.y + y) * m_size.x + i])
			valid = false;

	if (valid) {
		m_removeRow = true;
		m_removeTimer.reset();
		for (int i = 1; i < m_size.x - 1; i++)
			m_blocks[(z * m_size.y + y) * m_size.x + i] = m_removeColor;
	}

	return valid ? true : ready;
}

bool Terrain::isReady() const {
	for (int i = 1; i < m_size.x - 1; i++)
		for (int j = 1; j < m_size.y - 1; j++)
			for (int k = 1; k < m_size.z - 1; k++)
				if (m_blocks[(k * m_size.y + j) * m_size.x + i] == m_removeColor)
					return false;

	return true;
}

const std::vector<engine::Vector4f*>& Terrain::getBlocks() const {
	return m_blocks;
}

engine::Shader& Terrain::getShader() {
	return m_shader;
}

const engine::Vector3f& Terrain::getSize() const {
	return m_size;
}

void Terrain::addScore(int score) {
	m_score += score;
}

int Terrain::getScore() const {
	return m_score;
}
