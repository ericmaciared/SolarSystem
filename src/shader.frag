#version 330



uniform vec3 u_color;
out vec4 fragColor;
in vec3 v_color;
in vec2 v_uv;

uniform sampler2D u_texture;


void main(void)
{
	vec4 texture_color = texture(u_texture, v_uv);

	// We're just going to paint the interpolated colour from the vertex shader
	fragColor = vec4(texture_color.xyz, 1.0);
}
