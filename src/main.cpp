/*  by Alun Evans 2016 LaSalle (aevanss@salle.url.edu)
	edited by Conrado Ruiz 2020 LaSalle (conrado.ruiz@salle.url.edu) 
	edited by Eric Macià 2020 LaSalle (eric.macia@students.salle.url.edu)
			  Guillermo Sabaté 2020 LaSalle (guillermo.sabate@students.salle.edu)
*/

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

enum {
	SUN, MERCURY, VENUS, MOON, MARS, JUPITER, SATURN, URANUS, NEPTUNE
};

vector<float> distances = {0, 0, 1.5, 1, 6, 13, 24, 49, 76 };
vector<float> scales = {20, 1.2, 1.8, 1.2, 1.6, 11, 9, 4, 3.5};
vector<char*> textures = {"assets/sunmap.bmp", "assets/mercurymap.bmp", "assets/venusmap.bmp", "assets/moonmap.bmp", "assets/marsmap.bmp", "assets/jupitermap.bmp", "assets/saturnmap.bmp", "assets/uranusmap.bmp", "assets/neptunemap.bmp"};

#define NUM_PLANETS 9

struct planet {
	GLuint texture_id;
	vec3 position;
	vec3 scale;
	float orbit_angle;
	float rotation;
	float orbit_speed;
	float pivot_distance;
};

vector<planet> planets;

vec3 earthPosition(0.8 * 4, 0, 0);
float earth_angle = 0.0f;
float earth_rotation = 0.0f;
float clouds_rotation = 0.0f;

string basepath = "assets/";
string inputfile = basepath + "sphere.obj";

vector< tinyobj::shape_t > shapes;
vector< tinyobj::material_t > materials;

vector< tinyobj::shape_t > skyboxShapes;
vector< tinyobj::material_t > skyboxMaterials;

vector< tinyobj::shape_t > earthShapes;
vector< tinyobj::material_t > earthMaterials;


//global variables to help us do things
int g_ViewportWidth = 720; int g_ViewportHeight = 720; // Default window size, in pixels
double mouse_x, mouse_y; //variables storing mouse position
const vec3 g_backgroundColor(0.2f, 0.2f, 0.2f); // background colour - a GLM 3-component vector

GLuint g_planetShader = 0; //shader identifier
GLuint g_skyboxShader = 0; //skybox shader identifier
GLuint g_earthShader = 0; //earth shader identifier
GLuint g_sunShader = 0; //sun shader identifier
GLuint g_cloudsShader = 0; // clouds shader identifier

GLuint g_Vao = 0; //vao
GLuint g_Vao_skybox = 0; //vao for skybox
GLuint g_Vao_earth = 0; //vao for earth

GLuint g_NumTriangles = 0; //  Numbre of triangles we are painting.
GLuint g_NumTriangles_skybox = 0; //  Numbre of triangles we are painting.
GLuint g_NumTriangles_earth = 0; //Number of triangles we are painting.

float deltaTime = 0.0f;
float lastTime = 0.0f;

float last_mouse_x = 0.0f;
float last_mouse_y = 0.0f;

bool is_down = false;
vec3 center = vec3(0.0f, 0.0f, 0.0f);
vec3 eye = vec3(0.0f, 4.0f, 20.0f);

float cam_yaw = 0.0f;
float cam_pitch = 0.0f;

//you may need to change these variables, depending on your system
float MOVE_SPEED = 0.00045f;
float LOOK_SPEED = 0.0005f;

//global variables used for camera movement
int key_flags[] = { 0, 0, 0, 0 }; //w, a, s, d

//global variables for texture
GLuint texture_id;
GLuint texture_id_skybox;
GLuint texture_id_earth;
GLuint texture_id_earth_normal;
GLuint texture_id_earth_spec;
GLuint texture_id_earth_night;
GLuint texture_id_clouds;


//global light source vector
vec3 g_light_dir(0.0f, 0.0f, 0.0f);
float g_light_distance = 10.0f;
float g_light_angle = 45.0f;


