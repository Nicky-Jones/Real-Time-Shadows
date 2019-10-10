
//
// Model a simple light
//

// Ensure matrices are row-major
#pragma pack_matrix(row_major)


//-----------------------------------------------------------------
// Structures and resources
//-----------------------------------------------------------------

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


//
// Textures
//

// Assumes texture bound to texture t0 and sampler bound to sampler s0
Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
SamplerState linearSampler : register(s0);


//-----------------------------------------------------------------
// Pixel Shader - Lighting 
//-----------------------------------------------------------------

FragmentOutputPacket main(FragmentInputPacket v) { 

	FragmentOutputPacket outputFragment;
	bool useTexture = true;

	//float3 T = normalize(v.tangentW);
	//float3 B = normalize(cross(T,N ));
	float4 baseColour = v.matDiffuse;
	if (useTexture)
		baseColour = baseColour * diffuseTexture.Sample(linearSampler, v.texCoord);

	//baseColour = baseColour * diffuseTexture.SampleLevel(linearSampler, v.texCoord, 0.0);
	//float3 Nt = normalize((normalTexture.SampleLevel(linearSampler, v.texCoord, 2.0)*2.0 - 1.0).xyz);
		float3 Nt = normalize((normalTexture.Sample(linearSampler, v.texCoord)*2.0 - 1.0).xyz);


		// Build orthonormal basis. 
		// Transform normals from texture space to world coordinates
		// Add Code Here (Reconstruct float3X3 tangent to world matrix)
		float3 N = normalize(v.normalW);
		float3 T = normalize(v.tangentW - dot(v.tangentW, N)*N);
		float3 B = cross(N, T);
		float3x3 TBN = float3x3(T, B, N);
		// Transform normal to world coordinates
		// Modify Code Here (Transform normal to world coordinates)
		// Transform from tangent space to world space. 
		float3 Nw = mul(Nt, TBN);
		float3 Nn = normalize(Nw);






	float3 colour = 0;
	for (int i = 0; i < 2; i++)
	{
		
		
		float att=1.0;

		// Calculate the lambertian term (essentially the brightness of the surface point based on the dot product of the normal vector with the vector pointing from v to the light source's location)
		float3 lightDir = -light[i].Vec.xyz; // Directional light
		if (light[i].Vec.w == 1.0)// Positional light
		{

			lightDir = light[i].Vec.xyz - v.posW; 
			float dist = length(lightDir);
			float cutOff = light[i].Attenuation.w;
			if (dist > cutOff)
				continue;
			else
			{
				float con = light[i].Attenuation.x; float lin = light[i].Attenuation.y; float quad = light[i].Attenuation.z; 
				att = 1.0 / (con + lin*dist + quad*dist*dist);
			}
				
		}

		//Initialise returned colour to ambient component
		colour += baseColour.xyz* light[i].Ambient*v.matDiffuse;

		lightDir = normalize(lightDir);
		// Add diffuse light if relevant (otherwise we end up just returning the ambient light colour)
		// Add Code Here (Add diffuse light calculation)
		colour += att*(max(dot(lightDir, Nn), 0.0f) *baseColour.xyz * light[i].Diffuse*v.matDiffuse);

		// Calc specular light
		float specPower = max(v.matSpecular.a*1000.0, 1.0f);

		float3 eyeDir = normalize(eyePos - v.posW);
		float3 R = reflect(-lightDir, Nn);

		// Add Code Here (Specular Factor calculation)	
		float specFactor = pow(max(dot(R, eyeDir), 0.0f),  specPower);
		colour += att*specFactor * v.matSpecular.xyz * light[i].Specular.xyz;
		
	}
	outputFragment.fragmentColour = float4(colour, baseColour.a);
	return outputFragment;

}
