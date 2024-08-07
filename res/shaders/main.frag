#version 410 core

out vec4 frag_color;

in vec4 frag_viewspace;
in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_uv;
in float frag_ao;
flat in uint frag_type;
flat in uint frag_opaque;

in vec4 frag_pos_light_space;
uniform sampler2D shadowMap;

float PCF(vec3 projCoords, float currentDepth, float bias) {
	float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

	return shadow;
}

float calcShadow(vec4 fposLightSpace) {
	// perform perspective divide
    vec3 projCoords = fposLightSpace.xyz / fposLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5; 

	float closestDepth = texture(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;

	float bias = max(0.000001 * (1.0 - dot(frag_normal, vec3(200.0, 200.0, 200.0))), 0.0000001);
	float shadow = PCF(projCoords, currentDepth, bias);
    
    if (projCoords.z > 1.0) shadow = 0.0;
    
	return shadow;
}

uniform float fog_min;
uniform float fog_max;

uniform sampler2DArray arrayTexture;
uniform vec3 camera_position;
uniform vec3 camera_direction;

vec4 fog_color = vec4(0.588, 0.784, 1.0, 1.0);

vec4 layer(vec4 foreground, vec4 background) {
	return foreground * foreground.a + background * (1.0 - foreground.a);
}

void main() {
	bool isOpaque = bool(frag_opaque);
	vec4 final = texture(arrayTexture, vec3(frag_uv, frag_type));
	vec4 color;

	// Calculate fog
	/*float dist = length(frag_viewspace.xyz);
	float fog_factor = (fog_max - dist) / (fog_max - fog_min);
	fog_factor = clamp(fog_factor, 0.0, 1.0);*/

	if (isOpaque) {
		float ao = clamp(frag_ao, 0.0, 1.0);
		final *= smoothstep(0.0, 1.0, ao);

		//color = vec4(mix(fog_color, final, fog_factor).xyz, 1.0);
		color = vec4(final.xyz, 1.0);
	} else {
		//color = mix(fog_color, final, fog_factor);
		color = final;
	}

	float ambient = 0.3;

	float shadow = calcShadow(frag_pos_light_space);
	frag_color = vec4(color.xyz * (ambient + (1.0 - shadow)), color.a);

}