void loadSkybox() {
	// Load the mesh - SKYBOX
	string err;
	bool ret = tinyobj::LoadObj(skyboxShapes, skyboxMaterials, err, inputfile.c_str(), basepath.c_str());

	//check for errors
	cout << "# of shapes mesh 1: " << skyboxShapes.size() << endl;
	if (!err.empty()) std::cerr << err << std::endl;
	
	Shader skyboxShader("src/shader_skybox.vert", "src/shader_skybox.frag");
	g_skyboxShader = skyboxShader.program;

	g_Vao_skybox = gl_createAndBindVAO();
	gl_createAndBindAttribute(&(skyboxShapes[0].mesh.positions[0]),
		skyboxShapes[0].mesh.positions.size() * sizeof(float),
		g_skyboxShader,
		"a_vertex",
		3);

	gl_createIndexBuffer(&(skyboxShapes[0].mesh.indices[0]),
		skyboxShapes[0].mesh.indices.size() * sizeof(unsigned int));

	gl_createAndBindAttribute(&(skyboxShapes[0].mesh.texcoords[0]),
		skyboxShapes[0].mesh.texcoords.size() * sizeof(GLfloat),
		g_skyboxShader,
		"a_uv",
		2);

	gl_unbindVAO();
	g_NumTriangles_skybox = skyboxShapes[0].mesh.indices.size() / 3;

	Image* image = loadBMP("assets/milkyway.bmp");

	glGenTextures(1, &(texture_id_skybox));
	glBindTexture(GL_TEXTURE_2D, texture_id_skybox);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGB,
		image->width,
		image->height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		image->pixels);
}

void loadEarth() {
	// Load the mesh - SKYBOX
	string err;
	bool ret = tinyobj::LoadObj(earthShapes, earthMaterials, err, inputfile.c_str(), basepath.c_str());

	//check for errors
	cout << "# of shapes mesh 1: " << earthShapes.size() << endl;
	if (!err.empty()) std::cerr << err << std::endl;

	Shader earthShader("src/shader_earth.vert", "src/shader_earth.frag");
	g_earthShader = earthShader.program;

	Shader cloudsShader("src/shader.vert", "src/shader_clouds.frag");
	g_cloudsShader = cloudsShader.program;

	g_Vao_earth = gl_createAndBindVAO();

	gl_createAndBindAttribute(&(earthShapes[0].mesh.positions[0]), earthShapes[0].mesh.positions.size() * sizeof(float),
		g_earthShader, "a_vertex", 3);

	gl_createIndexBuffer(&(earthShapes[0].mesh.indices[0]), earthShapes[0].mesh.indices.size() * sizeof(unsigned int));

	gl_createAndBindAttribute(&(earthShapes[0].mesh.texcoords[0]), earthShapes[0].mesh.texcoords.size() * sizeof(GLfloat), 
		g_earthShader, "a_uv", 2);

	gl_createAndBindAttribute(&(earthShapes[0].mesh.normals[0]), earthShapes[0].mesh.normals.size() * sizeof(float),
		g_earthShader, "a_normal", 3);


	gl_unbindVAO();
	g_NumTriangles_earth = earthShapes[0].mesh.indices.size() / 3;

	Image* image = loadBMP("assets/earthmap.bmp");

	glGenTextures(1, &(texture_id_earth));
	glBindTexture(GL_TEXTURE_2D, texture_id_earth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGB,
		image->width,
		image->height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		image->pixels);

	Image* image_normal = loadBMP("assets/earthnormal.bmp");

	glGenTextures(1, &(texture_id_earth_normal));
	glBindTexture(GL_TEXTURE_2D, texture_id_earth_normal);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGB,
		image_normal->width,
		image_normal->height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		image_normal->pixels);

	Image* image_spec = loadBMP("assets/earthspec.bmp");

	glGenTextures(1, &(texture_id_earth_spec));
	glBindTexture(GL_TEXTURE_2D, texture_id_earth_spec);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGB,
		image_spec->width,
		image_spec->height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		image_spec->pixels);

	Image* image_night = loadBMP("assets/earthnight.bmp");

	glGenTextures(1, &texture_id_earth_night);
	glBindTexture(GL_TEXTURE_2D, texture_id_earth_night);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, // target
		0, // level = 0 base, no mipmap
		GL_RGB, // how the data will be stored
		image_night->width, // width of the image
		image_night->height, // height of the image
		0, //border
		GL_RGB, // format of original data
		GL_UNSIGNED_BYTE, // type of data
		image_night->pixels); // pointer to the start of data

	Image* image_clouds = loadBMP("assets/clouds.bmp");

	glGenTextures(1, &texture_id_clouds);
	glBindTexture(GL_TEXTURE_2D, texture_id_clouds);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_clouds->width, image_clouds->height, 0, GL_RGB,
		GL_UNSIGNED_BYTE, image_clouds->pixels);

}

