#version 430

in vec2 ex_TexCoor;
in float ex_Neighbor;
in vec4 ex_DeltaCs;
in float ex_Type;

out vec4 color;

uniform float time;
uniform bool showParticles;

uniform bool showMaterial;

vec3 CalcParaLight(vec3 lightDir)
{
    vec3 paraLightDir = normalize(lightDir);
    float diff = max(dot(ex_DeltaCs.xyz, paraLightDir), 0.0);

    vec3 ambient  = vec3(0.7f, 1.0f, 0.7f)  * 0.4f;
    vec3 diffuse  = vec3(1.0f, 1.0f, 1.0f)  * diff * 0.6f;

    return (ambient + diffuse);
}

vec3 CalcParaLightForMat(vec3 lightDir)
{
    vec3 paraLightDir = normalize(lightDir);
    float diff = max(dot(ex_DeltaCs.xyz, paraLightDir), 0.0);

    vec3 ambient  = vec3(0.8f, 0.4f, 0.2f)  * 0.4f;
    vec3 diffuse  = vec3(1.0f, 1.0f, 1.0f)  * diff * 0.6f;

    return (ambient + diffuse);
}

void main(void){
	
	if(ex_Type < 0.0f) discard;

	if (distance(ex_TexCoor, vec2(0.5, 0.5)) > 0.5) {
        discard;
    }

	if(ex_Type < 0.5f) {
		if(showMaterial){
			color = vec4(CalcParaLightForMat(vec3(-2.0f, -1.0f, 1.0f)), 1.0f);
		}

		else{
			if(color.a == 0.0f) color = vec4(1.0f, 1.0f, 0.0f, 1.0f);
			else color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
		}
		return;
	}
	
	if(showParticles == false) discard;

	if(ex_Neighbor < 6)
	{
		color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		color = vec4(CalcParaLight(vec3(2.0f, -1.0f, 1.0f)), 1.0f);
	}
	
}