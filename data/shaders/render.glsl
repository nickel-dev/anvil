#ifdef VERTEX_SHADER

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv0;
layout (location = 2) in vec4 color0;
layout (location = 3) in vec3 normal0;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 xform;
uniform mat4 light_space;  // New: light space matrix

out vec2 uv;
out vec4 color;
out vec3 normal;
out vec3 frag_pos;
out vec4 frag_pos_light_space;  // For shadow mapping

void main() {
    gl_Position = projection * view * xform * vec4(position, 1.0);
    uv = uv0;
	color = color0;
    normal = transpose(inverse(mat3(xform))) * normalize(normal0);
    frag_pos = vec3(xform * vec4(position, 1.0));
    frag_pos_light_space = light_space * vec4(frag_pos, 1.0);  // Transform to light space
}

#else

in vec2 uv;
in vec4 color;
in vec3 normal;
in vec3 frag_pos;
in vec4 frag_pos_light_space;

uniform sampler2D shadow_map;
uniform vec3 light_pos;
uniform vec3 view_pos;

float shadow_calculation(vec4 frag_pos_light_space) {
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    proj_coords = proj_coords * 0.5 + 0.5;
    
    // Early exit if outside shadow map
    if(proj_coords.z > 1.0 || any(lessThan(proj_coords.xy, vec2(0.0))))
		return 0.0;
	
	float closest_depth = texture(shadow_map, proj_coords.xy).r;
	float current_depth = proj_coords.z;
	
	// Improved bias calculation
	vec3 light_dir = normalize(light_pos - frag_pos);
	float bias = max(0.05 * (1.0 - dot(normal, light_dir)), 0.005);
	
	// Check if depth is in shadow
	float shadow = current_depth - bias > closest_depth ? 1.0 : 0.0;
	
	return shadow;
}

void main() {
    // Basic lighting
    vec3 light_color = vec3(1.0);
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light_pos - frag_pos);
    
    // Ambient
    vec3 ambient = 0.2 * light_color;
    
    // Diffuse
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;
    
    // Specular
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32.0);
    vec3 specular = 0.5 * spec * light_color;
    
    // Shadow
    float shadow = shadow_calculation(frag_pos_light_space);
    
    // Final color with shadow
    vec3 result = (ambient + (1.0f - shadow) * (diffuse + specular)) * color.rgb;
    gl_FragColor = vec4(result, color.a);
}

#endif