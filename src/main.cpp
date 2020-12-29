/*  by Alun Evans 2016 LaSalle (aevanss@salle.url.edu)
	edited by Conrado Ruiz 2020 LaSalle (conrado.ruiz@salle.url.edu) */

	//include some standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

//include OpenGL and GLM libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

//include some custom code files
#include "glfunctions.h" //include all OpenGL stuff
#include "Shader.h" // class to compile shaders
#include <string>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "imageloader.h"

using namespace std;
using namespace glm;

string basepath = "assets/";
string inputfile = basepath + "sphere.obj";
vector< tinyobj::shape_t > shapes;
vector< tinyobj::material_t > materials;

/*string inputfile2 = basepath + "bunny.obj";
vector< tinyobj::shape_t > shapes2;
vector< tinyobj::material_t > materials2;*/


//global variables to help us do things
int g_ViewportWidth = 720; int g_ViewportHeight = 720; // Default window size, in pixels
double mouse_x, mouse_y; //variables storing mouse position
const vec3 g_backgroundColor(0.2f, 0.2f, 0.2f); // background colour - a GLM 3-component vector

GLuint g_simpleShader = 0; //shader identifier
GLuint g_Vao = 0; //vao
//GLuint g_Vao2 = 0; //vao
GLuint g_NumTriangles = 0; //  Numbre of triangles we are painting.
//GLuint g_NumTriangles2 = 0; //  Numbre of triangles we are painting.


float deltaTime = 0.0f;
float lastTime = 0.0f;

float last_mouse_x = 0.0f;
float last_mouse_y = 0.0f;

bool is_down = false;
vec3 center = vec3(0.0f, 0.0f, 0.0f);
vec3 eye = vec3(0.0f, 1.0f, 1.0f);

float cam_yaw = 0.0f;
float cam_pitch = 0.0f;

//you may need to change these variables, depending on your system
float MOVE_SPEED = 0.00015f;
float LOOK_SPEED = 0.0005f;

//global variables used for camera movement
int key_flags[] = { 0, 0, 0, 0 }; //w, a, s, d

//global variables for texture
GLuint texture_id;

// ------------------------------------------------------------------------------------------
// This function load the meshes to the memory and binds to the VAO
// ------------------------------------------------------------------------------------------
void load()
{
	// Load the mesh - TEAPOT
	string err;
	bool ret = tinyobj::LoadObj(shapes, materials, err, inputfile.c_str(), basepath.c_str());

	//check for errors
	cout << "# of shapes mesh 1: " << shapes.size() << endl;
	if (!err.empty()) std::cerr << err << std::endl;

	// Load the mesh - BUNNY
	/*bool ret2 = tinyobj::LoadObj(shapes2, materials2, err, inputfile2.c_str(), basepath.c_str());

	//check for errors
	cout << "# of shapes mesh 2: " << shapes2.size() << endl;
	if (!err.empty()) std::cerr << err << std::endl;*/

	//load the shader
	Shader simpleShader("src/shader.vert", "src/shader.frag");
	g_simpleShader = simpleShader.program;

	// Create the VAO where we store all geometry (stored in g_Vao)
	g_Vao = gl_createAndBindVAO();

	//create vertex buffer for positions, colors, and indices, and bind them to shader
	gl_createAndBindAttribute(&(shapes[0].mesh.positions[0]),
		shapes[0].mesh.positions.size() * sizeof(float), g_simpleShader, "a_vertex", 3);
	gl_createIndexBuffer(&(shapes[0].mesh.indices[0]),
		shapes[0].mesh.indices.size() * sizeof(unsigned int));

	gl_createAndBindAttribute(
		&(shapes[0].mesh.texcoords[0]),
		shapes[0].mesh.texcoords.size() * sizeof(GLfloat),
		g_simpleShader,
		"a_uv", 2);


	gl_unbindVAO();
	g_NumTriangles = shapes[0].mesh.indices.size() / 3;


	// Create the VAO where we store all geometry (stored in g_Vao)
	/*g_Vao2 = gl_createAndBindVAO();

	//create vertex buffer for positions, colors, and indices, and bind them to shader
	gl_createAndBindAttribute(&(shapes2[0].mesh.positions[0]),
		shapes2[0].mesh.positions.size() * sizeof(float), g_simpleShader, "a_vertex", 3);
	gl_createIndexBuffer(&(shapes2[0].mesh.indices[0]),
		shapes2[0].mesh.indices.size() * sizeof(unsigned int));

	gl_unbindVAO();
	g_NumTriangles2 = shapes2[0].mesh.indices.size() / 3;*/

	Image* image = loadBMP("assets/earthmap.bmp");
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, // target
		0, // level = 0 base, no mipmap
		GL_RGB, // how the data will be stored
		image->width, // width of the image
		image->height, // height of the image
		0, //border
		GL_RGB, // format of original data
		GL_UNSIGNED_BYTE, // type of data
		image->pixels); // pointer to the start of data


}

