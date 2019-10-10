
//
// Grass effect - Modified a fur technique
//

// Ensure matrices are row-major
#pragma pack_matrix(row_major)


//-----------------------------------------------------------------
// Structures and resources
//-----------------------------------------------------------------

//-----------------------------------------------------------------
//// Globals
////-----------------------------------------------------------------



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
	bool				USE_SHADOW_MAP;
	bool				REFLECTION_PASS;
}

//
// Textures
//


Texture2D myTexture : register(t0);
Texture2D grassAlpha: register(t1);
Texture2D shadowMap: register(t2);
SamplerState anisotropicSampler : register(s0);
SamplerComparisonState SamShadow : register(s1);




//-----------------------------------------------------------------
// Input / Output structures
//-----------------------------------------------------------------


// Input fragment - this is the per-fragment packet interpolated by the rasteriser stage
struct FragmentInputPacket {
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


struct FragmentOutputPacket {

	float4				fragmentColour : SV_TARGET;
};



float3 lambertian(float3 baseColour, float3 N, float3 posW, float specMaterialAmount, float3 specMaterialColour)
{
	// Calculate the lambertian term (essentially the brightness of the surface point based on the dot product of the normal vector with the vector pointing from v to the light source's location)
	float3 lightDir = -lightVec.xyz; // Directional light
	if (lightVec.w == 1.0) lightDir = lightVec.xyz -posW; // Positional light
	lightDir = normalize(lightDir);
	float3 colour = baseColour*max(dot(lightDir, N), 0.0f)*lightDiffuse;

	// Calculate the specular term
	float specPower = max(specMaterialAmount*1000.0, 1.0f);
	float3 eyeDir = normalize(eyePos.xyz - posW);
	float3 R = reflect(-lightDir, N);
	float specFactor = pow(max(dot(R, eyeDir), 0.0f), specPower);
	colour += baseColour*specFactor * specMaterialColour;
	return colour;
}
//-----------------------------------------------------------------
// Pixel Shader - Lighting 
//-----------------------------------------------------------------
float unproject(float zBuf)
{
	float f = 1000.0;
	float n = 1.0;
	//return f*n / (f - zBuf*(f + n));
	return (2.0 * n) / (f + n - zBuf * (f - n));
}
FragmentOutputPacket main(FragmentInputPacket v) {

	float maxHeight = 0.005;
	float grassStage = (1.0 / maxHeight)*grassHeight;//0...1
	float tileRepeat = 64;

	FragmentOutputPacket outputFragment;

	float alpha = 1.0;
	float3 N = normalize(v.normalW);
	float3 baseColour = myTexture.Sample(anisotropicSampler, v.texCoord)*v.matDiffuse.xyz;

	//Fade from grass to dry mud and then to wet mud
	float upperFadeStart = 0.5;//-0.2;
		float lowerFadeEnd = -0.2;//-0.95;
	float fadeRange = upperFadeStart - lowerFadeEnd;

	float3  mudcolour = float3(222, 171, 106)/100.0;// / float3(159, 125, 98) / 255.0;
	mudcolour *= (baseColour.r + baseColour.g + baseColour.b) / 3.0;// greyscale of grass to get consistent texture

	if (REFLECTION_PASS)
		clip(v.posW.y < -1.0f ? -1 : 1);


	if (v.posW.y < upperFadeStart)
	{
		if (v.posW.y > lowerFadeEnd)
		{
			//fade from grass to mud
			float w = (v.posW.y - lowerFadeEnd) / fadeRange;
			baseColour = (baseColour*w) + (mudcolour*(1 - w));
		}
		else baseColour = mudcolour * 0.6;// wet mudcolour is darker;
	}

	// Apply lighting

	// Apply ambient lighting
	float3 colour = baseColour*lightAmbient.xyz;

	
	//Test if this pixel is in shadow#
	float3 lamb = lambertian(baseColour, N, v.posW, v.matSpecular.a, v.matSpecular.xyz);


	if (USE_SHADOW_MAP)
	{
		v.posSdw.xyz /= v.posSdw.w;// Complete projection by doing division by w.
		// Check if pixel is outside of the bounds of the shadowmap
		if (v.posSdw.x < 1 && v.posSdw.x>0 && v.posSdw.y < 1 && v.posSdw.y>0)
		{
			float percentLit = 1.0;
			percentLit = shadowMap.SampleCmp(SamShadow, v.posSdw.xy, v.posSdw.z);
			//float depthShadowMap = shadowMap.Sample(anisotropicSampler, v.posSdw.xy).r;
			
			
			//outputFragment.fragmentColour = float4(1,0,0, 1);
			//return outputFragment;
			
			
			//if (v.posSdw.z >= (depthShadowMap)) percentLit = 0.0;

			lamb *= min(percentLit + 0.5, 1.0);
		}
	}
	colour += lamb;


	//if (USE_SHADOW_MAP)
	//{
	//	v.posSdw.xyz /= v.posSdw.w;// Complete projection by doing division by w.
	//	// Check if pixel is outside of the bounds of the shadowmap
	//	if (v.posSdw.x < 1 && v.posSdw.x>0 && v.posSdw.y < 1 && v.posSdw.y>0)
	//	{
	//		float depthShadowMap = shadowMap.Sample(anisotropicSampler, v.posSdw.xy).r;
	//		// Test if pixel depth is >= the value in the shadowmap(i.e behind/obscured) 
	//		if (v.posSdw.z >= (depthShadowMap)){//add small bias to avoid z fighting
	//			// If this pixel is behind the value in the shadowmap it is in shadow
	//			// so return ambient only. If not complete full lighting calculation 


	//		float percentLit = 1.0;
	//		percentLit = shadowMap.SampleCmp(SamShadow, v.posSdw.xy, v.posSdw.z);
	//		float depthShadowMap = shadowMap.Sample(anisotropicSampler, v.posSdw.xy).r;
	//		//if (v.posSdw.z >= (depthShadowMap)) percentLit = 0.0;

	//		lamb *= min(percentLit + 0.5, 1.0);



	//			colour *= 1.7;

	//		}
	//		else
	//			colour += lambertian(baseColour, N, v.posW, v.matSpecular.a, v.matSpecular.xyz);
	//	}
	//	else
	//		colour += lambertian(baseColour, N, v.posW, v.matSpecular.a, v.matSpecular.xyz);
	//}
	//else
	//	colour += lambertian(baseColour, N, v.posW, v.matSpecular.a, v.matSpecular.xyz);


	float distanceFadeMaxEnd = 10;
	float distanceFadeMinStart = 5;
	float distanceFadeRange = distanceFadeMaxEnd - distanceFadeMinStart;
	float distance = length(eyePos.xyz - v.posW);

	// only render grass that is less than distanceFadeMaxEnd
	if (grassStage > 0.0 && distance < distanceFadeMaxEnd &&v.posW.y > lowerFadeEnd)
	{
		alpha = grassAlpha.Sample(anisotropicSampler, v.texCoord*tileRepeat).a;
		// Alpha Test
		// clip(alpha < 0.9f ? -1 : 1);// clip for alpha < 0.9

		// Reduce alpha
		alpha *= (1.0 - grassStage);

		// and increase illumination for tips of grass
		colour *= (grassStage + 1.0);

		//fade out grass that is below upperFadeStart
		if (v.posW.y - lowerFadeEnd)
		{
			float w = (v.posW.y - lowerFadeEnd) / fadeRange;
			alpha = lerp(0, alpha, w);
		}

		//fade out grass that is further than distanceFadeMinStart and less than distanceFadeMaxEnd
		if (distance > distanceFadeMinStart)
		{
			float w = (distance - distanceFadeMinStart) / distanceFadeRange;
			alpha = lerp( alpha,0, w);
		}
	}


	outputFragment.fragmentColour = float4(colour,alpha);
	return outputFragment;

}
