
#include "stdafx.h"
#include "ShadowVolume.h"
#include "BaseModel.h"
#include "Camera.h"
#include "Effect.h"
#include <stdio.h>
#include <DirectXTK\DDSTextureLoader.h>
#include <DirectXTK\WICTextureLoader.h>

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

#define CPU 0

void ShadowVolume::load(ID3D11Device *_device, Effect *_renderEffect, XMVECTOR _pointLightVec, DWORD _vBSizeBytes, DWORD  *_indices, DWORD _numFaces)
{
	// Setup Shadow
	device = _device;
	renderEffect = _renderEffect;
	numFacesObj = _numFaces;
	indicesObj = _indices;
	HRESULT hr;

	// Setup rasteriser state
	auto RSState = renderEffect->getRasterizerState();
	D3D11_RASTERIZER_DESC RSdesc = {};
	RSState->GetDesc(&RSdesc);
	RSdesc.FillMode = D3D11_FILL_SOLID;
	RSdesc.CullMode = D3D11_CULL_NONE;
	RSdesc.FrontCounterClockwise = FALSE;
	device->CreateRasterizerState(&RSdesc, &RSState);
	renderEffect->setRasterizerState(RSState);

	// Setup wireframe rasteriser state
	RSdesc.FillMode = D3D11_FILL_WIREFRAME;
	device->CreateRasterizerState(&RSdesc, &RSstateWire);

	// Setup Depth and Stencil states
	auto updateSState = renderEffect->getDepthStencilState();
	shadowRenderPassDepthStencil = renderEffect->getDepthStencilState();
	D3D11_DEPTH_STENCIL_DESC updateSStateDesc = {};
	updateSState->GetDesc(&updateSStateDesc);
	//setup appropriate DepthStencil states for rendering to the stencil buffer

	//????
	updateSStateDesc.DepthEnable = true;
	updateSStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	updateSStateDesc.DepthFunc = D3D11_COMPARISON_LESS;

	// Stencil test parameters
	updateSStateDesc.StencilEnable = true;
	updateSStateDesc.StencilReadMask = 0xFF;
	updateSStateDesc.StencilWriteMask = 0xFF;

	updateSStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	updateSStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	// Increment to the stencil on pass.
	updateSStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	updateSStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	updateSStateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	updateSStateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	// Decrement the stencil on pass.
	updateSStateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_DECR;
	updateSStateDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;


	//device->CreateDepthStencilState(&dsDescRenderpass, &shadowRenderPassDepthStencil);
	device->CreateDepthStencilState(&updateSStateDesc, &updateSState);
	//renderEffect->setDepthStencilState(shadowRenderPassDepthStencil);
	renderEffect->setDepthStencilState(updateSState);
	// Initialise Shadow buffers
	initialiseShadowBuffers(device, _vBSizeBytes);
}


void ShadowVolume::initialiseShadowBuffers(ID3D11Device *device, DWORD vBSizeBytes) {

		DWORD maxNumEdgesSdw = numFacesObj*3;//max possible silhouette edges =  number of faces of shadow casting object*3
		DWORD maxNumVerticesSdw = maxNumEdgesSdw * 6;//create quad (2xTriangles) for each silhouette edge
		
		// Create vertex and edge buffers in system memory to model the Shadow Volume
		verticesObj = (BYTE*)malloc(vBSizeBytes);
		verticesSdw = (BasicVertexStruct*)malloc(maxNumVerticesSdw * sizeof(BasicVertexStruct));
		edgesSdw = (DWORD*)malloc(maxNumEdgesSdw * 2 * sizeof(DWORD));

		D3D11_BUFFER_DESC vertexDesc;
		D3D11_SUBRESOURCE_DATA vertexData;

		// This version only allows GPU Shadow vertex buffer to be created
		vertexDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vertexDesc.MiscFlags = 0;

		vertexDesc.ByteWidth = sizeof(BasicVertexStruct) * maxNumVerticesSdw;
		vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexData.pSysMem = verticesSdw; // subresource data points to actual vertex buffer

		HRESULT hr = device->CreateBuffer(&vertexDesc, &vertexData, &shadowBuffer);
		printf("Create VB = %d\n", hr);

		// This version only allows GPU Shadow vertex buffer to be created
		vertexDesc.Usage = D3D11_USAGE_STAGING;
		vertexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		vertexDesc.MiscFlags = 0;

		vertexDesc.ByteWidth = vBSizeBytes;
		vertexDesc.BindFlags = 0;
		//vertexData.pSysMem = verticesSdw; // subresource data points to actual vertex buffer

		hr = device->CreateBuffer(&vertexDesc, NULL, &stagingBuffer);
		printf("Create VB = %d\n", hr);
}


