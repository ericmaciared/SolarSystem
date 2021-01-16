#version 330
 

in vec2 a_uv;
in vec3 a_normal;
in vec3 a_vertex;
in vec3 a_color;

out vec2 v_uv;
out vec3 v_normal;
out vec3 v_vertex;
out vec3 v_color;

uniform mat4 u_model;
uniform mat4 u_projection;
uniform mat4 u_view;


void main()
{
	// position of the vertex
	gl_Position = u_projection * u_view * u_model * vec4( a_vertex , 1.0 );

	// pass the colour to the fragment shader
	v_color = a_color;
	
	// pass the uv coordinates to the fragment shader
	v_uv = a_uv;

	//pass normal values to fragment shader
	v_normal = (a_normal * vec4( a_normal, 1.0 ).xyz);

	//pass vertex values to fragment shader
	v_vertex = (u_model * vec4( a_vertex , 1.0 )).xyz;

}