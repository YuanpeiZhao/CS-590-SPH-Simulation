#include "Voxel.h"

bool inSphere(float r, vec3 o, vec3 pos) {
	return distance(o, pos) <= r;
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