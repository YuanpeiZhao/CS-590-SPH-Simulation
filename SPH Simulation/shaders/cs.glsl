#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

#define PI_FLOAT 3.1415927410125732421875f
#define PARTICLE_RADIUS 0.025f
#define PARTICLE_RESTING_DENSITY 1000
#define PARTICLE_MASS 65
#define SMOOTHING_LENGTH (24 * PARTICLE_RADIUS)
#define PARTICLE_VISCOSITY 250.f
#define GRAVITY_FORCE vec3(0, -10, 0)
#define PARTICLE_STIFFNESS 200
#define WALL_DAMPING 0.8f

struct particle{
  vec4  currPos;
  vec4  prevPos;
  vec4  acceleration;
  vec4  velocity;
  vec4  pamameters;
  vec4  deltaCs;	
  vec4  matProp;		// 0: if it has voxel(0 no; 1 yes) 1: duration(if less than 0, destroy)
};

layout(std430, binding=0) buffer particles{
  particle p[];
};

uniform int pass;
uniform float frameTimeDiff;
uniform uint maxParticles;
uniform uint maxVoxels;
uniform int voxelX;
uniform int voxelY;
uniform int voxelZ;

layout (rgba32f, binding = 0) uniform imageBuffer voxelData;	// 0: if has voxel, 1: duration, 2: if is hit

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

ivec3 convertIndex1DToIndex3D(int index){

	int i = int(index);
	int x = i / (voxelY * voxelZ);
	int y = (i % (voxelY * voxelZ)) / voxelZ;
	int z = i % voxelZ;

	return ivec3(x,y,z);
}

int convertIndex3DToIndex1D(ivec3 index){

	return index.x * voxelY * voxelZ + index.y * voxelZ + index.z;
}

int convertPositionToIndex1D(vec3 pos){
	
	if(max(abs(pos.x), max(abs(pos.y), abs(pos.z))) >= 2.0f ) return -1;

	vec3 lowerBound = vec3(-2.0f, -2.0f, -2.0f);
	vec3 upperBound = vec3( 2.0f,  2.0f,  2.0f);

	int x = int(float(pos.x-lowerBound.x)/float(upperBound.x-lowerBound.x)*voxelX);
	int y = int(float(pos.y-lowerBound.y)/float(upperBound.y-lowerBound.y)*voxelY);
	int z = int(float(pos.z-lowerBound.z)/float(upperBound.z-lowerBound.z)*voxelZ);

	return x * voxelY * voxelZ + y * voxelZ + z;
}

vec3 normal(ivec3 index){

	int x = index.x;
	int y = index.y;
	int z = index.z;
	int offset = 0;
	vec3 n = vec3(0.0f);

	float degX, degY, degZ;

	while(length(n) <= 0.1f){
		offset ++;
		degX = imageLoad(voxelData, convertIndex3DToIndex1D(ivec3(x+offset, y, z))).x - imageLoad(voxelData, convertIndex3DToIndex1D(ivec3(x-offset, y, z))).x;
		degY = imageLoad(voxelData, convertIndex3DToIndex1D(ivec3(x, y+offset, z))).x - imageLoad(voxelData, convertIndex3DToIndex1D(ivec3(x, y-offset, z))).x;
		degZ = imageLoad(voxelData, convertIndex3DToIndex1D(ivec3(x, y, z+offset))).x - imageLoad(voxelData, convertIndex3DToIndex1D(ivec3(x, y, z-offset))).x;

		n = normalize(vec3(degX, degY, degZ));
	}
	return n;
}

bool isIsolate(ivec3 index){
	
	float cnt = 0.0f;
	for(int i=-1;i<=1;i++){
		for(int j=-1;j<=1;j++){
			for(int k=-1;k<=1;k++){
				cnt += imageLoad(voxelData, convertIndex3DToIndex1D(index + ivec3(i,j,k))).x;
			}
		}
	}
	return cnt <= 2.0f;
}

