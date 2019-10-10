
#include "stdafx.h"
#include "BasicCloth.h"
#include "Effect.h"
#include "Camera.h"
#include <stdio.h>
#include <DirectXTK\DDSTextureLoader.h>
#include <DirectXTK\WICTextureLoader.h>

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

#define CPU 0


//// Mapping lambda
//std::function<ClothUpdateConstants(const DXClothEnvironment_actual * const)> DXClothEnvironment_actual::ClothEnvMap() {
//	return[&](const DXClothEnvironment_actual * const env) {
//		ClothUpdateConstants C;
//		C.gravity = env->Gravity;
//		C.wind = env->Wind;
//		C.reCalcNormals = env->ReCalcNormals;
//		return C;
//	};
//}
//// Public function implementation
//DXClothEnvironment createClothConstants() {
//	auto C = make_shared<DXClothEnvironment_actual>();
//	dxCreateDynamicCBufferMap<DXClothEnvironment_actual, ClothUpdateConstants>(C, DXClothEnvironment_actual::ClothEnvMap());
//	return C;
//}
//




//
//#pragma region DXBasicCloth layout descriptor and interface setup
//HRESULT DXBasicClothVertex::createInputLayout(ID3D11Device *device, DXBlob shaderBlob, UINT iaSlotIndex, ID3D11InputLayout **layout) 
//{
//
//	return device->CreateInputLayout(basicClothVertexDesc, ARRAYSIZE(basicClothVertexDesc), shaderBlob->getBuffer(), shaderBlob->getBufferSize(), layout);
//}

//// SO stream configuration for cloth.  Used to setup geometry shader object.  Fields in each array entry are: output stream index (from GS), semantic name, semantic index (useful if duplicate semantics used), start component, component count, index of SO output buffer slot to write to.
//D3D11_SO_DECLARATION_ENTRY clothSODesc[] = {
//	{ 0, "POSITION", 0, 0, 3, 0 },
//	{ 0, "POSITION_OLD", 0, 0, 3, 0 },
//	{ 0, "NORMAL", 0, 0, 3, 0 },
//	{ 0, "DIFFUSE", 0, 0, 1, 0 },
//	{ 0, "SPECULAR", 0, 0, 1, 0 },
//	{ 0, "TEXCOORD", 0, 0, 2, 0 },
//	{ 0, "FIXED", 0, 0, 1, 0 }
//};
//#pragma endregion




void BasicCloth::load(ID3D11Device *device_in,
	const			DWORD newClothWidth,
	const			DWORD newClothHeight,
	Effect			*updateForcesEffect_in,
	Effect			*updateSpringsEffect_in)
{
	device = device_in;
	//ID3D11DeviceContext *context = dxDeviceContext();
	updateForcesEffect= updateForcesEffect_in;
	ID3D11DepthStencilState *disabledDepthStencilState = updateForcesEffect->getDepthStencilState();
	D3D11_DEPTH_STENCIL_DESC disabledDSStateDesc; disabledDepthStencilState->GetDesc(&disabledDSStateDesc);
	disabledDSStateDesc.DepthEnable = FALSE;
	disabledDSStateDesc.StencilEnable = FALSE;
	device->CreateDepthStencilState(&disabledDSStateDesc, &disabledDepthStencilState);
	updateForcesEffect->setDepthStencilState(disabledDepthStencilState);

	updateSpringsEffect= updateSpringsEffect_in;
	updateSpringsEffect->setDepthStencilState(disabledDepthStencilState);
	//renderEffect= renderPipeline_in;
	// Setup cloth

	clothWidth = newClothWidth;
	clothHeight = newClothHeight;


	//clothVertexLayout = effect->getVSInputLayout();
	
	//createDefaultLinearSampler(device);

	initCBuffer(device);


	//clothTextureSRV = clothTextureSRV_in;


	// Setup fixed function pipeline stages

	auto rasteriserState = effect->getRasterizerState();
	D3D11_RASTERIZER_DESC RSdesc = {};
	rasteriserState->GetDesc(&RSdesc);
	
	// Setup rasteriser state
	//RSdesc.FillMode = D3D11_FILL_SOLID;
	RSdesc.CullMode = D3D11_CULL_BACK;
	RSdesc.FrontCounterClockwise = FALSE;
	//RSdesc.DepthBias = 0;
	//RSdesc.SlopeScaledDepthBias = 0.0f;
	//RSdesc.DepthBiasClamp = 0.0f;
	//RSdesc.DepthClipEnable = TRUE;
	//RSdesc.ScissorEnable = FALSE;
	//RSdesc.MultisampleEnable = FALSE;
	//RSdesc.AntialiasedLineEnable = FALSE;

	device->CreateRasterizerState(&RSdesc,  &rasteriserState);
	effect->setRasterizerState(rasteriserState);




	// Initialise cloth buffers
	initialiseClothBuffers(device);


	//// Create vertex / particle layout object
	//hr = DXBasicClothVertex::createInputLayout(device, clothVS->getBytecode(), 0, &clothVertexLayout);
	
	//// Setup cloth constants
	//clothEnvConstants = createClothConstants();
	//XMFLOAT4 g = XMFLOAT4(0, -9.8, 0, 0);
	//XMFLOAT4 wind = XMFLOAT4(0.0, 0.0, -15.02, 0);
	//clothEnvConstants->setGravity(g);
	//clothEnvConstants->setWind(wind);
	
	

}