void loadPlanets() {
	// Load the mesh - SPHERE
	string err;
	bool ret = tinyobj::LoadObj(shapes, materials, err, inputfile.c_str(), basepath.c_str());

	//check for errors
	cout << "# of shapes mesh 1: " << shapes.size() << endl;
	if (!err.empty()) std::cerr << err << std::endl;

	Shader planetShader("src/shader_planet.vert", "src/shader_planet.frag");
	g_planetShader = planetShader.program;

	Shader sunShader("src/shader_sun.vert", "src/shader_sun.frag");
	g_sunShader = sunShader.program;

	g_Vao = gl_createAndBindVAO();
	gl_createAndBindAttribute(&(shapes[0].mesh.positions[0]), shapes[0].mesh.positions.size() * sizeof(float), g_planetShader, "a_vertex", 3);
	gl_createAndBindAttribute(&(shapes[0].mesh.texcoords[0]), shapes[0].mesh.texcoords.size() * sizeof(GLfloat), g_planetShader, "a_uv", 2);
	gl_createAndBindAttribute(&(shapes[0].mesh.normals[0]), shapes[0].mesh.normals.size() * sizeof(float), g_planetShader, "a_normal", 3);
	gl_createIndexBuffer(&(shapes[0].mesh.indices[0]), shapes[0].mesh.indices.size() * sizeof(unsigned int));

	gl_unbindVAO();

	g_NumTriangles = shapes[0].mesh.indices.size() / 3;

	for (int i = 0; i < NUM_PLANETS; i++) {
		planet newPlanet;
		GLuint texture_id = 0;

		//Load image to texture id

		cout << "Loading: " << textures[i] << "\n";

		Image* image = loadBMP(textures[i]);
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGB,
			image->width,
			image->height,
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			image->pixels);

		if (i == SUN) newPlanet.pivot_distance = 0;
		else if (i == MOON) newPlanet.pivot_distance = 0.8 * distances[i];
		else newPlanet.pivot_distance = 10.0f + 0.8 * distances[i];


		newPlanet.position = vec3(newPlanet.pivot_distance, 0.0, 0.0);
		newPlanet.scale = vec3(scales[i]*0.15, scales[i]*0.15, scales[i]*0.15);
		newPlanet.texture_id = texture_id;
		newPlanet.orbit_angle = rand() % 6;
		newPlanet.orbit_speed = NUM_PLANETS-i;
		newPlanet.rotation = 0;
		planets.push_back(newPlanet);

	}

}

// ------------------------------------------------------------------------------------------
// This function load the meshes to the memory and binds to the VAO
// ------------------------------------------------------------------------------------------
void load()
{
	loadSkybox();
	loadEarth();
	loadPlanets();

}

void drawSkybox() {
	glDisable(GL_DEPTH_TEST);
	glCullFace(GL_FRONT);

	glUseProgram(g_skyboxShader);
	gl_bindVAO(g_Vao_skybox);

	GLuint u_model = glGetUniformLocation(g_skyboxShader, "u_model");
	GLuint u_view = glGetUniformLocation(g_skyboxShader, "u_view");
	GLuint u_projection = glGetUniformLocation(g_skyboxShader, "u_projection");

	//set MVP
	mat4 projection_matrix = perspective(
		90.0f, // Field of view
		1.0f, // Aspect ratio
		0.1f, // near plane (distance from camera)
		600.0f // Far plane (distance from camera)
	);

	mat4 view_matrix = glm::lookAt(
		eye, // the position of your camera, in world space
		center, // where you want to look at, in world space
		vec3(0.0, 1.0, 0.0) // probably glm::vec3(0,1,0)
	);

	mat4 model_matrix = translate(scale(mat4(1.0f), vec3(80.0f, 80.0f, 80.0f)), vec3(0, 0, 0));

	//send all values to shader
	glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model_matrix));
	glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view_matrix));
	glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));

	GLuint u_texture = glGetUniformLocation(g_skyboxShader, "u_texture");
	glUniform1i(u_texture, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id_skybox);
	glDrawElements(GL_TRIANGLES, 3 * g_NumTriangles_skybox, GL_UNSIGNED_INT, 0);
	gl_unbindVAO();

}

