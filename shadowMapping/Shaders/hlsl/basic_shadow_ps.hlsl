
//
// Pixel shader for basic texturing with a point light source
//

// Ensure matrices are row-major
#pragma pack_matrix(row_major)


// Structures





Texture2D myTexture : register(t0);
SamplerState textureSampler : register(s0);



// Input fragment
struct FragmentInputPacket {


	float4				posH			: SV_POSITION;
};


struct FragmentOutputPacket {

	float4				fragmentColour : SV_TARGET;
};






FragmentOutputPacket main(FragmentInputPacket inputFragment) {

	FragmentOutputPacket outputFragment;

	outputFragment.fragmentColour=float4(0.0, 0.0, 0.0, 1.0);

	return outputFragment;
}