ShadowVolume::~ShadowVolume() {
	//// Dispose of local (system memory) model
	if (verticesSdw)
		free(verticesSdw);

	if (verticesObj)
		free(verticesObj);
//	if (indicesObj)
//		free(indicesObj);

	if (edgesSdw)
		free(edgesSdw);

	//// Release DirectX interfaces
	//if (vertexBuffer)
	//	vertexBuffer->Release();

	//if (indexBuffer)
	//	indexBuffer->Release();

	//if (ShadowEffect)
	//	ShadowEffect->Release();
}

void ShadowVolume::render(ID3D11DeviceContext *context)
{

	//ShadowInstance->mapCBuffer(context);
	context->PSSetConstantBuffers(0, 1, &cBufferModelGPU);
	context->VSSetConstantBuffers(0, 1, &cBufferModelGPU);
	context->GSSetConstantBuffers(0, 1, &cBufferModelGPU);

	// Set basic Shadow  vertex and index buffers for IA
	UINT vertexStrides[] = { sizeof(BasicVertexStruct) };
	UINT vertexOffsets[] = { 0 };

	context->IASetVertexBuffers(0, 1, &shadowBuffer, vertexStrides, vertexOffsets);

	// Set vertex layout
	context->IASetInputLayout(inputLayout);
	// Set primitive topology for IA
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Bind render pipeline
	renderEffect->bindPipeline(context);
	// Only rendering to Stencil so set RenderTarget to null
	ID3D11RenderTargetView *RTArray[1] = { nullptr },*RTArray2[1] = { nullptr };
	ID3D11DepthStencilView *DSView;
	context->OMGetRenderTargets(1, RTArray2, &DSView);
	context->OMSetRenderTargets(1, RTArray, DSView);
	//Render Shadow to Stencil Buffer
	context->Draw(numVerticesSdw, 0);
	//Put RenderTarget Back
	context->OMSetRenderTargets(1, RTArray2, DSView);
}

void ShadowVolume::renderWire(ID3D11DeviceContext *context)
{
	context->PSSetConstantBuffers(0, 1, &cBufferModelGPU);
	context->VSSetConstantBuffers(0, 1, &cBufferModelGPU);
	context->GSSetConstantBuffers(0, 1, &cBufferModelGPU);
	// Set basic Shadow  vertex and index buffers for IA
	UINT vertexStrides[] = { sizeof(BasicVertexStruct) };
	UINT vertexOffsets[] = { 0 };
	context->IASetVertexBuffers(0, 1, &shadowBuffer, vertexStrides, vertexOffsets);
	// Set vertex layout
	context->IASetInputLayout(inputLayout);
	// Set primitive topology for IA
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Store Solid Fill Rasteriser State
	auto RSStateSolid = renderEffect->getRasterizerState();
	// Setup wireframe rasteriser state
	renderEffect->setRasterizerState(RSstateWire);
	// Bind render pipeline
	renderEffect->bindPipeline(context);
	// Draw data declared above
	context->Draw(numVerticesSdw, 0);
	// Return RSState to Solid Fill
	renderEffect->setRasterizerState(RSStateSolid);
}


