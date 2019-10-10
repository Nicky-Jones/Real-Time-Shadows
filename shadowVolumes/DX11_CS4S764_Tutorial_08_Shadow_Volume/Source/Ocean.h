#pragma once
#include <Grid.h>
//#define W_WIDTH 100
//#define W_HEIGHT 100
//#define N_W_IND ((W_WIDTH-1)*2*3)*(W_HEIGHT-1)


class Effect;
class Material;
class Texture;


class Ocean : public Grid {
	ID3D11SamplerState			*cubeMapSampler = nullptr;
	ID3D11ShaderResourceView	*cubeMapTextureResourceView= nullptr;
	ID3D11ShaderResourceView	*dynamicCubeMapTextureResourceView = nullptr;

public:
	Ocean(UINT widthl, UINT heightl, ID3D11Device *device, Effect *_effect, ID3D11ShaderResourceView *_normalMapTexture, ID3D11ShaderResourceView *_cubeMapTexture, Material *_material);
	~Ocean();
	void render(ID3D11DeviceContext *context);
	void setCubeMap(ID3D11ShaderResourceView *_cubeMapTexture){cubeMapTextureResourceView = _cubeMapTexture;};
	void setDynamicCubeMap(ID3D11ShaderResourceView *_dynamiCubeMapTexture){dynamicCubeMapTextureResourceView = _dynamiCubeMapTexture;};
};