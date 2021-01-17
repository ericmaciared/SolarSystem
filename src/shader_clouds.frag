#version 330

in vec2 v_uv;

out vec4 fragColor;

uniform sampler2D u_texture; 
uniform float u_transparency;

void main(void)
{
	vec3 texture_color = texture(u_texture, v_uv).xyz;
	fragColor = vec4(1-texture_color.x, 1-texture_color.y, 1-texture_color.z, u_transparency);

}