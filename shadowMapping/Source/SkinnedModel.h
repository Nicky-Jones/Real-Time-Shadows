
//
// Model.h
//

// Version 1.  Encapsulate the mesh contents of a CGModel imported via CGImport3.  Currently supports obj, 3ds or gsf files.  md2, md3 and md5 (CGImport4) untested.  For version 1 a single texture and sampler interface are associated with the Model.


#pragma once
#include <d3d11_2.h>
#include <DirectXMath.h>
#include <Model.h>
#include <Animation.h>
#include <string>
#include <vector>
#include <cstdint>
#include <CBufferStructures.h>
#include <Utils.h>
#include <Camera.h>
#include <VertexStructures.h>
#include <map>
#include <Assimp\include\assimp\Importer.hpp>      // C++ importer interface
#include <Assimp\include\assimp\scene.h>           // Output data structure
#include <Assimp\include\assimp\postprocess.h>     // Post processing flags

using namespace std;

struct Bone{
	DirectX::XMMATRIX		offMatrix;
	DirectX::XMMATRIX		finalTrans;
	DirectX::XMFLOAT4		weights;
	DirectX::XMINT4			vertexID;
};

class SkinnedModel : public Model {
	int						numBones;
	Bone					*bones = nullptr;
	CBufferBone				*cBufferBonesCPU = nullptr;
	ID3D11Buffer			*cBufferBonesGPU = nullptr;
	map<string, int> m_BoneMapping; // maps a bone name to its index

	XMMATRIX invGlobalTransform;
	int currentAnim=0;
	

public:

	SkinnedModel(ID3D11Device *device,  std::wstring& filename, Effect *_effect, Material *_materials[] = nullptr, int _numMaterials = 0, ID3D11ShaderResourceView **textures = nullptr, int numTextures = 0) : Model(device, filename, _effect, _materials, _numMaterials, textures, numTextures,false){ loadBones(device, _effect, filename, NULL); }
	~SkinnedModel();
	void loadBones(ID3D11Device *device, Effect *_effect,  std::wstring& filename, Material *_material);	
	void render(ID3D11DeviceContext *context);
	void renderSimp(ID3D11DeviceContext *context);
	XMMATRIX boneTransform(float time,int anim);
	const aiNodeAnim* SkinnedModel::findNodeAnim(const aiAnimation* pAnimation, const string NodeName);
	void readNodeHeirarchy(float animTime, const aiNode* node, const XMMATRIX parentTransform,int anim);
	int findPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
	int findScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
	int findRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
	void calcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void calcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void calcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void update(ID3D11DeviceContext *context, double time);
	int getNumAnimimations(){ return scene->mNumAnimations; };
	int getCurrentAnim(){ return currentAnim; };
	void setCurrentAnim(int anim){currentAnim=anim; };

};
