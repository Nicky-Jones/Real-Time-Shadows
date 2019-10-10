
#include <stdafx.h>
#include <Ocean.h>
#include <VertexStructures.h>
#include <Effect.h>

using namespace std;


//Ocean::Ocean(UINT widthl, UINT heightl, ID3D11Device *device, Effect *_effect, ID3D11ShaderResourceView *_normalMapTexture, ID3D11ShaderResourceView *_cubeMapTexture, Material *_material) :Grid(widthl, heightl, device, _effect, _normalMapTexture, _material) {
//
//	try
//	{
//		cubeMapTextureResourceView = _cubeMapTexture;
//
//		D3D11_SAMPLER_DESC samplerDesc;
//		ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
//		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
//		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
//		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
//		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
//		samplerDesc.MinLOD = 0.0f;
//		samplerDesc.MaxLOD = 0.0f;
//		samplerDesc.MipLODBias = 0.0f;
//		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
//		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
//		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
//		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
//		HRESULT hr = device->CreateSamplerState(&samplerDesc, &cubeMapSampler);
//	}
//	catch (exception& e)
//	{
//		cout << "Ocean object could not be instantiated due to:\n";
//		cout << e.what() << endl;
//
//		if (vertexBuffer)
//			vertexBuffer->Release();
//
//		//if (inputLayout)
//		//	inputLayout->Release();
//
//		vertexBuffer = nullptr;
//		//inputLayout = nullptr;
//		indexBuffer = nullptr;
//	}
//}


Ocean::~Ocean() {
	if (vertexBuffer)
		vertexBuffer->Release();
	if (indexBuffer)
		indexBuffer->Release();

	if (cubeMapSampler)
		cubeMapSampler->Release();
}


void Ocean::render(ID3D11DeviceContext *context) {
	// Bind additional Ocean texture resource views and texture sampler object to the pipeline
	if (cubeMapTextureResourceView && dynamicCubeMapTextureResourceView && cubeMapSampler) 
	{
		context->PSSetShaderResources(1, 1, &cubeMapTextureResourceView);
		context->PSSetShaderResources(2, 1, &dynamicCubeMapTextureResourceView);
		context->PSSetSamplers(1, 1, &cubeMapSampler);
	}
	Grid::render(context);
	ID3D11ShaderResourceView* nosrvs[1] = { 0 }; //unbind the dynamic srv...
	context->PSSetShaderResources(2, 1, nosrvs);
}