void drawEarth() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glCullFace(GL_BACK);

	glUseProgram(g_earthShader);
	gl_bindVAO(g_Vao_earth);

	GLuint u_model = glGetUniformLocation(g_earthShader, "u_model");
	GLuint u_view = glGetUniformLocation(g_earthShader, "u_view");
	GLuint u_projection = glGetUniformLocation(g_earthShader, "u_projection");
	GLuint u_normal_matrix = glGetUniformLocation(g_earthShader, "u_normal_matrix");
	GLuint u_texture = glGetUniformLocation(g_earthShader, "u_texture");
	GLuint u_texture_normal = glGetUniformLocation(g_earthShader, "u_texture_normal");
	GLuint u_texture_spec = glGetUniformLocation(g_earthShader, "u_texture_spec");
	GLuint u_texture_night = glGetUniformLocation(g_earthShader, "u_texture_night");
	GLuint u_light_dir = glGetUniformLocation(g_earthShader, "u_light_dir");
	GLuint u_cam_pos = glGetUniformLocation(g_earthShader, "u_cam_pos");
	GLuint u_ambient = glGetUniformLocation(g_earthShader, "u_ambient");
	GLuint u_diffuse = glGetUniformLocation(g_earthShader, "u_diffuse");
	GLuint u_specular = glGetUniformLocation(g_earthShader, "u_specular");
	GLuint u_shininess = glGetUniformLocation(g_earthShader, "u_shininess");


	mat4 projection_matrix = perspective(
		50.0f, // Field of view
		1.0f, // Aspect ratio
		0.1f, // near plane (distance from camera)
		600.0f // Far plane (distance from camera)
	);

	mat4 view_matrix = glm::lookAt(
		eye, // the position of your camera, in world space
		center, // where you want to look at, in world space
		vec3(0.0, 1.0, 0.0) // probably glm::vec3(0,1,0)
	);
	mat4 model_matrix = translate(mat4(1.0f), earthPosition) 
		* scale(mat4(1.0f), vec3(0.45, 0.45, 0.45)) 
		* rotate(mat4(1.0f), earth_rotation, vec3(0.0f, 1.0f, 0.0f));
	mat3 normal_matrix = transpose(inverse(mat3(model_matrix)));


	float light_x = g_light_distance * sinf(g_light_angle);
	float light_z = g_light_distance * cosf(g_light_angle);

	glUniformMatrix4fv(u_model, 1, GL_FALSE, value_ptr(model_matrix));
	glUniformMatrix4fv(u_view, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(u_projection, 1, GL_FALSE, value_ptr(projection_matrix));
	glUniformMatrix3fv(u_normal_matrix, 1, GL_FALSE, value_ptr(normal_matrix));
	glUniform1i(u_texture, 0);
	glUniform1i(u_texture_normal, 1);
	glUniform1i(u_texture_spec, 2);
	glUniform1i(u_texture_night, 3);
	glUniform3f(u_light_dir, g_light_dir.x - earthPosition.x, g_light_dir.y - earthPosition.y, g_light_dir.z - earthPosition.z);
	glUniform3f(u_diffuse, 1.0, 1.0, 1.0);
	glUniform3f(u_specular, 1.0, 1.0, 1.0);
	glUniform3f(u_cam_pos, eye.x, eye.y, eye.z);
	glUniform1f(u_shininess, 50.0);
	glUniform3f(u_ambient, 0.1, 0.1, 0.2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id_earth);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture_id_earth_normal);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture_id_earth_spec); 

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texture_id_earth_night);

	glDrawElements(GL_TRIANGLES, 3 * g_NumTriangles_earth, GL_UNSIGNED_INT, 0);

	
	//				Clouds


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(g_cloudsShader);

	GLuint projection_loc = glGetUniformLocation(g_cloudsShader, "u_projection");
	GLuint view_loc = glGetUniformLocation(g_cloudsShader, "u_view");
	GLuint model_loc = glGetUniformLocation(g_cloudsShader, "u_model");
	//GLuint cu_normal_matrix = glGetUniformLocation(g_earthShader, "u_normal_matrix");
	GLuint transparency_loc = glGetUniformLocation(g_cloudsShader, "u_transparency");
	GLuint texture = glGetUniformLocation(g_cloudsShader, "u_texture");

	mat4 c_projection_matrix = perspective(
		50.0f, // Field of view
		1.0f, // Aspect ratio
		0.1f, // near plane (distance from camera)
		600.0f // Far plane (distance from camera)
	);

	mat4 c_view_matrix = glm::lookAt(
		eye, // the position of your camera, in world space
		center, // where you want to look at, in world space
		vec3(0.0, 1.0, 0.0) // probably glm::vec3(0,1,0)
	);

	mat4 c_model_matrix = translate(mat4(1.0f), earthPosition)
		* scale(mat4(1.0f), vec3(0.46, 0.46, 0.46))
		* rotate(mat4(1.0f), clouds_rotation, vec3(0.0f, 1.0f, 0.0f));

	mat3 c_normal_matrix = transpose(inverse(mat3(c_model_matrix)));


	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, value_ptr(c_projection_matrix));
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, value_ptr(c_view_matrix));
	glUniformMatrix4fv(model_loc, 1, GL_FALSE, value_ptr(c_model_matrix));
	//glUniformMatrix3fv(cu_normal_matrix, 1, GL_FALSE, value_ptr(c_normal_matrix));
	glUniform1f(transparency_loc, 0.4f);
	glUniform1i(texture, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id_clouds);

	glDrawElements(GL_TRIANGLES, 3 * g_NumTriangles, GL_UNSIGNED_INT, 0);
	glDisable(GL_BLEND);

	gl_unbindVAO();
}

