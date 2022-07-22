//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <windows.h>
#include <mmsystem.h>
#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

//skybox
std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

int glWindowWidth = 1920;
int glWindowHeight = 1080;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 4096;
const unsigned int SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(
	glm::vec3(-4.42f, 7.78f, 10.99f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.09f;

bool pressedKeys[1024];
GLfloat angle;
GLfloat lightAngle;

//obiectele din scena
gps::Model3D lightCube;
gps::Model3D fullScene;
gps::Model3D fireball1;
gps::Model3D fireball2;
gps::Model3D ninja1;
gps::Model3D ninja2;
gps::Model3D lightningBall;
gps::Model3D lightningBall1;
gps::Model3D pig;

//shaderele
gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;
bool showDepthMap;
bool play;

// rotate camera
bool cameraRotate = false;
float angleCamera = 0.0f;

// camera animation (preview)
bool startPreview = false;
float previewAngle;

float i[7];
float j[7];
float x[7];

//rotate ball
GLfloat ballRotation = 0.0f;

//rotate planet
GLfloat planetRotation = 0.0f;


//move first ninja
float move1;
float move2;
float move3;
glm::vec3 ninja1Position;

//move second ninja
float moven1;
float moven2;
float moven3;
glm::vec3 ninja2Position;

// fog
int foginit = 0;
GLint foginitLoc;
GLfloat fogDensity = 0.007f;

// lumina punctiforma
int pointinit = 0;
glm::vec3 lightPos1; // on hurt ninja
GLuint lightPos1Loc;
glm::vec3 lightPos2; // on kakashi sensei
GLuint lightPos2Loc;

bool mouse = true;

float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch;

GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO	
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	// set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void initFaces()
{
	faces.push_back("textures/skybox/right.png");
	faces.push_back("textures/skybox/left.png");
	faces.push_back("textures/skybox/top.png");
	faces.push_back("textures/skybox/bottom.png");
	faces.push_back("textures/skybox/back.png");
	faces.push_back("textures/skybox/front.png");
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
		if (mouse)
		{
			lastX = xpos;
			lastY = ypos;
			mouse = false;
		}
	
		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.05f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		myCamera.rotate(pitch, yaw);

}

void processMovement()
{
	// rotire camera la stanga
	if (pressedKeys[GLFW_KEY_E]) {
		angle += 0.5f;
		if (angle > 360.0f)
			angle -= 360.0f;
	}
	// rotire camera la dreapta
	if (pressedKeys[GLFW_KEY_Q]) {
		angle -= 0.5f;
		if (angle < 0.0f)
			angle += 360.0f;
	}
	//schimba directia luminii spre stanga
	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;
	}
	//schimba directia luminii spre dreapta
	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}
	//miscare camera in fata
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}
	//miscare camera in spate
	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}
	//miscare camera la stanga
	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}
	//miscare camera la dreapta
	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);

	}
	// printeaza coordonatele camerei
	if (pressedKeys[GLFW_KEY_I]) {
		printf("x = %f   y = %f   z = %f \n", myCamera.cameraPosition.x, myCamera.cameraPosition.y, myCamera.cameraPosition.z);
		printf("x = %f   y = %f   z = %f \n", myCamera.cameraFrontDirection.x, myCamera.cameraFrontDirection.y, myCamera.cameraFrontDirection.z);
	}
	// start ceata
	if (pressedKeys[GLFW_KEY_F]) {

		myCustomShader.useShaderProgram();
		foginit = 1;
		foginitLoc = glGetUniformLocation(myCustomShader.shaderProgram, "foginit");
		glUniform1i(foginitLoc, foginit);
	}
	// stop ceata
	if (pressedKeys[GLFW_KEY_G]) {
		myCustomShader.useShaderProgram();
		foginit = 0;
		foginitLoc = glGetUniformLocation(myCustomShader.shaderProgram, "foginit");
		glUniform1i(foginitLoc, foginit);
	}
	//mareste intensitatea cetei
	if (pressedKeys[GLFW_KEY_F] && pressedKeys[GLFW_KEY_EQUAL])
	{
		fogDensity = glm::min(fogDensity + 0.0001f, 1.0f);
	}
	// micsoreaza intensitatea cetei
	if (pressedKeys[GLFW_KEY_F] && pressedKeys[GLFW_KEY_MINUS])
	{
		fogDensity = glm::max(fogDensity - 0.0001f, 0.0f);
	}
	// Miscari ninja1
	if (pressedKeys[GLFW_KEY_O]) {
		if (move1 < 100)
			move1 += 0.2;
	}
	if (pressedKeys[GLFW_KEY_P]) {
		if (move1 < 100)
			move1 -= 0.2;
	}
	if (pressedKeys[GLFW_KEY_LEFT]) {
		if (move2 < 100)
			move2 -= 0.2;
	}
	if (pressedKeys[GLFW_KEY_RIGHT]) {
		if (move2 < 100)
			move2 += 0.2;
	}
	if (pressedKeys[GLFW_KEY_UP]) {
		if (move3 < 100)
			move3 += 0.2;
	}
	if (pressedKeys[GLFW_KEY_DOWN]) {
		if (move3 < 100)
			move3 -= 0.2;
	}
	// Miscare ninja 2
	if (pressedKeys[GLFW_KEY_KP_5]) {
		if (moven1 < 100)
			moven1 += 0.2;
	}
	if (pressedKeys[GLFW_KEY_KP_0]) {
		if (moven1 < 100)
			moven1 -= 0.2;
	}
	if (pressedKeys[GLFW_KEY_KP_4]) {
		if (moven2 < 100)
			moven2 -= 0.2;
	}
	if (pressedKeys[GLFW_KEY_KP_6]) {
		if (moven2 < 100)
			moven2 += 0.2;
	}
	if (pressedKeys[GLFW_KEY_KP_8]) {
		if (moven3 < 100)
			moven3 -= 0.2;
	}
	if (pressedKeys[GLFW_KEY_KP_2]) {
		if (moven3 < 100)
			moven3 += 0.2;
	}
	// start Lumina punctiforma
	if (pressedKeys[GLFW_KEY_9]) {
		myCustomShader.useShaderProgram();
		pointinit = 1;
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointinit"), pointinit);
	}
	// stop lumina punctiforma
	if (pressedKeys[GLFW_KEY_0]) {
		myCustomShader.useShaderProgram();
		pointinit = 0;
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointinit"), pointinit);
	}
	// start preview
	if (pressedKeys[GLFW_KEY_Z]) {
		PlaySound(TEXT("Naruto.wav"), NULL, SND_ASYNC | SND_FILENAME);
		startPreview = true;
	}
	// stop preview
	if (pressedKeys[GLFW_KEY_X]) {
		PlaySound(NULL, NULL, SND_ASYNC);
		startPreview = false;
		play = false;
		move3 = 0;
		moven3 = 0;
	}
	// line view
	if (pressedKeys[GLFW_KEY_1]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	// point view
	if (pressedKeys[GLFW_KEY_2]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
	// normal view
	if (pressedKeys[GLFW_KEY_3]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "Naruto vs Pain scene", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_BLEND);
}

void initObjects() {
	fullScene.LoadModel("objects/fullScene/narutoScene.obj");
	lightCube.LoadModel("objects/cube/cube.obj");
	ninja1.LoadModel("objects/BreezeDancer/BreezeDancer.obj");
	ninja2.LoadModel("objects/MidnightBlade/MidnightBlade.obj");
	lightningBall.LoadModel("objects/Ball/chidori.obj");
	lightningBall1.LoadModel("objects/Ball/chidori1.obj");
	fireball1.LoadModel("objects/Ball/ball1.obj");
	fireball2.LoadModel("objects/Ball/fire1.obj");
	pig.LoadModel("objects/Tonton/Tonton.obj");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
	depthMapShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	lightDirMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDirMatrix");

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(-16.0f, 16.0f, 9.65f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(0.7f, 0.7f, 0.7f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	// pointlight
	lightPos1 = glm::vec3(-4.46f, 10.18f, -29.70); // at hurt ninja
	lightPos1Loc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPos1");
	glUniform3fv(lightPos1Loc, 1, glm::value_ptr(lightPos1));

	lightPos2 = glm::vec3(10.05f, 10.79f, 27.67f); // at kakashi
	lightPos2Loc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPos2");
	glUniform3fv(lightPos2Loc, 1, glm::value_ptr(lightPos2));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	//bind nothing to attachment points
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	//unbind until ready to use
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix
	glm::mat4 lightView = glm::lookAt(glm::inverseTranspose(glm::mat3(lightRotation)) * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 0.1f, far_plane = 100.0f;
	glm::mat4 lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}

void collisionDetect(glm::vec3 ninja1, glm::vec3 ninja2) {
	if (ninja1.x >= ninja2.x - 1 && ninja1.x <= ninja2.x + 1 && ninja1.y >= ninja2.y - 1 && ninja1.y <= ninja2.y + 1 && ninja1.z >= ninja2.z - 1 && ninja1.z <= ninja2.z + 1){
		move3 -= 3.7f;
		moven3 += 3.7f;
		mciSendString(TEXT("play swordSound.wav"), NULL, 0, NULL);
		play = true;
	}
}

void previewFunction() {
	if (startPreview) {
		previewAngle += 0.07f;
		move3 += 0.05;
		moven3 -= 0.05;
		if (play)
		{
			move3 = 0;
			moven3 = 0;
		}
		myCamera.scenePreview(previewAngle);
	}
}

void drawObjects(gps::Shader shader, bool depthPass) {

	shader.useShaderProgram();
	// draw the scene
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	fullScene.Draw(shader);
	
	// draw ninja number 1
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(35.93f + move2, 9.73f + move1, -15.70f + move3));
	ninja1Position = glm::vec3(35.93f + move2, 9.73f + move1, -15.70f + move3);
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	ninja1.Draw(shader); 

	// draw ninja number 2
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(35.93f + moven2, 8.89f + moven1, 10.68f + moven3));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	ninja2Position = glm::vec3(35.93f + moven2, 8.89f + moven1, 10.68f + moven3);
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	ninja2.Draw(shader);

	// draw the lightningBall and its rings
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(10.54f, 8.97f, 27.66f));
	model = glm::scale(model, glm::vec3(1.3f));
	model = glm::rotate(model, ballRotation, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	lightningBall.Draw(shader);

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(10.54f, 8.97f, 27.66f));
	model = glm::scale(model, glm::vec3(12.3f));
	model = glm::rotate(model, ballRotation, glm::vec3(0, -1, 1));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	lightningBall1.Draw(shader);

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(10.54f, 8.97f, 27.66f));
	model = glm::scale(model, glm::vec3(12.3f));
	model = glm::rotate(model, ballRotation, glm::vec3(0, 1, -1));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	lightningBall1.Draw(shader);

	// draw the rock
	model = glm::rotate(glm::mat4(2.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-2.60f, 8.00f, 6.1f));
	model = glm::rotate(model, planetRotation, glm::vec3(0, -1, 0));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	fireball1.Draw(shader);
	
	//draw the fire
	for (int n = 0; n <= 6; n++) {
		model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(-2.60f, 8.00f, 6.1f));
		model = glm::rotate(model, planetRotation, glm::vec3(i[n], j[n], x[n]));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}
		fireball2.Draw(shader);
	}

	//draw the pig
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-5.06f, 9.9f, -34.22f));
	model = glm::scale(model, glm::vec3(0.7f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	pig.Draw(shader);

}

void renderScene() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// depth maps creation pass
	
	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	//render scene = draw objects
	drawObjects(depthMapShader, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render depth map on screen - toggled with the M key

	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);
		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity"), fogDensity);
		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);
		collisionDetect(ninja1Position, ninja2Position);
		ballRotation += 0.05f;
		planetRotation += 0.01f;

		//draw a white cube around the light

		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		
		model = lightRotation;
		model = glm::translate(model, lightDir);
		model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.8f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		lightCube.Draw(lightShader);

		mySkyBox.Draw(skyboxShader, view, projection);


	}
}

void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

void initSkyBoxShader()
{
	mySkyBox.Load(faces);
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
		glm::value_ptr(projection));
}

int main(int argc, const char* argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}
	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	initFaces();
	initSkyBoxShader();

	for (int n = 0; n <= 6; n++) {
		i[n] = -1.0f + static_cast <int> (rand()) / (static_cast <int> (RAND_MAX / (1.0f - (-1.0f))));
		j[n] = -1.0f + static_cast <int> (rand()) / (static_cast <int> (RAND_MAX / (1.0f - (-1.0f))));
		x[n] = -1.0f + static_cast <int> (rand()) / (static_cast <int> (RAND_MAX / (1.0f - (-1.0f))));
	}
	glCheckError();


	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		previewFunction();
		renderScene();
		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}
	cleanup();

	return 0;
}
