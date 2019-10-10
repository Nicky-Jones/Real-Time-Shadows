

// Ensure matrices are row-major
#pragma pack_matrix(row_major)
#define MAX_BONES 120
//-----------------------------------------------------------------
// Globals
//-----------------------------------------------------------------

cbuffer modelCBuffer : register(b0) {
	float4x4			worldMatrix;
	float4x4			worldITMatrix; // Correctly transform normals to world space
};
cbuffer cameraCbuffer : register(b1) {
	float4x4			viewMatrix;
	float4x4			projMatrix;
	float4				eyePos;
}
cbuffer lightCBuffer : register(b2) {
	float4				lightVec; // w=1: Vec represents position, w=0: Vec  represents direction.
	float4				lightAmbient;
	float4				lightDiffuse;
	float4				lightSpecular;
};

cbuffer boneCBuffer : register(b5) {
	float4x4 bones[MAX_BONES];
};
//-----------------------------------------------------------------
// Input / Output structures
//-----------------------------------------------------------------
struct vertexInputPacket {

	float3				pos			: POSITION;
	float3				normal		: NORMAL;
	float3				tangent		: TANGENT;
	float4				matDiffuse	: DIFFUSE; // a represents alpha.
	float4				matSpecular	: SPECULAR;  // a represents specular power. 
	float2				texCoord	: TEXCOORD;
	float4				weights		: WEIGHTS;
	int4				indices		: INDICES;
	float4				weights2		: WEIGHTSX;
	int4				indices2		: INDICESX;
	float4				weights3		: WEIGHTSY;
	int4				indices3		: INDICESY;
};


struct vertexOutputPacket {


	// Vertex in world coords
	float3				posW			: POSITION;
	// Normal in world coords
	float3				normalW			: NORMAL;
	float3				tangentW		: TANGENT;
	float4				matDiffuse		: DIFFUSE;
	float4				matSpecular		: SPECULAR;
	float2				texCoord		: TEXCOORD;
	float4				posH			: SV_POSITION;
};


//-----------------------------------------------------------------
// Vertex Shader
//-----------------------------------------------------------------
vertexOutputPacket main(vertexInputPacket inputVertex) {

	float4x4 WVP = mul(worldMatrix, mul(viewMatrix,projMatrix));
	
	vertexOutputPacket outputVertex;
	
	
	float3 posL = 0;
	float3 normL = 0;
	float3 tangL = 0;

		float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		weights[0] = inputVertex.weights.x;
		weights[1] = inputVertex.weights.y;
		weights[2] = inputVertex.weights.z;
		weights[3] = inputVertex.weights.w;
		for (int i = 0; i < 4; i++)
		{
			// Assume no nonuniform scaling when transforming normals, so
			// that we do not have to use the inverse-transpose.
			if (inputVertex.indices[i] >= 0)
			{
				posL += weights[i] * mul(float4(inputVertex.pos, 1.0f), bones[inputVertex.indices[i]]).xyz;
				normL += weights[i] * mul(inputVertex.normal, (float3x3)bones[inputVertex.indices[i]]);
				tangL += weights[i] * mul(inputVertex.tangent, (float3x3)bones[inputVertex.indices[i]]);
			}
		}
		weights[0] = inputVertex.weights2.x;
		weights[1] = inputVertex.weights2.y;
		weights[2] = inputVertex.weights2.z;
		weights[3] = inputVertex.weights2.w;

		for (int i = 0; i < 4; i++)
		{
			if (inputVertex.indices2[i] >= 0)
			{
				posL += weights[i] * mul(float4(inputVertex.pos, 1.0f), bones[inputVertex.indices2[i]]).xyz;
				normL += weights[i] * mul(inputVertex.normal, (float3x3)bones[inputVertex.indices2[i]]);
				tangL += weights[i] * mul(inputVertex.tangent, (float3x3)bones[inputVertex.indices2[i]]);
			}
		}
		weights[0] = inputVertex.weights3.x;
		weights[1] = inputVertex.weights3.y;
		weights[2] = inputVertex.weights3.z;
		weights[3] = inputVertex.weights3.w;
		for (int i = 0; i < 4; i++)
		{

			if (inputVertex.indices3[i] >= 0)
			{
				posL += weights[i] * mul(float4(inputVertex.pos, 1.0f), bones[inputVertex.indices3[i]]).xyz;
				normL += weights[i] * mul(inputVertex.normal, (float3x3)bones[inputVertex.indices3[i]]);
				tangL += weights[i] * mul(inputVertex.tangent, (float3x3)bones[inputVertex.indices3[i]]);
			}
		}

	//}


	// Lighting is calculated in world space.
	//Add Code Here(Transform vertex position to world coordinates)
	outputVertex.posW = mul(float4(posL, 1.0f), worldMatrix);
	// Transform normals to world space with gWorldIT.
	outputVertex.normalW = mul(normL, (float3x3)worldITMatrix);
	outputVertex.tangentW = mul(tangL, (float3x3)worldITMatrix);
	// Pass through material properties
	//int bone23 = 0;
	//if (inputVertex.indices.x == 23 | inputVertex.indices.y == 23 |  inputVertex.indices.z == 23 | inputVertex.indices.w == 23)
	//	bone23 = 1;
	//outputVertex.matDiffuse = float4(inputVertex.weights.x, inputVertex.weights.x + inputVertex.weights.y + inputVertex.weights.z + inputVertex.weights.w, bone23, 1);
	//outputVertex.matDiffuse = float4(inputVertex.weights.y, 0, 0, 1);
	outputVertex.matDiffuse = inputVertex.matDiffuse;
	outputVertex.matSpecular = inputVertex.matSpecular;
	// .. and texture coordinates.
	outputVertex.texCoord = inputVertex.texCoord;
	// Finally transform/project pos to screen/clip space posH
	outputVertex.posH = mul(float4(posL, 1.0f), WVP);

	return outputVertex;
}
