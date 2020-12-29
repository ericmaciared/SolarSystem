#version 330


uniform vec3 u_color;
out vec4 fragColor;
in vec3 v_color;
in vec2 v_uv;

uniform sampler2D u_texture;

in vec3 v_normal;

uniform vec3 u_light_dir;


void main(void)
{
	vec4 texture_color = texture(u_texture, v_uv);

	vec3 N = normalize(v_normal);
	fragColor = vec4(N, 1.0);

	vec3 L = normalize(u_light_dir);
	float NdotL = max (dot(N,L), 0.0) ;
		
	// We're just going to paint the interpolated colour from the vertex shader
	//vec3 final_color = v_color * NdotL;

	vec3 final_color = texture_color.xyz * NdotL;
	fragColor = vec4 ( final_color, 1.0);


}
