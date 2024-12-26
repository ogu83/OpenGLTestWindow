#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

float rectX = 0.0f;
float rectY = -0.95f;
float rectWidth = 0.4f;
float rectHeight = 0.01f;
const float rectBaseSpeed = 0.01f;
const float rectSpeedMultiplier = 1.03;
float rectSpeed = rectBaseSpeed;

float circleX = 0.0f;
float circleY = -0.8f;
float circleRadius = 0.05f;
const float base_circleSpeed = 0.0002f;
const float circle_seg = 32;
float circleXSpeed = -1 * base_circleSpeed;
float circleYSpeed = base_circleSpeed;

const int numRows = 3;
const int numCols = 6;
const float brickWidth = 0.10f * 2.5f;
const float brickHeight = 0.16f * 2.5f;
const float brickLMargin = 0.06f;
const float brickTMargin = 0.02f;

const std::string Title = "OpenGLTestWindow";

int screen_w = 1300;
int screen_h = 800;

std::vector<int> brokenBricks;

//GLuint textureID; // Texture ID for the loaded image

struct rect {
	float left;
	float right;
	float top;
	float bottom;
};

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
		case GLFW_KEY_LEFT:
			if (rectX > -0.77f)
				rectX -= rectSpeed;
			rectSpeed *= rectSpeedMultiplier;
			break;
		case GLFW_KEY_RIGHT:
			if (rectX < 0.77f)
				rectX += rectSpeed;
			rectSpeed *= rectSpeedMultiplier;
			break;
		case GLFW_KEY_UP:
			//rectY += rectSpeed;
			break;
		case GLFW_KEY_DOWN:
			//rectY -= rectSpeed;
			break;
		}
	}
	else if (action == GLFW_RELEASE)
	{
		rectSpeed = rectBaseSpeed;
	}
}

bool contains(const std::vector<int>& vec, int value) {
	return std::find(vec.begin(), vec.end(), value) != vec.end();
}

bool isclamped(float mid, float A, float B)
{
	if (A > B)
	{
		return mid >= B && mid <= A;
	}
	return mid >= A && mid <= B;
}

bool checkCollisionOneWay(rect rectA, rect rectB)
{
	if ((isclamped(rectA.left, rectB.left, rectB.right)
		|| isclamped(rectA.right, rectB.left, rectB.right))
		&& (isclamped(rectA.bottom, rectB.bottom, rectB.top)
			|| isclamped(rectA.top, rectB.bottom, rectB.top)))
	{
		return true;
	}
	return false;
}

bool checkCollisionBothWays(rect rectA, rect rectB)
{
	return checkCollisionOneWay(rectA, rectB) || checkCollisionOneWay(rectB, rectA);
}

bool checkCollision(float circleX, float circleY, float circleRadius,
	float rectX, float rectY, float rectWidth, float rectHeight) {

	rect rect1;
	rect1.left = circleX - circleRadius;
	rect1.right = circleX + circleRadius;
	rect1.top = circleY - circleRadius;
	rect1.bottom = circleY + circleRadius;

	rect rect2;
	rect2.left = rectX - rectWidth / 2;
	rect2.right = rectX + rectWidth / 2;
	rect2.top = rectY - rectHeight / 2;
	rect2.bottom = rectY + rectHeight / 2;
	return checkCollisionBothWays(rect1, rect2);
}

bool checkCollisionBricks(float circleX, float circleY, float circleRadius) {
	for (int row = 0; row < numRows; ++row) {
		for (int col = 0; col < numCols; ++col) {
			if (contains(brokenBricks, row * 99 + col * 9))
				continue;

			float brickX = -0.8f + col * (brickWidth + brickLMargin);
			float brickY = 0.8f - row * (brickHeight + brickTMargin);
			//drawRectangle(brickX, brickY, brickWidth, brickHeight);
			bool retval = checkCollision(circleX, circleY, circleRadius, brickX, brickY, brickWidth, brickHeight);
			if (retval) {
				brokenBricks.push_back(row * 99 + col * 9);
				return retval;
			}
		}
	}
	return false;
}

