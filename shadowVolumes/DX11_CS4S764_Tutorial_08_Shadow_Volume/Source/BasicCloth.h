
#pragma once
#define NUM_BATCHES 4

#include <d3d11_2.h>
#include <DirectXMath.h>
#include <string>
#include "VertexStructures.h"
#include "CBufferStructures.h"
#include <Texture.h>
#include <BaseModel.h>

#define MAX_TEXTURES 8
#define MAX_MATERIALS 8
class Effect;
class Camera;

//#include <DXFoundation\DXBaseModel.h>
//#include <DXFoundation\DXPipeline.h>
//#include <DXFoundation\DXBlob.h>
//#include <DXFoundation\DXPivotCamera.h>
//#include <DXFoundation\DXDynamicCBuffer.h>


struct Spring{
	float targetLength;
	int v1;
	int v2;
};





class BasicCloth : public BaseModel {

private:
	ID3D11Device *device;	
	// Cloth particle effect pipeline configurations
	Effect								*updateForcesEffect = nullptr;
	Effect								*updateSpringsEffect = nullptr;
	DWORD									*indices;
	DWORD									clothWidth, clothHeight;
	DWORD									numSprings,numHSprings,numVSprings;
	ID3D11Buffer							*updateIndexBuffer[NUM_BATCHES];//*indexBuffer,
	ID3D11Buffer							*Pb1, *Pb2;
	ID3D11Buffer							*resultBuffer;

	Spring* springs;
	void initSpringsGPU();
	void BasicCloth::initialiseClothBuffers(ID3D11Device *device);
	void load(ID3D11Device *device_in,
		const			DWORD clothWidth_in,
		const			DWORD clothHeight_in,
		Effect			*updateForcesEffect_in,
		Effect			*updateSpringsEffect_in);
public:

	
	~BasicCloth();
	DWORD getVBSizeBytes(){ return clothWidth*clothHeight*sizeof(BasicClothVertex); }
	DWORD getVBStrideBytes(){ return (DWORD)sizeof(BasicClothVertex); }
	DWORD *getIndices() { return  indices; }
	ID3D11Buffer*getVB(){ return  vertexBuffer; }

	DWORD getNumFaces(){ return (clothWidth-1)*(clothHeight-1)*2; }

	void render(ID3D11DeviceContext *context);
	void updateCloth(ID3D11DeviceContext *context);
	
	HRESULT init(ID3D11Device *device) { return S_OK; };

	BasicCloth(ID3D11Device *device_in, 
		ID3D11DeviceContext			*context, 
		const  DWORD				clothWidth_in, 
		const DWORD					clothHeight_in, 
		Effect						*renderEffect_in,
		Effect						*updateForcesEffect_in,
		Effect						*updateSpringsEffect_in,
		Material					*_materials[] = nullptr, 
		int							_numMaterials = 0,
		ID3D11ShaderResourceView	**textures = nullptr, 
		int							numTextures = 0)
	: BaseModel(device_in, renderEffect_in, _materials, _numMaterials, textures, numTextures) 
	{ 
		load(device_in,
			clothWidth_in,
			clothHeight_in,
			updateForcesEffect_in,
			updateSpringsEffect_in ); 
	}
};
