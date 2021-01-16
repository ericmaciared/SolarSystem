#version 330

in vec2 v_uv;
in vec3 v_vertex;
in vec3 v_normal;

out vec4 fragColor;

uniform sampler2D u_texture;
uniform vec3 u_color;
uniform vec3 u_light_dir;
uniform vec3 u_cam_pos;
uniform vec3 u_ambient;
uniform vec3 u_diffuse;
uniform vec3 u_specular;
uniform float u_shininess;


void main(void)
{
	vec3 texture_color = texture(u_texture, v_uv).xyz;

	vec3 N = normalize(v_normal);
	vec3 L = normalize(u_light_dir - v_vertex);
	vec3 R = normalize(-reflect(L, N));
	vec3 E = normalize(u_cam_pos - v_vertex);

	float NdotL = max (dot(N,L), 0.0);
	float RdotE = pow(max(dot(R,E), 0.0), u_shininess);
		
	vec3 amb = texture_color * u_ambient;
	vec3 diff = texture_color * NdotL * u_diffuse;
	vec3 spec = texture_color * u_specular * RdotE; 

	vec3 final_color = amb + diff + spec;
	fragColor = vec4 (final_color, 1.0);

}