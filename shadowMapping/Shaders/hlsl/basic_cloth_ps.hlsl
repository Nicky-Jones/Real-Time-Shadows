
//
// Pixel shader for basic texturing with a point light source
//

// Ensure matrices are row-major
#pragma pack_matrix(row_major)


// Structures


//// Basic point light model
//struct DXPointLight {
//
//	float3				pos;
//	float				range; // Radius of sphere in which light applied
//	float3				colour;
//	float3				attenuation; // (x, y, z) -> (constant, linear, quadratic) attenuation
//};
//
// Model a given surface point from the fragment shader to pass to lighting calculation functions.  The surface point is modelled in world coordinate space.
struct SurfacePointW {

	float3				pos;
	float3				normal;
	float2				texCoord;
	float4				matDiffuse;
	float4				matSpecular;
};
//
//
//
//// Resources
//
//cbuffer Camera : register(b0) {
//
//	float4x4			viewProjMatrix;
//	float4				eyePos;
//};
//
//cbuffer PointLightModel : register(b1) {
//
//	DXPointLight		pointLightSource;
//};
//-----------------------------------------------------------------
// Structures and resources
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Globals
//-----------------------------------------------------------------
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

Texture2D myTexture : register(t0);
SamplerState textureSampler : register(s0);



// Input fragment
struct FragmentInputPacket {

	float3				pos			: POSITION; // Vertex in world coords
	float3				normal			: NORMAL; // Normal in world coords
	float4				matDiffuse		: DIFFUSE;
	float4				matSpecular		: SPECULAR;
	float2				texCoord		: TEXCOORD;
	float4				posH			: SV_POSITION;
};

//struct FragmentInputPacket {
//	float3				pos			: POSITION;
//	float3				pos_old		: POSITION_OLD;
//	float3				normal		: NORMAL;
//	float4				matDiffuse	: DIFFUSE;
//	float4				matSpecular	: SPECULAR;
//	float2				texCoord	: TEXCOORD;
//	uint				isFixed		: FIXED;
//};
struct FragmentOutputPacket {

	float4				fragmentColour : SV_TARGET;
};




//
// Lighting functions
//

// Calculate colour / brightness at surface point P given light source L with the specular term calculated wrt. the camera's position in world coordinates represented by 'eyePos'
float4 pointLight(inout SurfacePointW P, in Light L, in float3 eyePos) {

	// Initialise returned colour to ambient component (0.0, 0.0, 0.0) for a localied point light source
	float3 colour = myTexture.Sample(textureSampler, P.texCoord).rgb *float3(0.3, 0.3, 0.3);

		float3 lightVec = L.Vec.xyz - P.pos;

		float d = length(lightVec);

	//if (d > L.range)
	//	return float4(colour, P.matDiffuse.a);

	lightVec /= d;

	// Calculate the lambertian term (dot product)
	float dp = dot(lightVec, P.normal);

	// Evalulate (dp > 0) and implicit (dp <= 0) branch and DX will select result based on initial value of dp.  This avoids issues where M.diffuseMaterialColour is not called in some branches and can lead to unwanted shading artefacts.
	[flatten]
	if (dp > 0.0f)
	{
		// Add diffuse light if relevant (otherwise we end up just returning the ambient light colour)
		float3 _colour = max(dp, 0.0f) * myTexture.Sample(textureSampler, P.texCoord).rgb * L.Diffuse;

			// Calc specular light
			float specPower = max(P.matSpecular.a, 1.0);
		float3 toEye = normalize(eyePos - P.pos);
			float3 R = reflect(-lightVec, P.normal);
			float specFactor = pow(max(dot(R, toEye), 0.0f), specPower);

		_colour += specFactor * P.matSpecular.rgb * L.Specular;

		// Add in attenuation
		_colour /= dot(L.Attenuation, float3(1.0f, d, d*d));

		colour += _colour;
	}

	// Return colour with diffuse.a set as the alpha component
	return float4(colour, P.matDiffuse.a);
}



FragmentOutputPacket main(FragmentInputPacket inputFragment) {

	FragmentOutputPacket outputFragment;

	SurfacePointW P = {
		inputFragment.pos,
		normalize(inputFragment.normal),
		inputFragment.texCoord,
		inputFragment.matDiffuse,
		inputFragment.matSpecular
	};

	outputFragment.fragmentColour = pointLight(P, light[0], eyePos);
	//outputFragment.fragmentColour = myTexture.Sample(textureSampler, P.texCoord).rgba;
	return outputFragment;
}
