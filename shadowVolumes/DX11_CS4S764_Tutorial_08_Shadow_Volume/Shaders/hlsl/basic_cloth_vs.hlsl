
//
// Vertex shader for basic texturing 
//

// Ensure matrices are row-major
#pragma pack_matrix(row_major)
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



// Input vertex
struct VertexInputPacket {

	float3				pos			: POSITION;
	float3				pos_old		: POSITION_OLD;
	float3				normal		: NORMAL;
	float4				matDiffuse	: DIFFUSE;
	float4				matSpecular	: SPECULAR;
	float2				texCoord	: TEXCOORD;
	uint				isFixed		: FIXED;
};


// Output vertex
struct VertexOutputPacket {

	float3				posW		: POSITION; // Vertex in world coords
	float3				normalW		: NORMAL; // Normal in world coords
	float4				matDiffuse	: DIFFUSE;
	float4				matSpecular	: SPECULAR;
	float2				texCoord	: TEXCOORD;
	float4				posH		: SV_POSITION; // Vertex in clip coords
};



//
// Vertex Shader
//
VertexOutputPacket main(VertexInputPacket inputVertex) {

	VertexOutputPacket outputVertex;

	float4 pos = float4(inputVertex.pos, 1.0);
	float4x4 viewProjMatrix = mul(viewMatrix, projMatrix);
	float4x4 wvpMatrix = mul(worldMatrix, viewProjMatrix);

	outputVertex.posW = mul(pos, worldMatrix).xyz;
	outputVertex.posH = mul(pos, wvpMatrix);

	// Multiply the input normal by the inverse-transpose of the world transform matrix
	outputVertex.normalW = mul(float4(inputVertex.normal, 0.0), worldITMatrix).xyz;

	outputVertex.matDiffuse = inputVertex.matDiffuse;
	outputVertex.matSpecular = inputVertex.matSpecular;
	outputVertex.texCoord = inputVertex.texCoord;

	return outputVertex;
}
