#version 410 core

uniform sampler2DArray textureArray;
uniform sampler2DArray normalArray;
uniform sampler2D shadowMap;

uniform float fog_min;
uniform float fog_max;

out vec4 frag_color;

in VS_OUT {
	vec4 frag_viewspace;
	vec4 frag_light_space;
	vec3 frag_pos;
	vec3 frag_normal;
	vec2 frag_uv;
	flat uint frag_opaque;
	flat uint frag_type;
	float frag_ao;

	vec3 sun_position;
	vec3 camera_position;

	flat uint diffuse_index;
	flat uint normal_index;

	mat3 TBN;
} fs_in;


float PCF(vec3 projCoords, float currentDepth, float bias) {
	float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

	return shadow;
}

float calcShadow(vec4 fragLightSpace, vec3 normal) {
    vec3 projCoords = fragLightSpace.xyz / fragLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5; 

	float closestDepth = texture(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;

	float bias = max(0.000001 * (1.0 - dot(normal, vec3(200.0, 200.0, 200.0))), 0.0000001);
	float shadow = PCF(projCoords, currentDepth, bias);
    
    if (projCoords.z > 1.0) shadow = 0.0;
    
	return shadow;
}

vec4 layer(vec4 foreground, vec4 background) {
	return foreground * foreground.a + background * (1.0 - foreground.a);
}

void main() {
	bool isOpaque = bool(fs_in.frag_opaque);
	vec4 final = texture(textureArray, vec3(fs_in.frag_uv, fs_in.diffuse_index));
	vec4 color;

	vec3 normal = fs_in.TBN * normalize((texture(normalArray, vec3(fs_in.frag_uv, fs_in.normal_index)).rgb * 2.0 - 1.0));

	if (isOpaque) {
		float ao = clamp(fs_in.frag_ao, 0.0, 1.0);
		final *= smoothstep(0.0, 1.0, ao);
		color = vec4(final.xyz, 1.0);
	} else {
		color = final;
	}

	float ambientStrength = 0.15, specularStrength = 0.5;

	vec3 lightColor = vec3(1.0, 1.0, 0.785);
	vec3 ambient = ambientStrength * lightColor;
	float shadow = calcShadow(fs_in.frag_light_space, fs_in.frag_normal);
	vec3 lightDir = normalize(fs_in.sun_position - fs_in.frag_pos);
	vec3 diffuse = max(dot(normal, lightDir), 0.0) * lightColor;

    /*vec3 viewDir = normalize(camera_position - fs_in.frag_pos);
    vec3 reflectDir = reflect(-lightDir, fs_in.frag_normal);
    vec3 specular = specularStrength * pow(max(dot(viewDir, reflectDir), 0.0), 64) * lightColor;*/

	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse/* + specular*/));


	frag_color = vec4(color.xyz * lighting, color.a);

}