#version 330
 
in vec3 a_vertex;
in vec3 a_color;

// STEP 2: add a attribute: a_uv and varying v_uv here

out vec3 v_color;

void main()
{
	gl_Position = vec4( a_vertex , 1.0 );

	// pass the colour to the fragment shader
	v_color = a_color;

    // STEP 2: pass the uv coordinates to the fragment shader
}