#pragma region Private interface implementation


void BasicCloth::initialiseClothBuffers(ID3D11Device *device) {

		// Create vertex and index buffers in system memory to model the cloth
	BasicClothVertex*vertices = (BasicClothVertex*)malloc(clothWidth * clothHeight * sizeof(BasicClothVertex));
	indices = (DWORD*)malloc((clothWidth - 1) * (clothHeight - 1) * 6 * sizeof(DWORD));
		DWORD cloth_hw = (clothWidth - 1) / 2;
		DWORD cloth_hh = (clothHeight - 1) / 2;

		// Setup vertex positions
		float scale = 0.25f;
		BasicClothVertex *vptr = vertices;


		for (DWORD j = 0; j < clothHeight; j++) {

			float y = (float)j * scale - (float)cloth_hh * scale;

			for (DWORD i = 0; i < clothWidth; i++, vptr++) {

				float x = (float)i * scale - (float)cloth_hw * scale;

				// set position
				vptr->pos.x = x * 2.0f;
				vptr->pos.y = y * -2.0f;
				vptr->pos.z = 0;//0.3f * ( y*sinf(x) + x * cosf(y) ) * 1.5f;
				vptr->oldPos.x = x * 2.0f;
				vptr->oldPos.y = y * -2.0f;
				vptr->oldPos.z = 0;//0.3f * ( y*sinf(x) + x * cosf(y) ) * 1.5f;

				float r = (float)i / (float)(clothWidth - 1);
				float r2 = (float)j / (float)(clothHeight - 1);
				vptr->matDiffuse = XMCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

				vptr->matSpecular = XMCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
				vptr->normal = XMFLOAT3(0.0f, 0.0f, 1.0f);
				vptr->texCoord = XMFLOAT2((float)i / (float)(clothWidth - 1), (float)j / (float)(clothHeight - 1));
				vptr->isFixed = 0;
			}
		}
		// Fixed points
		vptr = vertices;
		vptr->isFixed = 1;
		vptr += (clothWidth/2)-1;
		vptr->isFixed = 1;
		vptr += clothWidth/2;
		vptr->isFixed = 1;

		// Setup index values
		DWORD *iptr = indices;
		for (DWORD j = 0; j < clothHeight - 1; j++) {


			for (DWORD i = 0; i < clothWidth - 1; i++, iptr += 6) {

				DWORD a = clothWidth * j + i;
				DWORD b = a + clothWidth;
				DWORD c = b + 1;
				DWORD d = a + 1;

				iptr[0] = a;
				iptr[1] = b;
				iptr[2] = d;

				iptr[3] = b;
				iptr[4] = c;
				iptr[5] = d;
			}
		}


		//
		// Setup DirectX vertex buffer
		//

		D3D11_BUFFER_DESC vertexDesc;
		D3D11_SUBRESOURCE_DATA vertexData;


		// This version only allows GPU cloth vertex buffer to be created
		vertexDesc.Usage = D3D11_USAGE_DEFAULT;

		vertexDesc.CPUAccessFlags = 0;
		vertexDesc.MiscFlags = 0;



		vertexDesc.ByteWidth = sizeof(BasicClothVertex) * clothWidth * clothHeight;
		vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
		vertexDesc.StructureByteStride = sizeof(BasicClothVertex);
		vertexData.pSysMem = vertices; // subresource data points to actual vertex buffer

		HRESULT hr = device->CreateBuffer(&vertexDesc, &vertexData, &Pb1);
		hr = device->CreateBuffer(&vertexDesc, &vertexData, &Pb2);
		vertexBuffer = Pb1;
		resultBuffer = Pb2;
		free(vertices);

		//
		// Setup DirectX index buffer
		//

		D3D11_BUFFER_DESC indexDesc;
		D3D11_SUBRESOURCE_DATA indexData;

		indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexDesc.ByteWidth = sizeof(DWORD) * ((clothWidth - 1) * (clothHeight - 1) * 6); // (clothWidth*clothHeight) faces, 3 indices per face
		indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexDesc.StructureByteStride = sizeof(DWORD);
		indexDesc.CPUAccessFlags = 0;
		indexDesc.MiscFlags = 0;

		indexData.pSysMem = indices; // subresource data points to actual index buffer

		device->CreateBuffer(&indexDesc, &indexData, &indexBuffer);

		initSpringsGPU();

}
#pragma endregion