//-----------------------------------------------------------------------------
// Name: buildShadowVolume()
// Desc: Takes a VertexBuffer as input, and uses it to build a shadow volume. The
//       technique used considers each triangle of the VertexBuffer, and adds it's
//       edges to a temporary list. The edge list is maintained, such that
//       only silohuette edges are kept. Finally, the silohuette edges are
//       extruded to make the shadow volume vertex list.
//-----------------------------------------------------------------------------
HRESULT  ShadowVolume::buildShadowVolume(ID3D11DeviceContext  *context, ID3D11Buffer *vertexBufferObj,DWORD stride, DirectX::XMVECTOR tLight)
{

	D3D11_BUFFER_DESC buffer_desc;
	vertexBufferObj->GetDesc(&buffer_desc);
	context->CopySubresourceRegion(stagingBuffer, 0, 0, 0, 0, vertexBufferObj, 0, NULL);

	D3D11_MAPPED_SUBRESOURCE mapped_resource;
	HRESULT hr = context->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &mapped_resource);
	if (SUCCEEDED(hr))
	{
		memcpy(verticesObj, mapped_resource.pData, buffer_desc.ByteWidth);
		context->Unmap(stagingBuffer, 0);
	}
	else
		printf("copy failed");
	
	DWORD dwNumVerticesObj = buffer_desc.ByteWidth / stride;
	DWORD dwNumEdges = 0;
	numVerticesSdw = 0;
	XMFLOAT3 *pXMF;
	// For each face
	for (DWORD i = 0; i < numFacesObj; ++i)
	{
		DWORD wFace0 = indicesObj[3 * i + 0];
		DWORD wFace1 = indicesObj[3 * i + 1];
		DWORD wFace2 = indicesObj[3 * i + 2];
		//printf("faces= %d,%d,%d\n", wFace0, wFace1, wFace2);
		pXMF = (XMFLOAT3 *)(verticesObj + (wFace0*stride));
		XMVECTOR v0 = XMLoadFloat3(pXMF);
		//printf("v0= %f,%f,%f\n", XMVectorGetX(v0), XMVectorGetY(v0), XMVectorGetZ(v0));
		pXMF = (XMFLOAT3 *)(verticesObj + (wFace1*stride));
		XMVECTOR v1 = XMLoadFloat3(pXMF);
		pXMF = (XMFLOAT3 *)(verticesObj + (wFace2*stride));
		XMVECTOR v2 = XMLoadFloat3(pXMF);
		// Transform vertices or transform light?
		XMVECTOR vCross1 = XMVECTOR(v2 - v1);
		XMVECTOR vCross2 = XMVECTOR(v1 - v0);
		
		XMVECTOR vNormal=XMVector3Cross( vCross1, vCross2);
		vNormal = XMVector3Normalize(vNormal);
		XMVECTOR tLightNorm = XMVector3Normalize(tLight);
		//printf("normal= %f,%f,%f\n", XMVectorGetX(vNormal), XMVectorGetY(vNormal), XMVectorGetZ(vNormal));


		float facing = XMVectorGetX(XMVector3Dot(vNormal, tLightNorm));
		if (facing >= 0.0f)
		{
			//printf("add edge");
			addEdge(edgesSdw, dwNumEdges, wFace0, wFace1);
			addEdge(edgesSdw, dwNumEdges, wFace1, wFace2);
			addEdge(edgesSdw, dwNumEdges, wFace2, wFace0);
		}
		//if object is 2 sided flip normals and check back of face
		if (objDoubleSided)
		{
			facing = XMVectorGetX(XMVector3Dot(-vNormal, tLightNorm));
			if (facing >= 0.0f)
			{
				//printf("add edge");
				addEdge(edgesSdw, dwNumEdges, wFace0, wFace1);
				addEdge(edgesSdw, dwNumEdges, wFace1, wFace2);
				addEdge(edgesSdw, dwNumEdges, wFace2, wFace0);
			}
		}

	}

	printf("num edges = %d\n", dwNumEdges);
	for (int i = 0; i < dwNumEdges; ++i)
	{
		pXMF = (XMFLOAT3 *)(verticesObj + (edgesSdw[2 * i + 0] * stride));
		XMVECTOR v1 = XMLoadFloat3(pXMF);
		pXMF = (XMFLOAT3 *)(verticesObj + (edgesSdw[2 * i + 1] * stride));
		XMVECTOR v2 = XMLoadFloat3(pXMF);

		//Positional Light
		XMVECTOR v3 = v1 - (tLight - v1) * 10;
		XMVECTOR v4 = v2 - (tLight - v2 ) * 10;
		//Directional Light
		//XMVECTOR v3 = v1 - tLight * 10;
		//XMVECTOR v4 = v2 - tLight * 10;
		// Add a quad (two triangles) to the vertex list
		XMStoreFloat3(&verticesSdw[numVerticesSdw++].pos,v1);
		XMStoreFloat3(&verticesSdw[numVerticesSdw++].pos, v2);
		XMStoreFloat3(&verticesSdw[numVerticesSdw++].pos, v3);
		XMStoreFloat3(&verticesSdw[numVerticesSdw++].pos, v2);
		XMStoreFloat3(&verticesSdw[numVerticesSdw++].pos, v4);
		XMStoreFloat3(&verticesSdw[numVerticesSdw++].pos, v3);
	}

	hr = context->Map(shadowBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
	//memcpy(mappedResource.pData, buffer, rowspan*height);

	if (SUCCEEDED(hr))
	{
		memcpy(mapped_resource.pData,verticesSdw, sizeof(BasicVertexStruct)*numVerticesSdw++);
		context->Unmap(shadowBuffer, 0);
	}
	else
		printf("copy failed");

	return 1;
}
//-------------------------------------------- -
// Desc: Adds an edge to a list of silohuette edges of a shadow volume.
//------------------------------------------------------------------------
// Remove interior edges (which appear in the list twice)
void ShadowVolume::addEdge(DWORD* pEdges, DWORD& dwNumEdges, DWORD v0, DWORD v1)
{
	for (DWORD i = 0; i < dwNumEdges; ++i)
	{
		if ((pEdges[2 * i + 0] == v0 && pEdges[2 * i + 1] == v1) ||
			(pEdges[2 * i + 0] == v1 && pEdges[2 * i + 1] == v0))
		{
			if (dwNumEdges > 1)
			{
				pEdges[2 * i + 0] = pEdges[2 * (dwNumEdges - 1) + 0];
				pEdges[2 * i + 1] = pEdges[2 * (dwNumEdges - 1) + 1];
			}
			--dwNumEdges;
			return;
		}
	}
	pEdges[2 * dwNumEdges + 0] = v0;
	pEdges[2 * dwNumEdges + 1] = v1;
	dwNumEdges++;
}
