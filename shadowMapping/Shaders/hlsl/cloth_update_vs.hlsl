
//
// Pass-through vertex shader.  
//


// Input / Output structures

struct ThroughVertexPacket {

	float3				pos			: POSITION;
	float3				pos_old		: POSITION_OLD;
	float3				normal		: NORMAL;
	float4				matDiffuse	: DIFFUSE;
	float4				matSpecular	: SPECULAR;
	float2				texCoord	: TEXCOORD;
	uint				isFixed		: FIXED;
};



//
// Vertex Shader
//

ThroughVertexPacket main(ThroughVertexPacket inputVertex) {

	return inputVertex;
}