void drawPlanets() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// activate shader
	glUseProgram(g_planetShader);

	for (int i = 1; i < NUM_PLANETS; i++) {

		GLuint projection_loc = glGetUniformLocation(g_planetShader, "u_projection");
		GLuint view_loc = glGetUniformLocation(g_planetShader, "u_view");
		GLuint u_normal_matrix = glGetUniformLocation(g_planetShader, "u_normal_matrix");
		GLuint light_dir_loc = glGetUniformLocation(g_planetShader, "u_light_dir");
		GLuint light_color_loc = glGetUniformLocation(g_planetShader, "u_light_color");
		GLuint eye_loc = glGetUniformLocation(g_planetShader, "u_eye");
		GLuint model_loc = glGetUniformLocation(g_planetShader, "u_model");
		GLuint ambient_loc = glGetUniformLocation(g_planetShader, "u_ambient");
		GLuint glossiness_loc = glGetUniformLocation(g_planetShader, "u_glossiness");
		GLuint u_texture = glGetUniformLocation(g_planetShader, "u_texture");

		mat4 projection_matrix = perspective(
			50.0f, // Field of view
			1.0f, // Aspect ratio
			0.1f, // near plane (distance from camera)
			600.0f // Far plane (distance from camera)
		);

		mat4 view_matrix = glm::lookAt(
			eye, // the position of your camera, in world space
			center, // where you want to look at, in world space
			vec3(0.0, 1.0, 0.0) // probably glm::vec3(0,1,0)
		);

		mat4 model = translate(mat4(1.0f), planets[i].position) 
			* scale(mat4(1.0f), planets[i].scale) 
			* rotate(mat4(1.0f), planets[i].rotation, vec3(0.0f, 1.0f, 0.0f));
		mat3 normal_matrix = transpose(inverse(mat3(model)));

		glUniformMatrix4fv(projection_loc, 1, GL_FALSE, value_ptr(projection_matrix));
		glUniformMatrix4fv(view_loc, 1, GL_FALSE, value_ptr(view_matrix));
		glUniformMatrix4fv(model_loc, 1, GL_FALSE, value_ptr(model));
		glUniformMatrix3fv(u_normal_matrix, 1, GL_FALSE, value_ptr(normal_matrix));
		glUniform3f(light_dir_loc, g_light_dir.x - planets[i].position.x, g_light_dir.y - planets[i].position.y, g_light_dir.z - planets[i].position.z);
		glUniform3f(light_color_loc, 1.0, 1.0, 1.0);
		glUniform3f(eye_loc, eye.x, eye.y, eye.z);
		glUniform3f(ambient_loc, 0.1, 0.1, 0.1);
		glUniform1f(glossiness_loc, 50);
		glUniform1i(u_texture, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, planets[i].texture_id);

		//bind the geometry
		gl_bindVAO(g_Vao);

		// Draw to screen
		glDrawElements(GL_TRIANGLES, 3 * g_NumTriangles, GL_UNSIGNED_INT, 0);
	}

}


