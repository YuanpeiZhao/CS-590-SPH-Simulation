#ifndef _PARTICLE_
#define _PARTICLE_

#define WORK_GROUP_SIZE 256

#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "imgui_impl_glfw_gl3.h"

#include "Voxel.h"

struct Particle {
	glm::vec4 currPos;
	glm::vec4 prevPos;
	glm::vec4 acc;
	glm::vec4 vel;
	glm::vec4 para;
	glm::vec4 deltaCs;
	glm::vec4 matProp;
};

void initParticles(GLuint& ssb, GLuint pNumber, int r, bool* voxels);
void initImGUI(GLFWwindow* window);
void initVAO(GLuint& vao, GLuint& ppvao, GLuint& ssb);
GLuint initShader(GLuint& program, const char* shaderFile, GLenum shaderType);
void initDepthMap(GLFWwindow* window, GLuint& depthMap, GLuint& depthMapFBO);

#endif