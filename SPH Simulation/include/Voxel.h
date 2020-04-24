#ifndef _VOXEL_
#define _VOXEL_

#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define PI 3.14159265358979f

using namespace std;
using namespace glm;

const int voxelDimensionX = 128;
const int voxelDimensionY = 16;
const int voxelDimensionZ = 128;
const glm::vec3 voxelLowerBound = glm::vec3(-4.0f, 0.0f, -4.0f);
const glm::vec3 voxelUpperBound = glm::vec3( 4.0f,  1.1f,  4.0f);

vec3 getPositionInVoxel(int i, int j, int k);
void writeSphereToFile();
void writeTerrainToFile();
void readVoxelToParticle(bool* voxels);

#endif