void drawSun() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// activate shader
	glUseProgram(g_sunShader);

	GLuint projection_loc = glGetUniformLocation(g_sunShader, "u_projection");
	GLuint view_loc = glGetUniformLocation(g_sunShader, "u_view");
	GLuint u_normal_matrix = glGetUniformLocation(g_sunShader, "u_normal_matrix");
	GLuint model_loc = glGetUniformLocation(g_sunShader, "u_model");
	GLuint u_texture = glGetUniformLocation(g_sunShader, "u_texture");

	mat4 projection_matrix = perspective(
		50.0f, // Field of view
		1.0f, // Aspect ratio
		0.1f, // near plane (distance from camera)
		600.0f // Far plane (distance from camera)
	);

	mat4 view_matrix = glm::lookAt(
		eye, // the position of your camera, in world space
		center, // where you want to look at, in world space
		vec3(0.0, 1.0, 0.0) // probably glm::vec3(0,1,0)
	);

	mat4 model = translate(mat4(1.0f), planets[0].position)
		* scale(mat4(1.0f), planets[0].scale)
		* rotate(mat4(1.0f), planets[0].rotation, vec3(0.0f, 1.0f, 0.0f));
	mat3 normal_matrix = transpose(inverse(mat3(model)));

	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, value_ptr(projection_matrix));
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(model_loc, 1, GL_FALSE, value_ptr(model));
	glUniformMatrix3fv(u_normal_matrix, 1, GL_FALSE, value_ptr(normal_matrix));
	glUniform1i(u_texture, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, planets[0].texture_id);

	//bind the geometry
	gl_bindVAO(g_Vao);

	// Draw to screen
	glDrawElements(GL_TRIANGLES, 3 * g_NumTriangles, GL_UNSIGNED_INT, 0);
}

// ------------------------------------------------------------------------------------------
// This function actually draws to screen and called non-stop, in a loop
// ------------------------------------------------------------------------------------------
void draw()
{
	//clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);

	drawSkybox();
	drawEarth();
	drawPlanets();
	drawSun();
}

// ------------------------------------------------------------------------------------------
// This function is called every time you press a screen
// ------------------------------------------------------------------------------------------
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (action == GLFW_PRESS)
		switch (key)
		{
		case GLFW_KEY_W:
			key_flags[0] = 1;
			break;
		case GLFW_KEY_A:
			key_flags[1] = 1;
			break;
		case GLFW_KEY_S:
			key_flags[2] = 1;
			break;
		case GLFW_KEY_D:
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
			key_flags[0] = 0;
			break;
		case GLFW_KEY_A:
			key_flags[1] = 0;
			break;
		case GLFW_KEY_S:
			key_flags[2] = 0;
			break;
		case GLFW_KEY_D:
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
	// use time to control the speed consistently across machines
	deltaTime = (glfwGetTime() - lastTime) * 10000;
	lastTime = glfwGetTime();

	//Update planets position and rotation
	for (int i = 0; i < NUM_PLANETS; i++) {
		planets[i].orbit_angle -= planets[i].orbit_speed * 0.0001f;

		if (i == MOON) {
			planets[i].position.x = cos(planets[i].orbit_angle) * planets[i].pivot_distance + earthPosition.x;
			planets[i].position.z = sin(planets[i].orbit_angle) * planets[i].pivot_distance + earthPosition.z;
		}
		else {
			planets[i].position.x = cos(planets[i].orbit_angle) * planets[i].pivot_distance;
			planets[i].position.z = sin(planets[i].orbit_angle) * planets[i].pivot_distance;
		}
		

		//Rotation
		planets[i].rotation -= 0.003f * deltaTime;
		if (planets[i].rotation > 6.28f) planets[i].rotation = 0.0f;
	}

	earth_angle -= 0.2 * 0.001f;
	earthPosition.x = cos(earth_angle) * (3 + 10.0f);
	earthPosition.z = sin(earth_angle) * (3 + 10.0f);

	//Rotation
	earth_rotation -= 0.003f * deltaTime;
	if (earth_rotation > 6.28f) earth_rotation = 0.0f;

	clouds_rotation -= 0.001f * deltaTime;;
	if (clouds_rotation > 6.28f) clouds_rotation = 0;

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


