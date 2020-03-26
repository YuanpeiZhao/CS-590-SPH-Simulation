#version 430

layout(location=0) in vec4 currentPos;
layout(location=1) in float nCount;
layout(location=2) in vec4 dCs;
layout(location=3) in float particleType;

out float vNeighbor;
out vec3 vDeltaCs;
out float vType;

void main(void){
	vType = particleType;
	vNeighbor = nCount;
	vDeltaCs = dCs.xyz;
	gl_Position = currentPos;
	//gl_PointSize = 40.0f;
}