////
////  main.cpp
////  Check Max Supported OpenGL version
////
////  Created by CGIS on 30/09/2020.
////  Copyright © 2020 CGIS. All rights reserved.
////
//
//#define GLEW_STATIC
//
//#include <iostream>
//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//
//int glWindowWidth = 640;
//int glWindowHeight = 480;
//
//GLFWwindow* glWindow = NULL;
//
//bool initOpenGLWindow()
//{
//	if (!glfwInit()) {
//		fprintf(stderr, "ERROR: could not start GLFW3\n");
//		return false;
//	}
//
//	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "Hello Window", NULL, NULL);
//	if (!glWindow) {
//		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
//		glfwTerminate();
//		return false;
//	}
//
//	glfwMakeContextCurrent(glWindow);
//
//	// start GLEW extension handler
//	glewExperimental = GL_TRUE;
//	glewInit();
//
//	// get version info
//	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
//	const GLubyte* version = glGetString(GL_VERSION); // version as a string
//	printf("Renderer: %s\n", renderer);
//	printf("OpenGL version supported %s\n", version);
//
//	return true;
//}
//
//int main(int argc, const char* argv[]) {
//
//	initOpenGLWindow();
//
//	//close GL context and any other GLFW resources
//	glfwTerminate();
//
//	return 0;
//}