GLuint loadTexture(const char* imagePath) {
	int width, height, channels;
	//stbi_set_flip_vertically_on_load(true); // Flip the image vertically to match OpenGL's coordinate system
	unsigned char* data = stbi_load(imagePath, &width, &height, &channels, 0);
	GLuint textureID = 0;

	if (data) {
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
	}
	else {
		fprintf(stderr, "Failed to load texture: %s\n", stbi_failure_reason());
	}

	return textureID;
}

void drawSpaceShip() {
	glBegin(GL_TRIANGLES);

	// Body of the spaceship
	glColor3f(0.5f, 0.5f, 0.5f);  // Gray color
	glVertex2f(0.0f, 0.0f);        // Center
	glVertex2f(-0.1f, -0.2f);      // Bottom-left
	glVertex2f(0.1f, -0.2f);       // Bottom-right

	// Left wing
	glColor3f(0.0f, 1.0f, 0.0f);  // Green color
	glVertex2f(-0.1f, 0.0f);       // Middle-left
	glVertex2f(-0.3f, 0.2f);       // Top-left
	glVertex2f(-0.1f, -0.2f);      // Bottom-left

	// Right wing
	glColor3f(0.0f, 1.0f, 0.0f);  // Green color
	glVertex2f(0.1f, 0.0f);        // Middle-right
	glVertex2f(0.3f, 0.2f);        // Top-right
	glVertex2f(0.1f, -0.2f);       // Bottom-right

	glEnd();
}


void drawCar() {
	glBegin(GL_QUADS);

	// Body of the car
	glColor3f(1.0f, 0.0f, 0.0f);  // Red color
	glVertex2f(-0.4f, -0.1f);      // Bottom-left
	glVertex2f(0.4f, -0.1f);       // Bottom-right
	glVertex2f(0.3f, 0.1f);        // Top-right
	glVertex2f(-0.3f, 0.1f);       // Top-left

	// Roof of the car
	glColor3f(0.0f, 0.0f, 1.0f);  // Blue color
	glVertex2f(-0.3f, 0.1f);       // Bottom-left
	glVertex2f(0.3f, 0.1f);        // Bottom-right
	glVertex2f(0.2f, 0.4f);        // Top-right
	glVertex2f(-0.2f, 0.4f);       // Top-left

	glEnd();
}

void drawBar() {
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.0f, 0.0f); // Set color to red
	glVertex2f(rectX - rectWidth / 2, rectY - rectHeight / 2);   // Bottom-left vertex
	glVertex2f(rectX + rectWidth / 2, rectY - rectHeight / 2);   // Bottom-right vertex
	glVertex2f(rectX + rectWidth / 2, rectY + rectHeight / 2);   // Top-right vertex
	glVertex2f(rectX - rectWidth / 2, rectY + rectHeight / 2);   // Top-left vertex
	glEnd();
}

void drawRectangle(float x, float y, float width, float height) {
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.0f, 0.0f); // Set color to red
	glVertex2f(x - width / 2, y - height / 2);   // Bottom-left vertex
	glVertex2f(x + width / 2, y - height / 2);   // Bottom-right vertex
	glVertex2f(x + width / 2, y + height / 2);   // Top-right vertex
	glVertex2f(x - width / 2, y + height / 2);   // Top-left vertex
	glEnd();
}

void drawBricks() {
	for (int row = 0; row < numRows; ++row) {
		for (int col = 0; col < numCols; ++col) {
			float brickX = -0.8f + col * (brickWidth + brickLMargin);
			float brickY = 0.8f - row * (brickHeight + brickTMargin);
			drawRectangle(brickX, brickY, brickWidth, brickHeight);
		}
	}
}

void drawGrid() {
	glColor3f(0.5f, 0.5f, 0.5f);  // Gray color

	glBegin(GL_LINES);

	// Draw horizontal lines
	for (float y = -1.0f; y <= 1.0f; y += 0.2f) {
		glVertex2f(-1.0f, y);
		glVertex2f(1.0f, y);
	}

	// Draw vertical lines
	for (float x = -1.0f; x <= 1.0f; x += 0.2f) {
		glVertex2f(x, -1.0f);
		glVertex2f(x, 1.0f);
	}

	glEnd();
}

