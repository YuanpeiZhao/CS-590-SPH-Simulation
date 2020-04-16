#ifndef _VOXEL_
#define _VOXEL_

#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

const int voxelDimensionX = 64;
const int voxelDimensionY = 64;
const int voxelDimensionZ = 64;
const glm::vec3 voxelLowerBound = glm::vec3(-2.0f, -2.0f, -2.0f);
const glm::vec3 voxelUpperBound = glm::vec3( 2.0f,  2.0f,  2.0f);

vec3 getPositionInVoxel(int i, int j, int k);
void writeSphereToFile();
void readVoxelToParticle(bool* voxels);

#endif