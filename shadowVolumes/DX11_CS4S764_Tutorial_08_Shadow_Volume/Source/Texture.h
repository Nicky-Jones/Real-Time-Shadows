#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <d3d11_2.h>



class Texture
{
	// Direct3D scene textures and resource views
	ID3D11Texture2D							*texture = nullptr;
	ID3D11ShaderResourceView				*SRV = nullptr;
	ID3D11DepthStencilView					*DSV = nullptr;
	ID3D11RenderTargetView					*RTV = nullptr;
public:
	ID3D11Texture2D	*getTexture(){ return texture; };
	Texture(ID3D11Device *device, const std::wstring& filename);
	Texture(ID3D11Device *device, ID3D11DeviceContext *context, const std::wstring& filename);
	ID3D11ShaderResourceView *getShaderResourceView(){ return SRV; };
	~Texture();
};

