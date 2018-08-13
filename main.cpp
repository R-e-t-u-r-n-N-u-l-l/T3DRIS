#include "window/window.h"
#include "graphics/render.h"
#include "graphics/shader.h"
#include "entities/light.h"
#include "graphics/skybox.h"
#include "entities/camera.h"
#include "utilities/timer.h"
#include "maths/random/noise.h"
#include "graphics/gui/font.h"
#include "graphics/shadow.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "terrain.h"
#include "block.h"

int main() {
	const char* icons[] = {
		"resources/icon128.png"
	};

	engine::Window window("T3DRIS");
	window.setWindowSize(window.getWidth() - 100 * window.getAspectRatio(), window.getHeight() - 100);
	window.setIcon(icons, 1);
	window.setMinVersion(3, 3);
	window.lockCursor();
	window.enableDepthTest();
	window.enableTransparancy();

	int n = 0;
	const GLFWvidmode* vidmode = glfwGetVideoMode(glfwGetMonitors(&n)[0]);

	window.setPosition((vidmode->width - window.getWidth()) / 2, (vidmode->height - window.getHeight()) / 2);

	const int GRID_SIZE = 12;
	bool ortho = false;

	GLuint nextImage = engine::File::loadTextureID("resources/next.png");
	engine::Light light(engine::Vector3f(3.0f, 30.0f, -10.0f), engine::Vector4f(1.0f, 1.0f, 1.0f, 1.0f));
	
	engine::Shader shadowShader("resources/shadow.vs", "resources/shadow.fs");
	engine::Shader nextShader("resources/next.vs", "resources/next.fs");

	engine::Font font("resources/font.png", "resources/font.fnt");
	glBindTexture(GL_TEXTURE_2D, font.getTexture());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	font.enableShader();
	font.getShader().setUniform3f(font.getShader().getUniformLocation("textColor"), engine::Vector3f(1.0f));
	font.disableShader();

	engine::Camera camera(engine::Vector3f(0.0f, 0.0f, 0.0f), window.getWidth(), window.getHeight());
	engine::Entity cameraObject;

	engine::Matrix4f transformation;
	engine::Matrix4f projection = engine::Matrix4f::perspective(70.0f, window.getAspectRatio(), 0.1f, 200.0f);
	engine::Matrix4f viewMatrix = engine::Maths::createViewMatrix(camera.getPosition(), camera.getRotation());

	const char* paths[] = {
		"resources/back.png",
		"resources/back.png",
		"resources/back.png",
		"resources/bottom.png",
		"resources/back.png",
		"resources/back.png",
	};

	engine::Skybox skybox(100.0f, paths);
	const engine::Model blockModel = engine::Shape3D::cube(0.5f).createModel(true, false);

	Terrain terrain({ float(GRID_SIZE), 21, float(GRID_SIZE) });

	engine::Random random;

	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			terrain.set(i, 0, j, new engine::Vector4f(0.5f, 0.8f, 1.0f, 1.0f));
			for (int k = 0; k < terrain.getSize().y - 1; k++)
				if (i % (GRID_SIZE - 1) == 0 || j == GRID_SIZE - 1 || (j == 0 && k == 0))
					terrain.set(i, k, j, &Block::COLORS[int(random.next() * 5)]);
		}
	}
	
	bool gameOver = false;
	bool paused = true;
	int highScore = 0;
	float fontSize = 0.2f;
	float speed = 1000;
	engine::Timer spawnTimer(300);
	Block* next = new Block(terrain, speed, random.next() * 5);
	Block* currentBlock = new Block(terrain, speed, random.next() * 5);
	currentBlock->draw();

	engine::Shadow shadow(2048);

	engine::Matrix4f lightProjection = engine::Matrix4f::ortho(-30.0f, 30.0f, -30.0f, 30.0f, 1.0f, 80.0f);
	engine::Matrix4f lightView = engine::Matrix4f::lookingAt(light.getPosition(), engine::Vector3f(), engine::Vector3f(0.0f, 1.0f, 0.0f));

	std::string level = "0";

	engine::Timer timer(1000);
	int frames = 0;
	int realFrames = 0;

	while (window.isOpen()) {

		if (timer.ready()) {
			realFrames = frames;
			window.setTitle(std::string("T3DRIS - FPS: " + std::to_string(frames)).c_str());
			frames = 0;
		}

		//update
		if (window.canUpdate()) {
			if (!gameOver && !paused) {
				if (!currentBlock && terrain.isReady()) {
					if (spawnTimer.ready()) {
						currentBlock = new Block(terrain, speed, next->getIndex());
						currentBlock->draw();
						next = new Block(terrain, speed, random.next() * 5);
						speed *= 0.985f;
						level = std::to_string(int(floor(20 - speed / 50)));
					}
				}

				if (currentBlock) {
					if (!currentBlock->update()) {
						if (currentBlock->gameOver) {
							gameOver = true;
							highScore = terrain.getScore();
						}
						currentBlock = nullptr;
					}
				}
			}

			if (engine::Input::keyPressed(GLFW_KEY_P))
				paused = !paused;

			if (gameOver && engine::Input::keyPressed(GLFW_KEY_ENTER)) {
				for (int i = 1; i < terrain.getSize().x - 1; i++)
					for (int j = 1; j < terrain.getSize().y - 1; j++)
						for (int k = 0; k < terrain.getSize().z - 1; k++)
							terrain.set(i, j, k, nullptr);

				speed = 1000;
				terrain.addScore(-terrain.getScore());
				gameOver = false;
			}

			if (engine::Input::keyPressed(GLFW_KEY_TAB)) {
				ortho = !ortho;
				if (ortho)
					projection = engine::Matrix4f::ortho(-1 * window.getWidth() / 65.0f, 1 * window.getWidth() / 65.0f, -1 * window.getHeight() / 65.0f, 1 * window.getHeight() / 65.0f, 0.1f, 200.0f);
				else
					projection = engine::Matrix4f::perspective(70.0f, window.getAspectRatio(), 0.1f, 200.0f);
			}

			terrain.update();

			camera.focusOnEntity(cameraObject, 0, GRID_SIZE * (1 + !ortho) + terrain.getSize().y * cos(camera.getPitch()) / 3.0f, 0);

			camera.setPitch(camera.getPitch() - engine::Input::mouse_dy / window.getWidth());
			camera.setPitch(camera.getPitch() < M_PI / 10 ? M_PI / 10 : camera.getPitch() > M_PI / 2 ? M_PI / 2 : camera.getPitch());

			viewMatrix = engine::Maths::createViewMatrix(camera.getPosition(), camera.getRotation());

			window.update();

			//render
			engine::Render::clear();
			glViewport(0, 0, shadow.getSize(), shadow.getSize());
			glBindFramebuffer(GL_FRAMEBUFFER, shadow.getShadowFBO());
			glClear(GL_DEPTH_BUFFER_BIT);

			terrain.render(&shadowShader, lightProjection, lightView, light, true);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			engine::Render::clear();
			glViewport(0, 0, window.getWidth(), window.getHeight());
			glBindTexture(GL_TEXTURE_2D, shadow.getShadowMap());

			terrain.getShader().enable();

			terrain.getShader().setUniform1f(terrain.getShader().getUniformLocation("shadowMapSize"), shadow.getSize());
			terrain.getShader().setUniformMatrix4f(terrain.getShader().getUniformLocation("lightProjection"), lightProjection);
			terrain.getShader().setUniformMatrix4f(terrain.getShader().getUniformLocation("lightView"), lightView);

			terrain.getShader().disable();

			terrain.render(nullptr, projection, viewMatrix, light);
			skybox.render(projection, camera.getRotation());

			nextShader.enable();
			nextShader.setUniformMatrix4f(nextShader.getUniformLocation("transformation"), engine::Maths::createTransformationMatrix(
				engine::Vector3f(-0.925f, -0.2f, 0), engine::Vector3f(-M_PI / 2.0f, 0, 0), engine::Vector3f(0.09f, 0.1f, 0.16f)));

			nextShader.setUniform4f(nextShader.getUniformLocation("blockColor"), Block::COLORS[next->getIndex()]);

			blockModel.bind();
			engine::Vector3f reference = next->getBlocks()[0];
			for (const engine::Vector3f& v : next->getBlocks()) {
				nextShader.setUniform3f(nextShader.getUniformLocation("blockPosition"), v - reference);
				engine::Render::renderNoBind(blockModel.getIndexLength());
			}

			blockModel.unbind();
			nextShader.disable();

			font.enableShader();
			font.getShader().setUniform2f(font.getShader().getUniformLocation("location"), engine::Vector2f(0.01f, 0.01f));
			font.render("TOP", fontSize);
			font.getShader().setUniform2f(font.getShader().getUniformLocation("location"), engine::Vector2f(0.01f, font.getTextHeight("TOP", fontSize) + 0.01f));
			font.render(std::to_string(highScore), fontSize);
			font.getShader().setUniform2f(font.getShader().getUniformLocation("location"), engine::Vector2f(0.01f, font.getTextHeight("TOP", fontSize) + font.getTextHeight(std::to_string(terrain.getScore()), fontSize) + 0.05f));
			font.render("SCORE", fontSize);
			font.getShader().setUniform2f(font.getShader().getUniformLocation("location"), engine::Vector2f(0.01f, font.getTextHeight("TOP", fontSize) + font.getTextHeight(std::to_string(terrain.getScore()), fontSize) + font.getTextHeight("SCORE", fontSize) + 0.05f));
			font.render(std::to_string(terrain.getScore()), fontSize);
			font.getShader().setUniform2f(font.getShader().getUniformLocation("location"), engine::Vector2f(0.01f, font.getTextHeight("TOP", fontSize) + font.getTextHeight(std::to_string(terrain.getScore()), fontSize) + font.getTextHeight("SCORE", fontSize) + font.getTextHeight(std::to_string(terrain.getScore()), fontSize) + 0.09f));
			font.render("NEXT", fontSize);
			font.getShader().setUniform2f(font.getShader().getUniformLocation("location"), engine::Vector2f(0.99f - font.getTextWidth("LEVEL", fontSize), 0.01f));
			font.render("LEVEL", fontSize);
			font.getShader().setUniform2f(font.getShader().getUniformLocation("location"), engine::Vector2f(0.99f - font.getTextWidth("LEVEL", fontSize), font.getTextHeight("LEVEL", fontSize) + 0.01f));
			font.render(level, fontSize);

			font.getShader().setUniform2f(font.getShader().getUniformLocation("location"), engine::Vector2f(0.99f - font.getTextWidth(ortho ? "ORTHOGRAPHIC" : "PERSPECTIVE", fontSize), 0.99f - font.getTextHeight(ortho ? "ORTHOGRAPHIC" : "PERSPECTIVE", fontSize)));
			font.render(ortho ? "ORTHOGRAPHIC" : "PERSPECTIVE", fontSize);

			if (gameOver) {
				font.getShader().setUniform2f(font.getShader().getUniformLocation("location"), engine::Vector2f(0.5f - font.getTextWidth("GAME OVER", 0.5f) / 2, 0.45f - font.getTextHeight("GAME OVER", 0.5f) / 2));
				font.render("GAME OVER", 0.5f);
				font.getShader().setUniform2f(font.getShader().getUniformLocation("location"), engine::Vector2f(0.5f - font.getTextWidth("PRESS ENTER TO PLAY AGAIN", fontSize) / 2, 0.45f + font.getTextHeight("GAME OVER", 0.5f) / 2));
				font.render("PRESS ENTER TO PLAY AGAIN", fontSize);
			}
			else if (paused) {
				font.getShader().setUniform2f(font.getShader().getUniformLocation("location"), engine::Vector2f(0.5f - font.getTextWidth("PAUSED", 0.5f) / 2, 0.45f - font.getTextHeight("PAUSED", 0.5f) / 2));
				font.render("PAUSED", 0.5f);
				font.getShader().setUniform2f(font.getShader().getUniformLocation("location"), engine::Vector2f(0.5f - font.getTextWidth("PRESS P TO CONTINUE", fontSize) / 2, 0.45f + font.getTextHeight("PAUSED", 0.5f) / 2));
				font.render("PRESS P TO CONTINUE", fontSize);
			}

			font.disableShader();

			frames++;

			window.sync();
		}
	}

	return 0;
}
