
//
// Model the update of each particle.  Each particle is modelled in world coordinate space
//


// Resources

//cbuffer GameTime : register (b0) {
//
//	float		gameTimeDelta;
//};
//
//cbuffer ClothUpdateConstants : register (b1) {
//
//	float4		gravity;
//	float4		wind;
//	uint		reCalcNormals;
//};


// Input / Output structures

// For the update process, the geometry shader (GS) inputs and outputs the same vertex structure
struct ParticleStructure {
	float3				pos			: POSITION;
	float3				pos_old		: POSITION_OLD;
	float3				normal		: NORMAL;
	float4				matDiffuse	: DIFFUSE;
	float4				matSpecular	: SPECULAR;
	float2				texCoord	: TEXCOORD;
	uint				isFixed		: FIXED;
};




float3 updateSpring(float3 P0, float3 P1, float targetDistance)
{

	//Apply spring constraints
	float3 delta = P1 - P0;

	//Ensure distance is not equal to zero
	float dist = max(length(delta), 1e-7);
	float stretching = 1 - targetDistance / dist;
	delta = stretching * delta;
	if (max(length(delta), 1e-7) != 1e-7)
		P0 += (0.4 * delta);
	return P0;
}

//One particle in maps to 1 particle out
//The order of the particles must be maintained so that the index buffers will match up when for subsequent passes
[maxvertexcount(1)]
void main(triangle ParticleStructure inputParticle[3],
	inout PointStream<ParticleStructure> outputParticleStream) {

	uint		reCalcNormals=1;

	ParticleStructure			outputParticle;

	outputParticle = inputParticle[0];


	//position
	if (inputParticle[0].isFixed == 0)    //particle not fixed so apply spring constraints
	{
		outputParticle.pos = updateSpring(outputParticle.pos, inputParticle[1].pos, 0.5);
	}

	//normal
	if (reCalcNormals == 1)
		outputParticle.normal = float3(0, 0, 0);

	//if (max(length(inputParticle[0].pos - inputParticle[1].pos), 1e-7) != 1e-7&max(length(inputParticle[0].pos - inputParticle[2].pos), 1e-7) != 1e-7)
	//{
		outputParticle.normal= (cross(inputParticle[1].pos - inputParticle[0].pos, inputParticle[2].pos - inputParticle[0].pos));
	//}
	//float3 U = inputParticle[1].pos - inputParticle[0].pos;
	//float3 V = inputParticle[2].pos - inputParticle[0].pos;
	//float3 N;
	//
	//N.x = U.y*V.z - U.z*V.y;
	//N.y = U.z*V.x - U.x*V.z;
	//N.z = U.x*V.y - U.y*V.x;

	////outputParticle.normal = (cross(inputParticle[1].pos - inputParticle[2].pos, inputParticle[0].pos - inputParticle[2].pos));
	//outputParticle.normal = -N;
	outputParticleStream.Append(outputParticle);
}