
// Basic texture pixel shader

// input fragment - this is the per-fragment packet interpolated by the rasteriser stage
struct fragmentInputPacket {

	float2				texCoord	: TEXCOORD;
};


struct fragmentOutputPacket {

	float4				fragmentColour : SV_TARGET;
};

Texture2D inputTex : register(t0);
Texture2D inputTex2 : register(t1);
SamplerState linearSampler : register(s0);

float unproject(float zBuf)
{
	float f = 1000.0;
	float n = 1.0;
	//return f*n / (f - zBuf*(f + n));
	return (2.0 * n) / (f + n - zBuf * (f - n));
}

fragmentOutputPacket main(fragmentInputPacket inputFragment) {

	fragmentOutputPacket outputFragment;
	float depth =1.0 - unproject(inputTex.Sample(linearSampler, inputFragment.texCoord).r);
	outputFragment.fragmentColour = float4(depth,depth,depth,1.0);
	return outputFragment;
}
