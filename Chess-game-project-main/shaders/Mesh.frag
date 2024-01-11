#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
	vec3 DlightDir;		// direction of the direct light
	vec3 DlightColor;	// color of the direct light
	vec3 AmbLightColor;	// ambient light
	vec3 eyePos;		// position of the viewer
	vec3 spotLightDir;
	vec3 spotlightPos;		// position of the spotlight
	vec4 spotlightColor;
	vec3 spot_eyePos;
} gubo;

layout(set = 1, binding = 0) uniform UniformBufferObject {
	float amb;
	float gamma;
	vec3 sColor;
	mat4 mvpMat;
	mat4 mMat;
	mat4 nMat;
} ubo;

layout(set = 1, binding = 1) uniform sampler2D tex;


//spotlight stuff

const float beta = 2.0f;
const float g = 1.5;
const float cosout = 0.85;
const float cosin  = 0.95;

void main() {
	vec3 N = normalize(fragNorm);				// surface normal
	vec3 V = normalize(gubo.eyePos - fragPos);	// viewer direction
	vec3 L = normalize(gubo.DlightDir);			// light direction

	vec3 albedo = texture(tex, fragUV).rgb;		// main color
	vec3 MD = albedo;
	vec3 MS = ubo.sColor;
	vec3 MA = albedo * ubo.amb;
	vec3 LA = gubo.AmbLightColor;
	
	//ADDED
	
 
	vec3 d = gubo.spotLightDir;

 	vec3 lightDir = normalize(gubo.spotlightPos - fragPos); // lx
 	float cos_alpha = dot(lightDir,d);
	float clamped_val = clamp((cos_alpha-cosout)/(cosin-cosout),0,1);

	vec3 lightColor = vec3(gubo.spotlightColor);
	vec3 finalColor = lightColor * pow(g/length(gubo.spotlightPos - fragPos),beta) * clamped_val;

	vec3 Diffuse = texture(tex, fragUV).rgb * 0.99f * clamp(dot(N, lightDir),0.0,1.0);
	vec3 Specular = vec3(pow(clamp(dot(N, normalize(lightDir + V)),0.0,1.0), 160.0f));
	vec3 Ambient = texture(tex, fragUV).rgb * 0.01f;
	
	outColor = vec4(clamp((Diffuse + Specular) * finalColor.rgb + Ambient,0.0,1.0), 1.0f);

	// END ADDED
/*
	outColor = outColor_1;

	outColor = clamp ( outColor_1 + vec4(
				clamp(MD * clamp(dot(L,N),0.0f,1.0f) +
					  MS * pow(clamp(dot(N, normalize(L + V)), 0.0f, 1.0f), ubo.gamma) +
					  LA * MA,
				0.0f, 1.0f), 1.0f),0.0f,1.0f);	// output color
	*/
 }