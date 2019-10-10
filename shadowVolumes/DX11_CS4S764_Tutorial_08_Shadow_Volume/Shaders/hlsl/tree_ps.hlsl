
//
// Model a simple light
//

// Ensure matrices are row-major
#pragma pack_matrix(row_major)

//-----------------------------------------------------------------
// Globals
//-----------------------------------------------------------------
struct Light{
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


//-----------------------------------------------------------------
// Input / Output structures
//-----------------------------------------------------------------

// Input fragment - this is the per-fragment packet interpolated by the rasteriser stage
struct FragmentInputPacket {

	// Vertex in world coords
	float3				posW			: POSITION;
	// Normal in world coords
	float3				normalW			: NORMAL;
	float3				tangentW		: TANGENT;
	float4				matDiffuse		: DIFFUSE; // a represents alpha.
	float4				matSpecular		: SPECULAR; // a represents specular power. 
	float2				texCoord		: TEXCOORD;
	float4				posH			: SV_POSITION;
};



struct FragmentOutputPacket {

	float4				fragmentColour : SV_TARGET;
};

Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
SamplerState linearSampler : register(s0);
//-----------------------------------------------------------------
// Pixel Shader - Lighting 
//-----------------------------------------------------------------

FragmentOutputPacket main(FragmentInputPacket v) { 


	FragmentOutputPacket outputFragment;

	float3 N = normalize(v.normalW);
	float4 baseColour = v.matDiffuse;
	baseColour = baseColour*diffuseTexture.Sample(linearSampler, v.texCoord);
	//baseColour = baseColour*diffuseTexture.SampleLevel(linearSampler, v.texCoord, 9);
	//baseColour *= myTexture2.Sample(linearSampler, v.texCoord);
	// Alpha Test
	// Add Code Here (Clip for alpha < 0.9)

	//clip()
	//Initialise returned colour to ambient component
	float3 colour = baseColour.xyz* light[0].Ambient;

	// Calculate the lambertian term (essentially the brightness of the surface point based on the dot product of the normal vector with the vector pointing from v to the light source's location)
	float3 lightDir = -light[0].Vec.xyz; // Directional light
	if (light[0].Vec.w == 1.0) lightDir = light[0].Vec.xyz - v.posW; // Positional light
	lightDir=normalize(lightDir);

	// Add diffuse light if relevant (otherwise we end up just returning the ambient light colour)
	colour += max(dot(lightDir, N), 0.0f) *baseColour.xyz * light[0].Diffuse;

	// Calc specular light
	float specPower = max(v.matSpecular.a*1000.0, 1.0f);
	float3 eyeDir = normalize(eyePos - v.posW);
	float3 R = reflect(-lightDir,N );
	float specFactor = pow(max(dot(R, eyeDir), 0.0f), specPower);
	colour += specFactor * v.matSpecular.xyz * light[0].Specular;

	outputFragment.fragmentColour = float4(colour, baseColour.a);

	return outputFragment;

}
