#pragma pack_matrix(row_major)

cbuffer modelCBuffer : register(b0) {
	float4x4			worldMatrix;
	float4x4			worldITMatrix; // Correctly transform normals to world space
};



//----------------------------
// Input / Output structures
//----------------------------
struct vertexInputPacket {

	float3				pos			: POSITION;
	float4				colour		: COLOR;
};

struct vertexOutputPacket {

	float2				texCoord	: TEXCOORD;
	float4				posH		: SV_POSITION;

};

//
// Vertex shader
//
vertexOutputPacket main(vertexInputPacket inputVertex) {

	vertexOutputPacket outputVertex;

	outputVertex.texCoord = inputVertex.pos.xy*0.5+0.5;
	outputVertex.texCoord.y*=-1;
	outputVertex.posH = mul(float4(inputVertex.pos, 1.0), worldMatrix); //float4(inputVertex.pos, 1.0);

	return outputVertex;
}