// ------------------------------------------------------------------------------------------
// This function actually draws to screen and called non-stop, in a loop
// ------------------------------------------------------------------------------------------
void draw()
{
	//clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST),
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// activate shader
	glUseProgram(g_simpleShader);

	//bind the geometry
	gl_bindVAO(g_Vao);

	GLuint colorLoc = glGetUniformLocation(g_simpleShader, "u_color");
	glUniform3f(colorLoc, 0.0, 1.0, 0.0);


	// PROJECTION MATRIX
	GLuint projection_loc = glGetUniformLocation(g_simpleShader, "u_projection");
	mat4 projection_matrix = perspective(
		90.0f, // Field of view
		1.0f,  // Aspect ratio
		0.1f,  // near plane (distance from camera)
		50.0f  // Far plane (distance from camera)
	);
	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));


	// VIEW MATRIX
	vec3 up = vec3(0.0f, 1.0f, 0.0f);
	mat4 view_matrix = glm::lookAt(
		eye, 		// the position of your camera, in world space
		center,   	// where you want to look at, in world space
		up       	// probably glm::vec3(0,1,0)
	);
	GLuint view_loc = glGetUniformLocation(g_simpleShader, "u_view");
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));

	// MODEL MATRIX 1 - TEAPOT
	GLuint model_loc = glGetUniformLocation(g_simpleShader, "u_model");
	mat4 T = translate(mat4(1.0f), vec3(0.0, 0.0, -2.0));
	mat4 R = rotate(mat4(1.0f), 45.0f, vec3(0.0, 1.0, 0.0));
	mat4 S = scale(mat4(1.0f), vec3(1.0, 1.0, 1.0));
	mat4 model = T * R * S;
	glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));

	//find the sample uniform in the shader
	GLuint u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
	// bind the sampler to the texture unit 0
	glUniform1i(u_texture, 0);
	// activate texture unit 0 and bin the texture object
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	// Draw to screen - TEAPOT
	glDrawElements(GL_TRIANGLES, 3 * g_NumTriangles, GL_UNSIGNED_INT, 0);
	gl_unbindVAO();

	// MODEL MATRIX 2 - BUNNY
	/*mat4 T2 = translate(mat4(1.0f), vec3(-0.5, -0.5, -1.5));
	mat4 R2 = rotate(mat4(1.0f), 0.0f, vec3(0.0, 0.0, 1.0));
	mat4 S2 = scale(mat4(1.0f), vec3(6.0, 6.0, 6.0));
	mat4 model2 = T2 * R2 * S2;

	glUniform3f(colorLoc, 1.0, 0.0, 0.0);
	glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model2));

	// Draw to screen - BUNNY
	gl_bindVAO(g_Vao2);
	glDrawElements(GL_TRIANGLES, 3 * g_NumTriangles2, GL_UNSIGNED_INT, 0);
	gl_unbindVAO();*/

}

