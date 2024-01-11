#pragma once

#include <glm/glm.hpp>

// The uniform buffer objects data structures
// Remember to use the correct alignas(...) value
//        float : alignas(4)
//        vec2  : alignas(8)
//        vec3  : alignas(16)
//        vec4  : alignas(16)
//        mat3  : alignas(16)
//        mat4  : alignas(16)

struct MeshUniformBlock {
	alignas(4) float amb;
	alignas(4) float gamma;
	alignas(16) glm::vec3 sColor;
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

struct OverlayUniformBlock {
	alignas(4) float visible;
};

struct GlobalUniformBlock {
	alignas(16) glm::vec3 DlightDir;
	alignas(16) glm::vec3 DlightColor;
	alignas(16) glm::vec3 AmbLightColor;
	alignas(16) glm::vec3 eyePos;
	alignas(16) glm::vec3 spotLightDir;
	alignas(16) glm::vec3 spotLightPos;
	alignas(16) glm::vec4 spotLightColor; 
	alignas(16) glm::vec3 spotLight_eyePos;
};

// The vertices data structures
struct VertexMesh {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};

struct VertexOverlay {
	glm::vec2 pos; //????
	glm::vec2 UV;
};

struct Vertex3DOverlay {
	glm::vec3 pos;
	glm::vec2 UV;
};

/* A16 */
/* Add the C++ datastructure for the required vertex format */
struct VertexVColor { 
	glm::vec3 pos; 
	glm::vec3 norm; 
	glm::vec3 color; 
}; 