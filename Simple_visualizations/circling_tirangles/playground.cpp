// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <windows.h>
#include <cmath>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Tutorial 02 - Red triangle", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint FirstProgramID = LoadShaders("SimpleVertexShader.vertexshader", "FirstFragmentShader.fragmentshader");
	GLuint SecondProgramID = LoadShaders("SimpleVertexShader.vertexshader", "SecondFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixIDfirst = glGetUniformLocation(FirstProgramID, "MVP");
	GLuint MatrixIDsecond = glGetUniformLocation(SecondProgramID, "MVP");


	//new
	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around
	//end_new


	static const GLfloat first_triangle_vertexes[] = {
		0.9f, -0.2f, 0.3f,
		 0.4f, 0.3f, -0.8f,
		 -0.6f,  0.2f, 0.7f,
	};

	static const GLfloat second_triangle_vertexes[] = {
		0.3f, -0.7f, 0.1f,
		 -0.9f, 0.9f, 0.9f,
		 0.6f,  -0.2f, -0.7f,
	};

	GLuint triangle_buffers[2];
	glGenBuffers(2, triangle_buffers);

	glBindBuffer(GL_ARRAY_BUFFER, triangle_buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(first_triangle_vertexes), first_triangle_vertexes, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, triangle_buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(second_triangle_vertexes), second_triangle_vertexes, GL_STATIC_DRAW);

	float x = 4.0;
	float y = 0.0;
	float move_x = 0.1;
	float mult_y = 1;
	float eps = 0.001;

	do {
		Sleep(50);
		std::cout << x << ' ' << y << '\n';
		glm::mat4 View = glm::lookAt(
			glm::vec3(x, y, 3), // Camera is at (4,3,3), in World Space
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);
		glm::mat4 MVP = Projection * View * Model;
		x = x - move_x;
		y = sqrt(16 - x * x) * mult_y;
		if (x < -4.0 || x > 4.0)
		{
			move_x *= (-1);
			y = 0;
			mult_y *= (-1);
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		

		// Use our shader
		glUseProgram(FirstProgramID);

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixIDfirst, 1, GL_FALSE, &MVP[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, triangle_buffers[0]);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		//Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle
				
		glUseProgram(SecondProgramID);

		glUniformMatrix4fv(MatrixIDsecond, 1, GL_FALSE, &MVP[0][0]);

		glBindBuffer(GL_ARRAY_BUFFER, triangle_buffers[1]);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle

		glDisableVertexAttribArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO
	glDeleteBuffers(2, triangle_buffers);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(FirstProgramID);
	glDeleteProgram(SecondProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