// ------------------------------------------------------------------------------------------
// This function is called every time you press a screen
// ------------------------------------------------------------------------------------------
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (action == GLFW_PRESS)
		switch (key)
		{
		case GLFW_KEY_W:
			cout << "Pressed up\n";
			key_flags[0] = 1;
			break;
		case GLFW_KEY_A:
			cout << "Pressed left\n";
			key_flags[1] = 1;
			break;
		case GLFW_KEY_S:
			cout << "Pressed down\n";
			key_flags[2] = 1;
			break;
		case GLFW_KEY_D:
			cout << "Pressed right\n";
			key_flags[3] = 1;
			break;
		case GLFW_KEY_R:
			eye = vec3(0.0f, 1.0f, 1.0f);
			cam_yaw = 0.0f;
			cam_pitch = 0.0f;
			break;
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, 1);
			break;
		}
	else if (action == GLFW_RELEASE)

		switch (key)
		{
		case GLFW_KEY_W:
			cout << "Released up\n";
			key_flags[0] = 0;
			break;
		case GLFW_KEY_A:
			cout << "Released left\n";
			key_flags[1] = 0;
			break;
		case GLFW_KEY_S:
			cout << "Released down\n";
			key_flags[2] = 0;
			break;
		case GLFW_KEY_D:
			cout << "Released right\n";
			key_flags[3] = 0;
			break;
		}

}

// ------------------------------------------------------------------------------------------
// This function is called every time you click the mouse
// ------------------------------------------------------------------------------------------
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		is_down = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		is_down = false;
	}

}

void onMouseMove(GLFWwindow* window, double x, double y) {
	mat4 tr = translate(mat4(1.0), eye);

	if (is_down) {
		cam_yaw += (last_mouse_x - x) * LOOK_SPEED * deltaTime;
		cam_pitch += (last_mouse_y - y) * LOOK_SPEED * deltaTime;                                // no z
	}

	last_mouse_x = x;
	last_mouse_y = y;

}


// --------------------------------------------------------------
// rotate camera according to pitch and yaw,
// move camera according to key flags
// --------------------------------------------------------------
void update() {

	//reset camera to look down z-axis
	vec3 initial_look_vector = vec3(0, 0, -1);
	//rotate look vector around y_axis (yaw)
	vec3 first_rotation = rotate(initial_look_vector, cam_yaw * 57.19f, vec3(0, 1, 0));
	//rotate x-axis by same amount - this avoids gimbal lock
	vec3 intermediate_axis = rotate(vec3(1, 0, 0), cam_yaw * 57.19f, vec3(0, 1, 0));
	//now rotate pitch around intermediate axis
	vec3 final_look_vector = rotate(first_rotation, cam_pitch * 57.19f, intermediate_axis);

	//set camera target to be position + our new look vector
	center = eye + final_look_vector;

	// use time to control the speed consistently across machines
	deltaTime = (glfwGetTime() - lastTime) * 10000;
	lastTime = glfwGetTime();

	//get forward and side vectors for movement
	vec3 forward = normalize(center - eye);
	vec3 side = cross(vec3(0, 1, 0), forward);

	// WASD = forward / back / strafe left / strafe right
	if (key_flags[0]) { // W
		eye = eye + forward * MOVE_SPEED * deltaTime;
		center = center + forward * MOVE_SPEED * deltaTime;
	}
	if (key_flags[1]) { // A
		eye = eye + side * MOVE_SPEED * deltaTime;
		center = center + side * MOVE_SPEED * deltaTime;
	}
	if (key_flags[2]) { // S
		eye = eye - forward * MOVE_SPEED * deltaTime;
		center = center - forward * MOVE_SPEED * deltaTime;
	}
	if (key_flags[3]) { // D
		eye = eye - side * MOVE_SPEED * deltaTime;
		center = center - side * MOVE_SPEED * deltaTime;
	}
}

int main(void)
{
	//setup window and boring stuff, defined in glfunctions.cpp
	GLFWwindow* window;
	if (!glfwInit())return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(g_ViewportWidth, g_ViewportHeight, "Hello OpenGL!", NULL, NULL);
	if (!window) { glfwTerminate();	return -1; }
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();

	//input callbacks
	glfwSetCursorPosCallback(window, onMouseMove);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

	//load all the resources
	load();

	// Loop until the user closes the window
	while (!glfwWindowShouldClose(window))
	{
		draw();

		// update the camera based on the yaw and pitch 
		update();

		// Swap front and back buffers
		glfwSwapBuffers(window);

		// Poll for and process events
		glfwPollEvents();

		//mouse position must be tracked constantly (callbacks do not give accurate delta)
		glfwGetCursorPos(window, &mouse_x, &mouse_y);
	}

	//terminate glfw and exit
	glfwTerminate();
	return 0;
}