void main(){
  uint i = gl_GlobalInvocationID.x;
  
  if(i < maxVoxels + maxParticles)
  {

	if(pass == 0)
	{
		if(i < maxVoxels){
			
			vec4 tmp = imageLoad(voxelData, int(i));
			if(tmp.z >= 0.1f) p[i].deltaCs = vec4(1.0f, 0.0f, 0.0f, 1.0f);
			else p[i].deltaCs = vec4(1.0f, 1.0f, 0.0f, 1.0f);

			imageStore(voxelData, int(i), vec4(p[i].matProp[0]+1.0f, tmp.y, 0.0f, tmp.w));
		}
		return;
	}

	if(p[i].matProp[0] <= 0.01f) return;
    
	// compute density and pressure per particle
    if(pass == 1)
    {
		int cnt = 0;
		float density_sum = 0.f;
		for (uint j = maxVoxels; j < maxVoxels + maxParticles; j++)
		{
			vec3 delta = (p[i].currPos - p[j].currPos).xyz;
			float r = length(delta);
			if (r < SMOOTHING_LENGTH)
			{
				cnt++;
				density_sum += PARTICLE_MASS * /* poly6 kernel */ 315.f * pow(SMOOTHING_LENGTH * SMOOTHING_LENGTH - r * r, 3) / (64.f * PI_FLOAT * pow(SMOOTHING_LENGTH, 9));
			}
		}
		p[i].pamameters[0] = density_sum;
		// compute pressure
		p[i].pamameters[1] = max(PARTICLE_STIFFNESS * (density_sum - PARTICLE_RESTING_DENSITY), 0.f);
		p[i].pamameters[2] = float(cnt); 
    } 

	else if(pass == 2)
    {
		// compute all forces
		vec3 pressure_force = vec3(0);
		vec3 viscosity_force = vec3(0);
		vec3 dCs = vec3(0);
    
		for (uint j = maxVoxels; j < maxVoxels + maxParticles; j++)
		{
			if (i == j)
			{
				continue;
			}
			vec3 delta = (p[i].currPos - p[j].currPos).xyz;
			float r = length(delta);
			if (r < SMOOTHING_LENGTH)
			{
				pressure_force -= PARTICLE_MASS * (p[i].pamameters[1] + p[j].pamameters[1]) / (2.f * p[j].pamameters[0]) *
				// gradient of spiky kernel
                -45.f / (PI_FLOAT * pow(SMOOTHING_LENGTH, 6)) * pow(SMOOTHING_LENGTH - r, 2) * normalize(delta);

				viscosity_force += PARTICLE_MASS * (p[j].velocity - p[i].velocity).xyz / p[j].pamameters[0] *
				// Laplacian of viscosity kernel
                45.f / (PI_FLOAT * pow(SMOOTHING_LENGTH, 6)) * (SMOOTHING_LENGTH - r);

				dCs -= PARTICLE_MASS * pow(SMOOTHING_LENGTH * SMOOTHING_LENGTH - r * r, 2) / p[j].pamameters[0] *
				// Poly6 kernel 
				945.f / (32.f * PI_FLOAT * pow(SMOOTHING_LENGTH, 9)) * delta;
			}
		}
		viscosity_force *= PARTICLE_VISCOSITY;

		p[i].acceleration = vec4((pressure_force / p[i].pamameters[0] + viscosity_force / p[i].pamameters[0] + GRAVITY_FORCE), 1.0f);

		p[i].deltaCs = vec4(normalize(dCs), 1.0f);
	} 

	else if(pass == 3)
    {
		// integrate
		vec3 new_velocity = (p[i].velocity + frameTimeDiff * p[i].acceleration).xyz;
		vec3 new_position = p[i].currPos.xyz + frameTimeDiff * new_velocity;

		// boundary conditions
		
		if (new_position.y < -7)
		{
			new_position.y = -7;
			new_velocity.y *= -1 * WALL_DAMPING;
		}
		else if (new_position.y > 10)
		{
			new_position.y = 10;
			new_velocity.y *= -1 * WALL_DAMPING;
		}
		if (new_position.x < -3)
		{
			new_position.x = -3;
			new_velocity.x *= -1 * WALL_DAMPING;
		}
		else if (new_position.x > 3)
		{
			new_position.x = 3;
			new_velocity.x *= -1 * WALL_DAMPING;
		}
		if (new_position.z < -10)
		{
			new_position.z = -10;
			new_velocity.z *= -1 * WALL_DAMPING;
		}
		else if (new_position.z > 4)
		{
			new_position.z = 4;
			new_velocity.z *= -1 * WALL_DAMPING;
		}

		p[i].velocity = vec4(new_velocity, 1.0f);
		p[i].prevPos = p[i].currPos;
		p[i].currPos = vec4(new_position, 1.0f);

		int index = convertPositionToIndex1D(p[i].currPos.xyz);
		if(index >= 0) {
			vec4 tmp = imageLoad(voxelData, index);
			float hasVoxel = tmp.x;
			if(hasVoxel > 0.1f) {
				vec3 norm = normal(convertIndex1DToIndex3D(index));
				p[i].velocity.xyz = length(p[i].velocity.xyz) * reflect(normalize(p[i].velocity.xyz), norm);
				p[i].currPos.xyz += frameTimeDiff * p[i].velocity.xyz;

				tmp.z = 1.0f;
				if(length(p[i].velocity.xyz) >= 2.0f) tmp.y -= length(p[i].velocity.xyz)/ 500.0f;
				if(tmp.y <= 0.0f) p[index].matProp[0] = -1.0f;

				imageStore(voxelData, index, tmp);
			}
		}
    } 
  }
}