void drawImageOnBricks(std::vector<GLuint> textureIDs) {
	int tId = 0;
	glColor3f(1.0f, 1.0f, 1.0f); // Set color to white

	for (int row = 0; row < numRows; ++row) {
		for (int col = 0; col < numCols; ++col) {

			GLuint textureID = textureIDs[tId];
			tId++;
			if (tId > (textureIDs.size() - 1))
				tId = 0;

			if (contains(brokenBricks, row * 99 + col * 9))
				continue;

			float brickX = -0.8f + col * (brickWidth + brickLMargin);
			float brickY = 0.8f - row * (brickHeight + brickTMargin);

			glBindTexture(GL_TEXTURE_2D, textureID);
			
			glBegin(GL_QUADS);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(brickX - brickWidth / 2, brickY - brickHeight / 2);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(brickX + brickWidth / 2, brickY - brickHeight / 2);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(brickX + brickWidth / 2, brickY + brickHeight / 2);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(brickX - brickWidth / 2, brickY + brickHeight / 2);
			glEnd();
		
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}

void drawCircle(float cx, float cy, float r, int num_segments) {
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(0.0f, 0.0f, 1.0f); // Set color to blue
	glVertex2f(cx, cy); // Center of the circle

	for (int i = 0; i <= num_segments; ++i) {
		float theta = static_cast<float>(2.0 * M_PI * i / num_segments);
		float x = r * cos(theta);
		float y = r * sin(theta);
		glVertex2f(cx + x, cy + y);
	}

	glEnd();
}

int main() {
	if (!glfwInit()) {
		return -1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	GLFWwindow* window = glfwCreateWindow(screen_w, screen_h, Title.c_str(), nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyCallback);

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		glfwTerminate();
		return -1;
	}

	glEnable(GL_TEXTURE_2D); // Enable 2D texturing

	//GLuint textureId = loadTexture("./Images/IMG_1052.HEIC");

	std::vector<std::string> filenames;
	std::vector<GLuint> textures;
	const std::string folderPath = "./Images/";

	for (const auto& entry : fs::directory_iterator(folderPath)) {
		filenames.push_back(folderPath + entry.path().filename().string());
		textures.push_back(loadTexture((folderPath + entry.path().filename().string()).c_str()));
	}
	int tId = 0;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set the background color to black

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);

		//circleX += circleXSpeed;
		//circleY += circleYSpeed;

		//// Check for collision with rectangle
		//if (checkCollision(circleX, circleY, circleRadius, rectX, rectY, rectWidth, rectHeight)) {
		//	circleYSpeed *= -1; // Invert Y speed for bounce
		//}

		////Check for collision with bricks
		//if (checkCollisionBricks(circleX, circleY, circleRadius)) { 
		//	circleYSpeed *= -1;
		//	circleX += circleXSpeed;
		//	circleY += circleYSpeed;
		//}
		//
		//if (checkCollisionBricks(circleX, circleY, circleRadius)) {
		//	circleXSpeed *= -1;
		//	circleYSpeed *= -1;
		//}

		//// Bounce when reaching screen boundaries
		//if (circleX - circleRadius <= -1.0f || circleX + circleRadius >= 1.0f) {
		//	circleXSpeed *= -1;
		//}

		//if (circleY + circleRadius >= 1.0f) {
		//	circleYSpeed *= -1;
		//}

		//if (circleY - circleRadius <= -1.0f) {
		//	//TODO: Game Over
		//}

		//// Draw bricks
		////drawBricks();
		//drawImageOnBricks(textures);

		////Draw ball
		//drawCircle(circleX, circleY, circleRadius, circle_seg);

		//// Draw the movable 2D bar as rectangle
		//drawBar();

		//drawSpaceShip();
		//drawCar();
		drawGrid();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
