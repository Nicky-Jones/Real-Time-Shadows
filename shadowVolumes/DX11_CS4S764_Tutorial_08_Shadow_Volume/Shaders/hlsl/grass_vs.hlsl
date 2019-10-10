
//
// Grass effect - Modified a fur technique
//

// Ensure matrices are row-major
#pragma pack_matrix(row_major)

//-----------------------------------------------------------------
// Globals
//-----------------------------------------------------------------

//cbuffer basicCBuffer : register(b0) {
//
//	float4x4			worldViewProjMatrix;
//	float4x4			worldITMatrix;				// Correctly transform normals to world space
//	float4x4			worldMatrix;
//	float4				eyePos;
//	float4				lightVec;					// w=1: Vec represents position, w=0: Vec  represents direction.
//	float4				lightAmbient;
//	float4				lightDiffuse;
//	float4				lightSpecular;
//	float4				windDir;
//	float				Timer;
//	float				grassHeight;
//};
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
cbuffer sceneCBuffer : register(b3) {
	float4						windDir;
	float						Time;
	float						grassHeight;
	float						deltaTime;
	bool				USE_SHADOW_MAP;
	bool				REFLECTION_PASS;
};

cbuffer shadowCBuffer : register(b5) {
	float4x4			shadowTransformMatrix;
};



//-----------------------------------------------------------------
// Input / Output structures
//-----------------------------------------------------------------
struct vertexInputPacket {

	float3				pos			: POSITION;
	float3				normal		: NORMAL;
	float4				matDiffuse	: DIFFUSE; // a represents alpha.
	float4				matSpecular	: SPECULAR;  // a represents specular power. 
	float2				texCoord	: TEXCOORD;
};


struct vertexOutputPacket {

		// Vertex in world coords
		float3				posW			: POSITION;
		// Normal in world coords
		float3				normalW			: NORMAL;
		float4				matDiffuse		: DIFFUSE; // a represents alpha.
		float4				matSpecular		: SPECULAR; // a represents specular power. 
		float2				texCoord		: TEXCOORD;
		float4				posSdw			: SHADOW;
		float4				posH			: SV_POSITION;
	};


//Texture2D heightTexture : register(t0);
//Texture2D normalTexture : register(t1);

//-----------------------------------------------------------------
// Vertex Shader
//-----------------------------------------------------------------
vertexOutputPacket main(vertexInputPacket inputVertex) {
	float4x4 worldViewProjMatrix = mul(worldMatrix, mul(viewMatrix, projMatrix));

	vertexOutputPacket outputVertex;
	float4 pos = float4(inputVertex.pos,1);
	float4 posW = mul(pos, worldMatrix);
	// Vertex position in Light coordinates
	outputVertex.posSdw = mul(posW, shadowTransformMatrix);
	// Lighting is calculated in world space.
	outputVertex.posW =posW.xyz;
	
	// Transform normals to world space with gWorldIT.
	outputVertex.normalW = mul(float4(inputVertex.normal, 1.0f), worldITMatrix).xyz;
	//outputVertex.normalW = (normalTexture.Load(int3(inputVertex.texCoord.x * 1024, inputVertex.texCoord.y * 1024, 0)).xyz*2-1);
	
	// Pass through material properties
	outputVertex.matDiffuse = inputVertex.matDiffuse;
	outputVertex.matSpecular = inputVertex.matSpecular;
	// .. and texture coordinates.
	outputVertex.texCoord = inputVertex.texCoord;
	
	
	// Finally transform/project pos to screen/clip space posH
	pos.y += grassHeight*8;
	float k = pow(grassHeight*50, 2);
	//float3 gWindDir = float3(sin(Time)*0.05, 0, 0);
	float3 gWindDir = float3((sin(Time) + 1)*0.1, 0, 0);
	pos.xyz = pos.xyz + gWindDir*k;
	outputVertex.posH = mul(pos, worldViewProjMatrix);

	return outputVertex;
}
