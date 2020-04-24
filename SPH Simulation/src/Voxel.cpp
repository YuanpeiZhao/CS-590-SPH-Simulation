#include "Voxel.h"

bool inSphere(float r, vec3 o, vec3 pos) {
	return distance(o, pos) <= r;
}

bool inSquare(vec2 pos, vec2 center, vec2 shape) {
	return true;
	return abs(pos.x - center.x) <= shape.x/2.0f && abs(pos.y - center.y) <= shape.y/2.0f;
}

bool inTerrain(float basic, vec2 peak1pos, vec3 peak1shape, vec2 peak2pos, vec3 peak2shape, vec3 pos) {
	if (pos.y <= basic) return true;
	if (inSquare(vec2(pos.x, pos.z), peak1pos, vec2(peak1shape.x, peak1shape.z))) {
		//float sinX = -peak1shape.y * cos(2 * PI / peak1shape.x * pos.x + peak1shape.x - peak1pos.x) + peak1shape.y + basic;
		//float sinZ = -peak1shape.y * cos(2 * PI / peak1shape.z * pos.z + peak1shape.z - peak1pos.y) + peak1shape.y + basic;
		//if (pos.y < sinX) return true;
		//return false;
	}

	if (inSquare(vec2(pos.x, pos.z), peak2pos, vec2(peak2shape.x, peak2shape.z))) {
		float sinX = -peak2shape.y * cos(2 * PI / peak2shape.x * pos.x + peak2shape.x/2.0f - peak2pos.x) + peak2shape.y + basic;
		float sinZ = -peak2shape.y * cos(2 * PI / peak2shape.z * pos.z - peak2shape.z/2.0f + peak2pos.y) + peak2shape.y + basic;
		if (pos.y < sinZ) return true;
		return false;
	}
	return false;
}

vec3 getPositionInVoxel(int i, int j, int k) {
	vec3 rate = vec3(float(i)/float(voxelDimensionX),
					float(j) / float(voxelDimensionY),
					float(k) / float(voxelDimensionZ));

	return vec3(voxelLowerBound + rate * (voxelUpperBound - voxelLowerBound));
}

void writeSphereToFile() {
	ofstream out;
	out.open("voxelData.txt");

	out << voxelDimensionX << ' ' << voxelDimensionY << ' ' << voxelDimensionZ << endl;
	out << voxelLowerBound.x << ' ' << voxelLowerBound.y << ' ' << voxelLowerBound.z << endl;
	out << voxelUpperBound.x << ' ' << voxelUpperBound.y << ' ' << voxelUpperBound.z << endl;
	for (int i = 0; i < voxelDimensionX; i++) {
		for (int j = 0; j < voxelDimensionY; j++) {
			for (int k = 0; k < voxelDimensionZ; k++) {
				if (inSphere(2.0f, vec3(0.0f), getPositionInVoxel(i, j, k))) {
					out << "1 ";
				}
				else {
					out << "0 ";
				}
			}
		}
	}

	out.close();
}

void writeTerrainToFile() {
	ofstream out;
	out.open("voxelData.txt");

	out << voxelDimensionX << ' ' << voxelDimensionY << ' ' << voxelDimensionZ << endl;
	out << voxelLowerBound.x << ' ' << voxelLowerBound.y << ' ' << voxelLowerBound.z << endl;
	out << voxelUpperBound.x << ' ' << voxelUpperBound.y << ' ' << voxelUpperBound.z << endl;
	for (int i = 0; i < voxelDimensionX; i++) {
		for (int j = 0; j < voxelDimensionY; j++) {
			for (int k = 0; k < voxelDimensionZ; k++) {
				if (inTerrain(0.1f, vec2(-2.0f), vec3(2.0f, 0.5f, 2.0f), vec2(2.0f), vec3(2.0f, 0.5f, 2.0f), getPositionInVoxel(i, j, k))) {
					out << "1 ";
				}
				else {
					out << "0 ";
				}
			}
		}
	}

	out.close();
}

void readVoxelToParticle(bool* voxels) {
	ifstream fin("voxelData.txt");

	string line;
	getline(fin, line);
	getline(fin, line);
	getline(fin, line);
	getline(fin, line);

	int index = 0;

	for (int i = 0; i < voxelDimensionX; i++) {
		for (int j = 0; j < voxelDimensionY; j++) {
			for (int k = 0; k < voxelDimensionZ; k++) {

				char tmp = line[index];

				index += 2;

				if (tmp == '1') {
					voxels[i*voxelDimensionY*voxelDimensionZ+j*voxelDimensionZ+k] = true;
				}
				else {
					voxels[i * voxelDimensionY * voxelDimensionZ + j * voxelDimensionZ + k] = false;
				}
			}
		}
	}

	fin.close();
}