BasicCloth::~BasicCloth() {


	//// Dispose of local (system memory) model
	//if (vertices)
	//	free(vertices);

	//if (indices)
	//	free(indices);

	//// Release DirectX interfaces
	//if (vertexBuffer)
	//	vertexBuffer->Release();

	//if (indexBuffer)
	//	indexBuffer->Release();

	//if (clothVertexLayout)
	//	clothVertexLayout->Release();

	//if (clothEffect)
	//	clothEffect->Release();
}

void BasicCloth::render(ID3D11DeviceContext *context)
{

	context->PSSetConstantBuffers(0, 1, &cBufferModelGPU);
	context->VSSetConstantBuffers(0, 1, &cBufferModelGPU);
	context->GSSetConstantBuffers(0, 1, &cBufferModelGPU);
	//clothInstance->mapCBuffer(context);
	//context->PSSetConstantBuffers(0, 1, &cBufferModelGPU);
	//context->VSSetConstantBuffers(0, 1, &cBufferModelGPU);
	//context->GSSetConstantBuffers(0, 1, &cBufferModelGPU);
//	mapCbuffer(context, cBufferModelCPU, cBufferModelGPU, sizeof(CBufferModel));
	// Set basic cloth  vertex and index buffers for IA
	

	UINT vertexStrides[] = { sizeof(BasicClothVertex) };
	UINT vertexOffsets[] = { 0 };

	context->IASetVertexBuffers(0, 1, &vertexBuffer, vertexStrides, vertexOffsets);

	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set vertex layout
	context->IASetInputLayout(inputLayout);

	// Set primitive topology for IA
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Bind render pipeline
	effect->bindPipeline(context);

	// Bind texture resource views and texture sampler objects to the PS stage of the pipeline
	//if (clothTextureSRV>0 && clothSampler) {

	//	context->PSSetShaderResources(0, 1, &clothTextureSRV);
	//	context->PSSetSamplers(0, 1, &clothSampler);
	//}

	if (numTextures>0 && sampler) {

		context->PSSetShaderResources(0, numTextures, textures);
		context->PSSetSamplers(0, 1, &sampler);
		//context->PSSetSamplers(0, 1, &clothSampler);
	}

	// Draw data declared above

	context->DrawIndexed(((clothWidth - 1) * (clothHeight - 1)) * 6, 0, 0);

	// Unbind buffers from IA ready for next iteration of render()

	ID3D11Buffer* nullbuffer[] = { 0 };
	UINT nullStrides[] = { 0 };
	UINT nullOffsets[] = { 0 };

	context->IASetVertexBuffers(0, 1, nullbuffer, nullStrides, nullOffsets);

}


