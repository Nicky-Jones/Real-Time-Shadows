
//
// Vertex shader for basic texturing 
//

// Ensure matrices are row-major
#pragma pack_matrix(row_major)

// Resources



struct Light {
	float4				Vec; // w=1: Vec represents position, w=0: Vec  represents direction.
	float4				Ambient;
	float4				Diffuse;
	float4				Specular;
	float4				Attenuation;// x=constant,y=linear,z=quadratic,w=cutOff
};
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
	Light light[2];
};



// Input vertex
struct VertexInputPacket {

	float3				pos			: POSITION;
};


// Output vertex
struct VertexOutputPacket {


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


	outputVertex.posH = mul(pos, wvpMatrix);


	return outputVertex;
}
