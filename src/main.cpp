#include "gl.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "Camera.h"
#include "Player.h"
#include "Timer.h"
#include "World.h"
#include "globals.h"
#include "opengl/Program.h"
#include "opengl/Shader.h"
#include "utils.h"

const static unsigned int initialWindowWidth = 1024;
const static unsigned int initialWindowHeight = 768;
const static string windowCaption = "noise";

bool captureMouse = false;
glm::dvec2 mouseDownPos;

GLFWwindow* mainwindow;

gl::Program shaderProgram;
gl::Program normalDebuggingProgram;
gl::Program coordsProgram;

Timer timer;
World world;
Camera camera;
Player player;

glm::mat4 projectionMatrix;

void resizeGLScene(GLFWwindow*, int width, int height) {
	if (height <= 0)
		height = 1;
	projectionMatrix = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
	glViewport(0, 0, width, height);
}

bool initGL() {
	glClearColor(0.3f, 0.5f, 0.7f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_MULTISAMPLE);

	// Shaders
	shaderProgram = gl::Program{
		gl::Shader(GL_VERTEX_SHADER, fs::path{"../src/shaders/main.vert"}),
		gl::Shader(GL_FRAGMENT_SHADER, fs::path{"../src/shaders/main.frag"})};

	normalDebuggingProgram = gl::Program{gl::Shader(GL_VERTEX_SHADER, fs::path{"../src/shaders/normals.vert"}),
		gl::Shader(GL_GEOMETRY_SHADER, fs::path{"../src/shaders/normals.geom"}),
		gl::Shader(GL_FRAGMENT_SHADER, fs::path{"../src/shaders/normals.frag"})};

	coordsProgram = gl::Program{
		gl::Shader(GL_VERTEX_SHADER, fs::path{"../src/shaders/coords.vert"}),
		gl::Shader(GL_FRAGMENT_SHADER, fs::path{"../src/shaders/coords.frag"}),
	};

	return true;
}