void BasicCloth::updateCloth(ID3D11DeviceContext *context)
{

	//NULL buffers for unbinding
	ID3D11Buffer* nullbuffer[] = { 0 };
	UINT nullStrides[] = { 0 };
	UINT nullOffsets[] = { 0 };

	// Declare buffer binding properties for the cloth
	UINT vertexStrides[] = { sizeof(BasicClothVertex) };
	UINT vertexOffsets[] = { 0 };

	//clothEnvConstants->setReCalcNormals(1);
	//clothEnvConstants->mapCBuffer(context);

	
	// Update Springs passes
	for (DWORD j = 0; j < 100; j++)
	for (DWORD i = 0; i < NUM_BATCHES; i++)
	{

		// Bind source buffer and index buffer for IA and set triangle list topology
		context->IASetVertexBuffers(0, 1, &vertexBuffer, vertexStrides, vertexOffsets);
		context->IASetInputLayout(inputLayout);
		context->IASetIndexBuffer(updateIndexBuffer[i], DXGI_FORMAT_R32_UINT, 0);

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Bind update springs pipeline
		updateSpringsEffect->bindPipeline(context);

		// Bind result buffer to SO
		context->SOSetTargets(1, &resultBuffer, vertexOffsets);

		// Draw

		context->DrawIndexed(clothWidth * clothHeight * 3, 0, 0);

		// Unbind buffers from IA and SO since DX will not allow a resource to bind to a write / read binding point simultaneously

		context->IASetVertexBuffers(0, 1, nullbuffer, nullStrides, nullOffsets);
		context->SOSetTargets(1, nullbuffer, nullOffsets);

		//
		// Swap particle buffers for next iteration of the render() function
		//
		std::swap(vertexBuffer, resultBuffer);
		//clothEnvConstants->setReCalcNormals(0);
		//clothEnvConstants->mapCBuffer(context);
	}


	//
	// Update Forces pass
	//

	// Bind source buffer to IA and set point topology
	context->IASetVertexBuffers(0, 1, &vertexBuffer, vertexStrides, vertexOffsets);
	context->IASetInputLayout(inputLayout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	// Bind update pipeline
	updateForcesEffect->bindPipeline(context);

	// Bind result buffer to SO
	context->SOSetTargets(1, &resultBuffer, vertexOffsets);

	// Draw

	context->Draw(clothWidth * clothHeight, 0);

	// Unbind buffers from IA and SO since DX will not allow a resource to bind to a write / read binding point simultaneously

	context->IASetVertexBuffers(0, 1, nullbuffer, nullStrides, nullOffsets);
	context->SOSetTargets(1, nullbuffer, nullOffsets);

	//
	// Swap particle buffers for next iteration of the render() function
	//
	std::swap(vertexBuffer, resultBuffer);
}


void BasicCloth::initSpringsGPU()
{
	DWORD *iptr[NUM_BATCHES];
	DWORD *updateIndices[NUM_BATCHES];
	for(DWORD i=0;i<NUM_BATCHES;i++)
	{
		updateIndices[i] = (DWORD*)malloc((clothWidth) * (clothHeight) * 3 * sizeof(DWORD));
		iptr[i] = updateIndices[i];
	}

	DWORD a,b,c;

	for (DWORD j=0;j<clothHeight;j++) {
		for (DWORD i=0;i<clothWidth;i++, iptr[0]+=3,iptr[1]+=3, iptr[2]+=3,iptr[3]+=3) {

			a = clothWidth * j + i;
			c = clothWidth * j + min(i+1,clothWidth-1);
			b = clothWidth * min(j+1,clothHeight-1) + i;
			iptr[0][0] = a;
			iptr[0][1] = b;//y+1
			iptr[0][2] = c;//x+1

			b = (DWORD)(clothWidth * max((int)j-1,0) + i);
			iptr[1][0] = a;
			iptr[1][1] = c;//x+1;
			iptr[1][2] = b;//y-1

			c = (DWORD)(clothWidth * j + max((int)i-1,0));
			iptr[2][0] = a;
			iptr[2][1] = b;
			iptr[2][2] = c;

			b = clothWidth * min(j+1,clothHeight-1) + i;
			iptr[3][0] = a;
			iptr[3][1] = c;
			iptr[3][2] = b;

		}
	}
	D3D11_BUFFER_DESC updateIndexDesc;
	D3D11_SUBRESOURCE_DATA updateIndexData;
	updateIndexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	updateIndexDesc.ByteWidth = sizeof(DWORD) * ((clothWidth) * (clothHeight) * 3); // (clothWidth*clothHeight) faces, 3 indices per face
	updateIndexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	updateIndexDesc.CPUAccessFlags = 0;
	updateIndexDesc.MiscFlags = 0;
		
	for(DWORD i=0;i<NUM_BATCHES;i++)
	{
		updateIndexData.pSysMem = updateIndices[i]; // subresource data points to actual index buffer
		device->CreateBuffer(&updateIndexDesc, &updateIndexData, &updateIndexBuffer[i]);
	}
	for (DWORD i = 0; i<NUM_BATCHES; i++)
	{
		free(updateIndices[i]);
	}

}


//void BasicCloth::createDefaultLinearSampler(ID3D11Device *device) {
//
//	// If textures are used a sampler is required for the pixel shader to sample the texture
//	D3D11_SAMPLER_DESC samplerDesc;
//
//	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
//	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
//	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
//	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
//	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
//	samplerDesc.MinLOD = 0.0f;
//	samplerDesc.MaxLOD = 64.0f;
//	samplerDesc.MipLODBias = 0.0f;
//	samplerDesc.MaxAnisotropy = 16;
//	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
//	//samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
//
//	device->CreateSamplerState(&samplerDesc, &sampler);
//
//}
//void BasicCloth::initCBuffer(ID3D11Device *device) {
//
//	// Allocate 16 byte aligned block of memory for "main memory" copy of cBufferBasic
//	cBufferModelCPU = (CBufferModel*)_aligned_malloc(sizeof(CBufferModel), 16);
//
//	// Fill out cBufferModelCPU
//
//
//	XMVECTOR det = XMMatrixDeterminant(cBufferModelCPU->worldMatrix);
//	cBufferModelCPU->worldITMatrix  = XMMatrixInverse(&det, XMMatrixTranspose(cBufferModelCPU->worldMatrix));
//
//	// Create GPU resource memory copy of cBufferBasic
//	// fill out description (Note if we want to update the CBuffer we need  D3D11_CPU_ACCESS_WRITE)
//	D3D11_BUFFER_DESC cbufferDesc;
//	D3D11_SUBRESOURCE_DATA cbufferInitData;
//	ZeroMemory(&cbufferDesc, sizeof(D3D11_BUFFER_DESC));
//	ZeroMemory(&cbufferInitData, sizeof(D3D11_SUBRESOURCE_DATA));
//
//	cbufferDesc.ByteWidth = sizeof(CBufferModel);
//	cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
//	cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//	cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//	cbufferInitData.pSysMem = cBufferModelCPU;// Initialise GPU CBuffer with data from CPU CBuffer
//
//	HRESULT hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData,
//		&cBufferModelGPU);
//}