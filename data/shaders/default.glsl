#ifdef VERTEX_SHADER

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv0;
layout (location = 2) in vec4 color0;
layout (location = 3) in vec3 normal0;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 view_light;
uniform mat4 xform;

out vec2 uv;
out vec4 color;
out vec3 normal;
out vec3 frag_pos;
out vec4 frag_pos_light_space;

void main() {
	uv = uv0;
    color = color0;
	normal = transpose(inverse(mat3(xform))) * normalize(normal0);
	frag_pos = vec3(xform * vec4(position, 1.0));
	frag_pos_light_space = view_light * vec4(frag_pos, 1.0);
	
	gl_Position = projection * view * xform * vec4(position, 1.0);
}

#else

uniform sampler2D texture0;
uniform sampler2D shadow_map;

in vec2 uv;
in vec4 color;
in vec3 normal;
in vec3 frag_pos;
in vec4 frag_pos_light_space;

uniform vec3 view_pos;

// light_pos = vec3(-1.0f, 2.0f, 5.0f)

float ShadowCalculation(vec4 fragPosLightSpace) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadow_map, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(normal);
    vec3 lightDir = normalize(vec3(-1.0f, 2.0f, 5.0f) - frag_pos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadow_map, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadow_map, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
	
    return shadow;
}

void main() {
	vec4 frag_color = texture(texture0, uv);
    vec3 normal = normalize(normal);
    vec3 light_color = vec3(0.3);
    
	if (frag_color.w == 0.0f) {
		discard;
	}
	
	// ambient
    vec3 ambient = 0.3 * light_color;
	
    // diffuse
    vec3 light_dir = normalize(vec3(-1.0f, 2.0f, 5.0f) - frag_pos);
    float diff = max(dot(light_dir, normal), 0.0);
    vec3 diffuse = diff * light_color;
    
	// specular
    vec3 viewDir = normalize(view_pos - frag_pos);
    vec3 reflectDir = reflect(-light_dir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(light_dir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * light_color;    
    
	// calculate shadow
    float shadow = ShadowCalculation(frag_pos_light_space);                      
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * frag_color.rgb;    
	
	gl_FragColor = vec4(lighting, frag_color.w);
}

#endif