void update(double interval) {
	std::stringstream caption;
	caption << windowCaption << " @ " << fixed << setprecision(1) << timer.tps << " FPS";
	glfwSetWindowTitle(mainwindow, caption.str().c_str());

	// camera
	glm::dvec2 delta{0, 0};
	if (captureMouse) {
		glm::dvec2 pos;
		glfwGetCursorPos(mainwindow, &pos.x, &pos.y);
		delta = mouseDownPos - pos;
		glfwSetCursorPos(mainwindow, mouseDownPos.x, mouseDownPos.y);
	}

	uint8_t moveFlags = 0;
	if (glfwGetKey(mainwindow, GLFW_KEY_SPACE) == GLFW_PRESS) moveFlags |= Up;
	if (glfwGetKey(mainwindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) moveFlags |= Down;
	if (glfwGetKey(mainwindow, GLFW_KEY_W) == GLFW_PRESS) moveFlags |= Forward;
	if (glfwGetKey(mainwindow, GLFW_KEY_S) == GLFW_PRESS) moveFlags |= Backward;
	if (glfwGetKey(mainwindow, GLFW_KEY_A) == GLFW_PRESS) moveFlags |= Left;
	if (glfwGetKey(mainwindow, GLFW_KEY_D) == GLFW_PRESS) moveFlags |= Right;

	if (global::freeCamera)
		camera.update(interval, delta.x, delta.y, moveFlags);
	else
		player.update(interval, delta.x, delta.y, moveFlags, world, camera);

	world.update(camera);
}

void render() {
	if (global::showHud) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	// clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (global::polygonmode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// update shader states
	const glm::mat4 viewMatrix = camera.viewMatrix();
	const glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
	const glm::mat4 normalMatrix = transpose(inverse(viewMatrix));

	shaderProgram.use();
	glUniformMatrix4fv(shaderProgram.uniformLocation("uViewProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
	glUniformMatrix4fv(shaderProgram.uniformLocation("uViewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(shaderProgram.uniformLocation("uNormalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// render the world
	world.render();

	// render coordinate system
	if (global::showCoords) {
		coordsProgram.use();
		glUniformMatrix4fv(coordsProgram.uniformLocation("matrix"), 1, false, glm::value_ptr(viewProjectionMatrix));
		glDrawArrays(GL_LINES, 0, 12);
	}

	// render normals
	if (global::showNormals) {
		normalDebuggingProgram.use();
		glUniformMatrix4fv(normalDebuggingProgram.uniformLocation("uViewProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
		world.render();
	}

	glUseProgram(0);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(projectionMatrix));
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(viewMatrix));
	world.renderAuxiliary();

	// hud
	if (global::showHud) {
		ImGui::Checkbox("coords", &global::showCoords);
		ImGui::Checkbox("polygons", &global::polygonmode);
		ImGui::Checkbox("show triangles", &global::showTriangles);
		ImGui::Checkbox("show normals", &global::showNormals);
		ImGui::Checkbox("show chunks", &global::showChunks);
		ImGui::Checkbox("show voxels", &global::showVoxels);
		ImGui::SliderInt("chunk radius", &global::CAMERA_CHUNK_RADIUS, 1, 10);
		ImGui::SliderInt("octaves", &global::noise::octaves, 1, 10);

		if (ImGui::Button("regenerate"))
			world.clearChunks();

		ImGui::Checkbox("free camera", &global::freeCamera);

		{
			ImGui::Begin("Player");
			ImGui::LabelText("pos", "%f %f %f", camera.position.x, camera.position.y, camera.position.z);
			ImGui::LabelText("vel", "%f %f %f", player.velocity.x, player.velocity.y, player.velocity.z);
			ImGui::LabelText("on ground", "%d", player.onGround);
			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}

void onMouseButton(GLFWwindow*, int button, int action, int modifiers) {
	ImGui_ImplGlfw_MouseButtonCallback(mainwindow, button, action, modifiers);

	if (action == GLFW_PRESS) {
		switch (button) {
			case GLFW_MOUSE_BUTTON_RIGHT:
				captureMouse = true;
				glfwGetCursorPos(mainwindow, &mouseDownPos.x, &mouseDownPos.y);
				glfwSetInputMode(mainwindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				break;
		}
	}

	if (action == GLFW_RELEASE) {
		switch (button) {
			case GLFW_MOUSE_BUTTON_RIGHT:
				captureMouse = false;
				glfwSetInputMode(mainwindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				break;
		}
	}
}

void onMouseWheel(GLFWwindow*, double xOffset, double yOffset) {
	ImGui_ImplGlfw_ScrollCallback(mainwindow, xOffset, yOffset);
}

void onKey(GLFWwindow*, int key, int scancode, int action, int mods) {
	ImGui_ImplGlfw_KeyCallback(mainwindow, key, scancode, action, mods);

	if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_H:
				global::showHud = !global::showHud;
		}
	}
}

void onChar(GLFWwindow*, unsigned int codepoint) {
	ImGui_ImplGlfw_CharCallback(mainwindow, codepoint);
}

bool createSDLWindow(int width, int height) {
	if (!glfwInit())
		return false;

	glfwWindowHint(GLFW_DEPTH_BITS, 32);
	glfwWindowHint(GLFW_STENCIL_BITS, 0);
	glfwWindowHint(GLFW_FOCUSED, false);
#ifndef NDEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	mainwindow = glfwCreateWindow(width, height, windowCaption.c_str(), nullptr, nullptr);
	if (mainwindow == nullptr) {
		cerr << "Unable to create window" << endl;
		return false;
	}

	glfwMakeContextCurrent(mainwindow);

	glfwSetWindowSizeCallback(mainwindow, resizeGLScene);
	glfwSetMouseButtonCallback(mainwindow, onMouseButton);
	glfwSetScrollCallback(mainwindow, onMouseWheel);
	glfwSetKeyCallback(mainwindow, onKey);
	glfwSetCharCallback(mainwindow, onChar);

	// GLEW
	GLenum error = glewInit();
	if (error != GLEW_OK) {
		cerr << "Could not initialize GLEW." << endl;
		return false;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_Init(mainwindow, false, GlfwClientApi_Unknown);
	ImGui_ImplOpenGL3_Init("#version 330");

	return true;
}

void destroySDLWindow() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(mainwindow);
	glfwTerminate();
}

int main(int argc, char** argv) try {
	if (!createSDLWindow(initialWindowWidth, initialWindowHeight))
		return -1;

	if (!initGL())
		return -1;

	resizeGLScene(mainwindow, initialWindowWidth, initialWindowHeight);

	camera.position += 5;

	while (true) {
		glfwPollEvents();

		if (glfwGetKey(mainwindow, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwWindowShouldClose(mainwindow))
			break;

		timer.tick();
		update(timer.interval);
		render();
		glfwSwapBuffers(mainwindow);
	}

	destroySDLWindow();

	return 0;
} catch (const std::exception& e) {
	cerr << e.what() << endl;
	return -2;
} catch (...) {
	cerr << "Unknown exception" << endl;
	return -2;
}
