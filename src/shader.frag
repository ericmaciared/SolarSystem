#version 330

out vec4 fragColor;

in vec3 v_color;

// STEP 3: get the uv coordinates form the vertex shader


void main(void)
{
	// STEP 3: use the uv coordinates as colors
    // STEP 8: get the color from the texture
	fragColor =  vec4(v_color,1.0);     
	
}
