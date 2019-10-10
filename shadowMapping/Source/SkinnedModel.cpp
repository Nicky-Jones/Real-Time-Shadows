
#include "stdafx.h"
#include <SkinnedModel.h>
#include <Material.h>
#include <Effect.h>
#include <iostream>
#include <exception>

#include <CoreStructures\CoreStructures.h>
#include <CGImport3\CGModel\CGModel.h>
#include <CGImport3\Importers\CGImporters.h>



using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace CoreStructures;

XMMATRIX AIMat2XMMat(aiMatrix4x4 m)
{
	return XMMATRIX(m.a1, m.b1, m.c1, m.d1, m.a2, m.b2, m.c2, m.d2, m.a3, m.b3, m.c3, m.d3, m.a4, m.b4, m.c4, m.d4);
	//return XMMATRIX(m.a1, m.a2, m.a3, m.a4, m.b1, m.b2, m.b3, m.b4, m.c1, m.c2, m.c3, m.c4, m.d1, m.d2, m.d3, m.d4);

}
XMMATRIX AIMat2XMMat(aiMatrix3x3 m)
{
	XMFLOAT3X3 temp = XMFLOAT3X3(m.a1, m.b1, m.c1, m.a2, m.b2, m.c2, m.a3, m.b3, m.c3);
	//XMFLOAT3X3 temp = XMFLOAT3X3(m.a1, m.a2, m.a3, m.b1, m.b2, m.b3, m.c1, m.c2, m.c3);
	return XMLoadFloat3x3(&temp);
}
void SkinnedModel::loadBones(ID3D11Device *device, Effect *_effect,std::wstring& filename,  Material *_material) {
	wcout << L"Loading Bones: " << filename << endl;
	cout << "Num Meshes: " << scene->mNumMeshes << endl;

	XMMATRIX rootTransform = AIMat2XMMat(scene->mRootNode->mTransformation);


	invGlobalTransform = XMMatrixInverse(NULL, rootTransform);
	setWorldMatrix(invGlobalTransform);
	SkinnedVertexStruct *_vertexBuffer = (SkinnedVertexStruct*)malloc(numVertices * sizeof(SkinnedVertexStruct));

	for (int i = 0; i < numVertices; i++)
	{
		_vertexBuffer[i].pos = vertexBufferCPU[i].pos;
		_vertexBuffer[i].normal = vertexBufferCPU[i].normal;
		_vertexBuffer[i].tangent = vertexBufferCPU[i].tangent;
		_vertexBuffer[i].texCoord = vertexBufferCPU[i].texCoord;
		_vertexBuffer[i].matDiffuse = vertexBufferCPU[i].matDiffuse;
		_vertexBuffer[i].matSpecular = vertexBufferCPU[i].matSpecular;
		_vertexBuffer[i].boneWeights = XMFLOAT4(0,0,0,0);
		_vertexBuffer[i].boneIndices = XMINT4(-1, -1, -1, -1);
		_vertexBuffer[i].boneWeights2 = XMFLOAT4(0, 0, 0, 0);
		_vertexBuffer[i].boneIndices2 = XMINT4(-1, -1, -1, -1);
		_vertexBuffer[i].boneWeights3 = XMFLOAT4(0, 0, 0, 0);
		_vertexBuffer[i].boneIndices3 = XMINT4(-1, -1, -1, -1);
	}
	int max_num_weights = 0;
	int meshInd = 0;
	numBones = 0;
	for (int k = 0; k < scene->mNumMeshes; k++)
		numBones += scene->mMeshes[k]->mNumBones;
	cout << "Total Bones= " << numBones << endl;
	bones = (Bone*)malloc(sizeof(Bone) * numBones);
	int boneCounter = 0;
	for (int k = 0; k < scene->mNumMeshes; k++)
	{
		cout << "Loading Bones for Mesh: " << k << endl;
		meshInd = k;
		aiMesh* mesh = scene->mMeshes[meshInd];
		int numBonesl = mesh->mNumBones;
		
		cout << "Num Bones= " << numBonesl << endl;
		for (int i = 0; i < numBonesl; i++)
		{
			//cout << mesh->mBones[i]->mNumWeights << endl;
			string boneName(mesh->mBones[i]->mName.data);
			//cout << boneName << endl;
			int BoneIndex = 0;


			if (m_BoneMapping.find(boneName) == m_BoneMapping.end()) {

				BoneIndex = boneCounter;
				cout << "Adding new bone: " << boneName << " at slot: " << BoneIndex << " bone count:" << boneCounter + 1 << endl;
				boneCounter++;
				//BoneInfo bi;
				//m_BoneInfo.push_back(bi);
			}
			else {
				BoneIndex = m_BoneMapping[boneName];
				cout << "Bone: " << boneName << " found at slot: " << BoneIndex << " bone count:" << boneCounter << endl;
			}

			bones[BoneIndex].offMatrix = AIMat2XMMat(mesh->mBones[i]->mOffsetMatrix);

			m_BoneMapping[boneName] = BoneIndex;
			for (int j = 0; j < mesh->mBones[i]->mNumWeights; j++)
			{

				int index = baseVertexOffset[meshInd] + mesh->mBones[i]->mWeights[j].mVertexId;
				//cout << index << endl;
				if (index >= 0)
				{
					if (_vertexBuffer[index].boneIndices.x < 0)
					{
						_vertexBuffer[index].boneWeights.x = mesh->mBones[i]->mWeights[j].mWeight;
						_vertexBuffer[index].boneIndices.x = BoneIndex;
						if (max_num_weights < 1) max_num_weights = 1;
					}
					else if (_vertexBuffer[index].boneIndices.y < 0)
					{
						_vertexBuffer[index].boneWeights.y = mesh->mBones[i]->mWeights[j].mWeight;
						_vertexBuffer[index].boneIndices.y = BoneIndex;
						if (max_num_weights < 2) max_num_weights = 2;
					}
					else if (_vertexBuffer[index].boneIndices.z < 0)
					{
						_vertexBuffer[index].boneWeights.z = mesh->mBones[i]->mWeights[j].mWeight;
						_vertexBuffer[index].boneIndices.z = BoneIndex;
						if (max_num_weights < 3) max_num_weights = 3;
					}
					else if (_vertexBuffer[index].boneIndices.w < 0)
					{
						_vertexBuffer[index].boneWeights.w = mesh->mBones[i]->mWeights[j].mWeight;
						_vertexBuffer[index].boneIndices.w = BoneIndex;
						if (max_num_weights < 4) max_num_weights = 4;
					}
					else if (_vertexBuffer[index].boneIndices2.x < 0)
					{
						_vertexBuffer[index].boneWeights2.x = mesh->mBones[i]->mWeights[j].mWeight;
						_vertexBuffer[index].boneIndices2.x = BoneIndex;
						if (max_num_weights < 5) max_num_weights = 5;
					}
					else if (_vertexBuffer[index].boneIndices2.y < 0)
					{
						_vertexBuffer[index].boneWeights2.y = mesh->mBones[i]->mWeights[j].mWeight;
						_vertexBuffer[index].boneIndices2.y = BoneIndex;
						if (max_num_weights < 6) max_num_weights = 6;
					}
					else if (_vertexBuffer[index].boneIndices2.z < 0)
					{
						_vertexBuffer[index].boneWeights2.z = mesh->mBones[i]->mWeights[j].mWeight;
						_vertexBuffer[index].boneIndices2.z = BoneIndex;
						if (max_num_weights < 7) max_num_weights = 7;
					}
					else if (_vertexBuffer[index].boneIndices2.w < 0)
					{
						_vertexBuffer[index].boneWeights2.w = mesh->mBones[i]->mWeights[j].mWeight;
						_vertexBuffer[index].boneIndices2.w = BoneIndex;
						if (max_num_weights < 8) max_num_weights = 8;
					}
					else if (_vertexBuffer[index].boneIndices3.x < 0)
					{
						_vertexBuffer[index].boneWeights3.x = mesh->mBones[i]->mWeights[j].mWeight;
						_vertexBuffer[index].boneIndices3.x = BoneIndex;
						if (max_num_weights < 9) max_num_weights = 9;
					}
					else if (_vertexBuffer[index].boneIndices3.y < 0)
					{
						_vertexBuffer[index].boneWeights3.y = mesh->mBones[i]->mWeights[j].mWeight;
						_vertexBuffer[index].boneIndices3.y = BoneIndex;
						if (max_num_weights < 10) max_num_weights = 10;
					}
					else if (_vertexBuffer[index].boneIndices3.z < 0)
					{
						_vertexBuffer[index].boneWeights3.z = mesh->mBones[i]->mWeights[j].mWeight;
						_vertexBuffer[index].boneIndices3.z = BoneIndex;
						if (max_num_weights < 11) max_num_weights = 11;
					}
					else if (_vertexBuffer[index].boneIndices3.w < 0)
					{
						_vertexBuffer[index].boneWeights3.w = mesh->mBones[i]->mWeights[j].mWeight;
						_vertexBuffer[index].boneIndices3.w = BoneIndex;
						if (max_num_weights < 12) max_num_weights = 12;
					}
					else
					{
						cout << "too many weights" << endl;
					}

				}
			}

		}
		cout << "Num Counted= " << boneCounter << endl;
	}
	numBones = boneCounter;
	//numBones = boneCounter;
	cout << "Total Bones= " << numBones << endl;;
	cout << "Max num weights: " << max_num_weights << endl;
	float maxsumofweights = 0;
	float minsumofweights = 10;
	for (int i = 0; i < numVertices; i++)
	{
		float boneweightsum = _vertexBuffer[i].boneWeights.x + _vertexBuffer[i].boneWeights.y + _vertexBuffer[i].boneWeights.z + _vertexBuffer[i].boneWeights.w;
		boneweightsum += _vertexBuffer[i].boneWeights2.x + _vertexBuffer[i].boneWeights2.y + _vertexBuffer[i].boneWeights2.z + _vertexBuffer[i].boneWeights2.w;
		boneweightsum += _vertexBuffer[i].boneWeights3.x + _vertexBuffer[i].boneWeights3.y + _vertexBuffer[i].boneWeights3.z + _vertexBuffer[i].boneWeights3.w;

		_vertexBuffer[i].boneWeights.x /= boneweightsum; _vertexBuffer[i].boneWeights.y /= boneweightsum; _vertexBuffer[i].boneWeights.z /= boneweightsum; _vertexBuffer[i].boneWeights.w /= boneweightsum;;
		_vertexBuffer[i].boneWeights2.x /= boneweightsum; _vertexBuffer[i].boneWeights2.y /= boneweightsum; _vertexBuffer[i].boneWeights2.z /= boneweightsum; _vertexBuffer[i].boneWeights2.w /= boneweightsum;;
		_vertexBuffer[i].boneWeights3.x /= boneweightsum; _vertexBuffer[i].boneWeights3.y /= boneweightsum; _vertexBuffer[i].boneWeights3.z /= boneweightsum; _vertexBuffer[i].boneWeights3.w /= boneweightsum;;
		boneweightsum = _vertexBuffer[i].boneWeights.x + _vertexBuffer[i].boneWeights.y + _vertexBuffer[i].boneWeights.z + _vertexBuffer[i].boneWeights.w;
		boneweightsum += _vertexBuffer[i].boneWeights2.x + _vertexBuffer[i].boneWeights2.y + _vertexBuffer[i].boneWeights2.z + _vertexBuffer[i].boneWeights2.w;
		boneweightsum += _vertexBuffer[i].boneWeights3.x + _vertexBuffer[i].boneWeights3.y + _vertexBuffer[i].boneWeights3.z + _vertexBuffer[i].boneWeights3.w;
		if (boneweightsum > maxsumofweights)maxsumofweights = boneweightsum;
		if (boneweightsum < minsumofweights)minsumofweights = boneweightsum;
	}
	cout << "maxsumofweights: " << maxsumofweights << " minsumofweights: " << minsumofweights << endl;
	//	if (_vertexBuffer[i].boneIndices.x<0 | _vertexBuffer[i].boneIndices.y<0 | _vertexBuffer[i].boneIndices.z<0 | _vertexBuffer[i].boneIndices.w<0)
	//		cout << "vertex:" << i << "Bone indecies: " << " 0:" << _vertexBuffer[i].boneIndices.x << " 1:" << _vertexBuffer[i].boneIndices.y << " 2:" << _vertexBuffer[i].boneIndices.z << " 3:" << _vertexBuffer[i].boneIndices.w << " Bone Weights" << " 0:" << _vertexBuffer[i].boneWeights.x << " 1:" << _vertexBuffer[i].boneWeights.y << " 2:" << _vertexBuffer[i].boneWeights.z << " 3:" << _vertexBuffer[i].boneWeights.w << endl;
	if(vertexBuffer)
		vertexBuffer->Release();
	// Setup DX vertex buffer interfaces
	D3D11_BUFFER_DESC vertexDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	ZeroMemory(&vertexDesc, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&vertexData, sizeof(D3D11_SUBRESOURCE_DATA));

	vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.ByteWidth = numVertices * sizeof(SkinnedVertexStruct);
	vertexData.pSysMem = _vertexBuffer;

	HRESULT hr = device->CreateBuffer(&vertexDesc, &vertexData, &vertexBuffer);
	cBufferBonesCPU = (CBufferBone*)malloc(sizeof(CBufferBone)*numBones);
	// fill out description (Note if we want to update the CBuffer we need  D3D11_CPU_ACCESS_WRITE)
	D3D11_BUFFER_DESC cbufferDesc;
	D3D11_SUBRESOURCE_DATA cbufferInitData;
	ZeroMemory(&cbufferDesc, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&cbufferInitData, sizeof(D3D11_SUBRESOURCE_DATA));

	cbufferDesc.ByteWidth = sizeof(CBufferBone)*numBones;
	cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferInitData.pSysMem = cBufferBonesCPU;// Initialise GPU CBuffer with data from CPU CBuffer

	 hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData,&cBufferBonesGPU);
}

//
//void SkinnedModel::LoadBones(uint MeshIndex, const aiMesh* pMesh, vector<VertexBoneData>& Bones)
//{
//	for (uint i = 0; i < pMesh->mNumBones; i++) {
//		uint BoneIndex = 0;
//		string BoneName(pMesh->mBones[i]->mName.data);
//
//		if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
//			// Allocate an index for a new bone
//			BoneIndex = m_NumBones;
//			m_NumBones++;
//			BoneInfo bi;
//			m_BoneInfo.push_back(bi);
//			m_BoneInfo[BoneIndex].BoneOffset = pMesh->mBones[i]->mOffsetMatrix;
//			m_BoneMapping[BoneName] = BoneIndex;
//		}
//		else {
//			BoneIndex = m_BoneMapping[BoneName];
//		}
//
//		for (uint j = 0; j < pMesh->mBones[i]->mNumWeights; j++) {
//			uint VertexID = m_Entries[MeshIndex].BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;
//			float Weight = pMesh->mBones[i]->mWeights[j].mWeight;
//			Bones[VertexID].AddBoneData(BoneIndex, Weight);
//		}
//	}
//}


XMMATRIX SkinnedModel::boneTransform(float timeS,int anim)
{
	XMMATRIX ident=XMMatrixIdentity();
	cout << "mNumAnimations" << scene->mNumAnimations << endl;
	if (anim >= scene->mNumAnimations)anim = scene->mNumAnimations - 1;
	cout << "Running Animation:" << anim << "Name:" << scene->mAnimations[anim]->mName.C_Str()<< endl;
	float ticksPerSecond = scene->mAnimations[anim]->mTicksPerSecond != 0 ?scene->mAnimations[anim]->mTicksPerSecond : 25.0f;
	float ticks = timeS * ticksPerSecond;
	float animTime = fmod(ticks, scene->mAnimations[anim]->mDuration);
	cout << "animTime" << animTime << "ticksPerSecond" << ticksPerSecond << "ticks" << ticks << endl;
	readNodeHeirarchy(animTime, scene->mRootNode, ident, anim);

	//Transforms.resize(m_NumBones);

	for (int i = 0; i < numBones; i++) {
		cBufferBonesCPU[i].boneMatrix = bones[i].finalTrans;
	}
	//cout << numBones << endl;
	return XMMatrixIdentity();
}

const aiNodeAnim* SkinnedModel::findNodeAnim(const aiAnimation* pAnimation, const string NodeName)
{
	for (int i = 0; i < pAnimation->mNumChannels; i++) {
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

		if (string(pNodeAnim->mNodeName.data) == NodeName) {
			return pNodeAnim;
		}
	}

	return NULL;
}

void SkinnedModel::readNodeHeirarchy(float animTime, const aiNode* node, const XMMATRIX parentTransform,int anim)
{
	string nodeName(node->mName.data);
	//XMMATRIX globalTransformation = parentTransform;
	//if (m_BoneMapping.find(nodeName) != m_BoneMapping.end())
	//{
		//	cout << "Reading Node: " << nodeName << " found at: " << m_BoneMapping[nodeName] << " Num Children: " << node->mNumChildren << endl;
		//else
		//	cout << "Reading Node: " << nodeName << " not found " << " Num Children: " << node->mNumChildren << endl;
		const aiAnimation* animation = scene->mAnimations[anim];
		aiMatrix4x4 m = node->mTransformation;
		XMMATRIX nodeTrans = AIMat2XMMat(node->mTransformation);
		//cout << nodeTrans.m[0][0] << endl;
		const aiNodeAnim* nodeAnim = findNodeAnim(animation, nodeName);

		if (nodeAnim) {
			//cout << "Animation found for node" << nodeAnim << endl;

			// Interpolate scaling and generate scaling transformation matrix
			aiVector3D scaling;
			calcInterpolatedScaling(scaling, animTime, nodeAnim);
			XMMATRIX scalingMatrix = XMMatrixScaling(scaling.x, scaling.y, scaling.z);


			// Interpolate rotation and generate rotation transformation matrix
			aiQuaternion rotation;
			calcInterpolatedRotation(rotation, animTime, nodeAnim);
			aiMatrix3x3 m = rotation.GetMatrix();
			XMMATRIX rotMatrix = AIMat2XMMat(m);// XMLoadFloat3x3(&temp);

			// Interpolate translation and generate translation transformation matrix
			aiVector3D trans;
			calcInterpolatedPosition(trans, animTime, nodeAnim);
			XMMATRIX transMatrix = XMMatrixTranslation(trans.x, trans.y, trans.z);


			// Combine the above transformations
			nodeTrans = rotMatrix*transMatrix*scalingMatrix;
		}
		//else
		//	cout << "No Animation found for node" << nodeAnim << endl;

		XMMATRIX globalTransformation  = nodeTrans*parentTransform;

		if (m_BoneMapping.find(nodeName) != m_BoneMapping.end()) {
			int boneIndex = m_BoneMapping[nodeName];
			bones[boneIndex].finalTrans = bones[boneIndex].offMatrix*globalTransformation*invGlobalTransform;
		}
	
	for (int i = 0; i < node->mNumChildren; i++) {
		readNodeHeirarchy(animTime, node->mChildren[i], globalTransformation,anim);
	}
}

int SkinnedModel::findPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	for (int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}


int SkinnedModel::findRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);

	for (int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}


int SkinnedModel::findScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumScalingKeys > 0);

	for (int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}


void SkinnedModel::calcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumPositionKeys == 1) {
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	int PositionIndex = findPosition(AnimationTime, pNodeAnim);
	//cout << PositionIndex << endl;
	int NextPositionIndex = (PositionIndex + 1);
	assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	
	const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}


void SkinnedModel::calcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	int RotationIndex = findRotation(AnimationTime, pNodeAnim);
	int NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out = Out.Normalize();
}


void SkinnedModel::calcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1) {
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	int ScalingIndex = findScaling(AnimationTime, pNodeAnim);
	int NextScalingIndex = (ScalingIndex + 1);
	assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

SkinnedModel::~SkinnedModel() {

}
//
void SkinnedModel::update(ID3D11DeviceContext *context,  double time) {
	BaseModel::update(context); //call baseModel update

	boneTransform(time,currentAnim);
	mapCbuffer(context, cBufferBonesCPU, cBufferBonesGPU,sizeof(CBufferBone)*numBones);

}

void SkinnedModel::render(ID3D11DeviceContext *context) {//, int mode

	effect->bindPipeline(context);


	// Apply the cBuffer.
	context->VSSetConstantBuffers(0, 1, &cBufferModelGPU);
	context->PSSetConstantBuffers(0, 1, &cBufferModelGPU);
	context->VSSetConstantBuffers(5, 1, &cBufferBonesGPU);
	// Validate Model before rendering (see notes in constructor)
	if (!context || !vertexBuffer || !indexBuffer || !effect)
		return;

	// Set vertex layout
	context->IASetInputLayout(effect->getVSInputLayout());

	// Set Model vertex and index buffers for IA
	ID3D11Buffer* vertexBuffers[] = { vertexBuffer };
	UINT vertexStrides[] = { sizeof(SkinnedVertexStruct) };
	UINT vertexOffsets[] = { 0 };

	context->IASetVertexBuffers(0, 1, vertexBuffers, vertexStrides, vertexOffsets);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set primitive topology for IA
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Bind texture resource views and texture sampler objects to the PS stage of the pipeline
	if (numTextures>0 && sampler) {

		context->PSSetShaderResources(0, numTextures, textures);
		context->PSSetSamplers(0, 1, &sampler);
	}


	// Set vertex layout
	context->IASetInputLayout(inputLayout);





	// Draw Model
	for (uint32_t indexOffset = 0, i = 0; i < numMeshes; indexOffset += indexCount[i], ++i)
		context->DrawIndexed(indexCount[i], indexOffset, baseVertexOffset[i]);	
	

}

void SkinnedModel::renderSimp(ID3D11DeviceContext *context) {

	// Validate Model before rendering (see notes in constructor)
	if (!context || !vertexBuffer || !indexBuffer )
		return;

	// Set vertex layout
	context->IASetInputLayout(effect->getVSInputLayout());

	// Set Model vertex and index buffers for IA
	ID3D11Buffer* vertexBuffers[] = { vertexBuffer };
	UINT vertexStrides[] = { sizeof(ExtendedVertexStruct) };
	UINT vertexOffsets[] = { 0 };

	context->IASetVertexBuffers(0, 1, vertexBuffers, vertexStrides, vertexOffsets);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set primitive topology for IA
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	// Bind texture resource views and texture sampler objects to the PS stage of the pipeline
	if (numTextures>0 && sampler) {

		context->PSSetShaderResources(0, numTextures, textures);
		context->PSSetSamplers(0, 1, &sampler);
	}



	// Apply the cBuffer.
	context->VSSetConstantBuffers(0, 1, &cBufferModelGPU);
	context->PSSetConstantBuffers(0, 1, &cBufferModelGPU);

	// Draw Model
	for (uint32_t indexOffset = 0, i = 0; i < numMeshes; indexOffset += indexCount[i], ++i)
		context->DrawIndexed(indexCount[i], indexOffset, baseVertexOffset[i]);
}

//
//
//HRESULT SkinnedModel::loadModel(ID3D11Device *device, const std::wstring& filename)
//{
//	CGModel *actualModel = nullptr;
//	ExtendedVertexStruct *_vertexBuffer = nullptr;
//	uint32_t *_indexBuffer = nullptr;
//
//	try
//	{
//	actualModel = new CGModel();
//
//
//
//
//	if (!actualModel)
//		throw exception("Cannot create model to import");
//
//
//	// Get filename extension
//	wstring ext = filename.substr(filename.length() - 4);
//
//	CG_IMPORT_RESULT cg_err;
//
//	if (0 == ext.compare(L".gsf"))
//		cg_err = importGSF(filename.c_str(), actualModel);
//	else if (0 == ext.compare(L".3ds"))
//		cg_err = import3DS(filename.c_str(), actualModel);
//	else if (0 == ext.compare(L".obj"))
//		cg_err = importOBJ(filename.c_str(), actualModel);
//	else
//		throw exception("Object file format not supported");
//
//	if (cg_err != CG_IMPORT_OK)
//		throw exception("Could not load model");
//
//
//	// Build a buffer for each mesh
//
//	// Generate a single buffer object that stores a CGVertexExt struct per vertex
//	// Each CGPolyMesh in actualModel is stored contiguously in the buffer.
//	// The indices are also stored in the same way but no offset to each vertex sub-buffer is added.
//	// Model stores a vector of base vertex offsets to point to the start of each sub-buffer and start index offsets for each sub-mesh
//
//	numMeshes = actualModel->getMeshCount();
//
//	if (numMeshes == 0)
//		throw exception("Empty model loaded");
//
//	uint32_t numVertices = 0;
//	uint32_t numIndices = 0;
//
//	for (uint32_t i = 0; i < numMeshes; ++i) {
//
//		// Store base vertex index;
//		baseVertexOffset.push_back(numVertices);
//
//		CGPolyMesh *M = actualModel->getMeshAtIndex(i);
//
//		if (M) {
//
//			// Increment vertex count
//			numVertices += M->vertexCount();
//
//			// Store num indices for current mesh
//			indexCount.push_back(M->faceCount() * 3);
//			numIndices += M->faceCount() * 3;
//		}
//	}
//
//
//	// Create vertex buffer
//	_vertexBuffer = (ExtendedVertexStruct*)malloc(numVertices * sizeof(ExtendedVertexStruct));
//
//	if (!_vertexBuffer)
//		throw exception("Cannot create vertex buffer");
//
//
//	// Create index buffer
//	_indexBuffer = (uint32_t*)malloc(numIndices * sizeof(uint32_t));
//
//	if (!_indexBuffer)
//		throw exception("Cannot create index buffer");
//
//
//	// Copy vertex data into single buffer
//	ExtendedVertexStruct *vptr = _vertexBuffer;
//	uint32_t *indexPtr = _indexBuffer;
//
//	for (uint32_t i = 0; i < numMeshes; ++i) {
//
//		// Get mesh data (assumes 1:1 correspondance between vertex position, normal and texture coordinate data)
//		CGPolyMesh *M = actualModel->getMeshAtIndex(i);
//
//		if (M) {
//
//			CGBaseMeshDefStruct R;
//			M->createMeshDef(&R);
//
//			for (uint32_t k = 0; k < uint32_t(R.N); ++k, vptr++) {
//
//				vptr->pos = XMFLOAT3(R.V[k].x, R.V[k].y, R.V[k].z);
//				vptr->normal = XMFLOAT3(R.Vn[k].x, R.Vn[k].y, R.Vn[k].z);
//
//				//Flip normal.x for OBJ & GSF (might be required for other files too?)
//				if (0 == ext.compare(L".obj") || 0 == ext.compare(L".gsf"))// || 0 == ext.compare(L".3ds")
//				{
//					vptr->normal.x = -vptr->normal.x;
//					vptr->pos.x = -vptr->pos.x;
//				}
//				if (R.Vt && R.VtSize > 0)
//					vptr->texCoord = XMFLOAT2(R.Vt[k].s, 1.0f - R.Vt[k].t);
//				else
//					vptr->texCoord = XMFLOAT2(0.0f, 0.0f);
//
//				vptr->matDiffuse = materials[0]->getColour()->diffuse;//XMCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
//				vptr->matSpecular = materials[0]->getColour()->specular;// XMCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
//			}
//
//			// Copy mesh indices from CGPolyMesh into buffer
//			memcpy(indexPtr, R.Fv, R.n * sizeof(CGFaceVertex));
//
//			// Re-order indices to account for DirectX using the left-handed coordinate system
//			for (int k = 0; k < R.n; ++k, indexPtr += 3)
//				swap(indexPtr[0], indexPtr[2]);
//		}
//	}
//
//
//	//
//	// Setup DX vertex buffer interfaces
//	//
//
//	D3D11_BUFFER_DESC vertexDesc;
//	D3D11_SUBRESOURCE_DATA vertexData;
//
//	ZeroMemory(&vertexDesc, sizeof(D3D11_BUFFER_DESC));
//	ZeroMemory(&vertexData, sizeof(D3D11_SUBRESOURCE_DATA));
//
//	vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
//	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	vertexDesc.ByteWidth = numVertices * sizeof(ExtendedVertexStruct);
//	vertexData.pSysMem = _vertexBuffer;
//
//	HRESULT hr = device->CreateBuffer(&vertexDesc, &vertexData, &vertexBuffer);
//
//	if (!SUCCEEDED(hr))
//		throw exception("Vertex buffer cannot be created");
//
//
//	// Setup index buffer
//	D3D11_BUFFER_DESC indexDesc;
//	D3D11_SUBRESOURCE_DATA indexData;
//
//	ZeroMemory(&indexDesc, sizeof(D3D11_BUFFER_DESC));
//	ZeroMemory(&indexData, sizeof(D3D11_SUBRESOURCE_DATA));
//
//	indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
//	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
//	indexDesc.ByteWidth = numIndices * sizeof(uint32_t);
//	indexData.pSysMem = _indexBuffer;
//
//	hr = device->CreateBuffer(&indexDesc, &indexData, &indexBuffer);
//
//	if (!SUCCEEDED(hr))
//		throw exception("Index buffer cannot be created");
//
//
//	// Dispose of local resources
//	free(_vertexBuffer);
//	free(_indexBuffer);
//	actualModel->release();
//	}
//	catch (exception& e)
//	{
//		cout << "Model could not be instantiated due to:\n";
//		cout << e.what() << endl;
//
//		if (_vertexBuffer)
//			free(_vertexBuffer);
//
//		if (_indexBuffer)
//			free(_indexBuffer);
//
//		if (actualModel)
//			actualModel->release();
//		return-1;
//	}
//	return 0;
//}

//HRESULT SkinnedModel::loadModelAssimp(ID3D11Device *device, const std::wstring& filename)
//{
//	ExtendedVertexStruct *_vertexBuffer = nullptr;
//	uint32_t *_indexBuffer = nullptr;
//
//	Assimp::Importer importer;
//	std::wstring w(filename); 
//	std::string filename_string(w.begin(), w.end());
//	// Get filename extension
//	wstring ext = filename.substr(filename.length() - 4);
//	//if (0 == ext.compare(L".obj") || 0 == ext.compare(L".gsf"))// || 0 == ext.compare(L".3ds")
//	//printf("OBJ\n", ext);
//
//	try
//	{
//		const aiScene* scene = importer.ReadFile(filename_string, aiProcess_PreTransformVertices| aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |
//			aiProcess_Triangulate |
//			aiProcess_JoinIdenticalVertices |
//			aiProcess_SortByPType);
//		//printf("Root transform=",scene->mRootNode->)
//
//
//		if (!scene)
//		{
//			printf("Couldn't load model - Error Importing Asset");
//			return false;
//		}
//
//		numMeshes = scene->mNumMeshes;
//
//		if (numMeshes == 0)
//			throw exception("Empty model loaded");
//
//		uint32_t numVertices = 0;
//		uint32_t numIndices = 0;
//		//printf("Num Meshes %d\n", scene->mNumMeshes);
//		for (uint32_t k = 0; k < numMeshes; ++k)
//		{
//			aiMesh* mesh = scene->mMeshes[k];
//			//Store base vertex index;
//			baseVertexOffset.push_back(numVertices);
//			// Increment vertex count
//			numVertices += mesh->mNumVertices;
//			// Store num indices for current mesh
//			indexCount.push_back(mesh->mNumFaces * 3);
//			numIndices += mesh->mNumFaces * 3;
//			//printf("Mesh %d\n", k);
//			//printf("numVertices %d\n", mesh->mNumVertices);
//			//printf("numIndices %d\n", mesh->mNumFaces);
//		}
//
//			// Create vertex buffer
//		_vertexBuffer = (ExtendedVertexStruct*)malloc(numVertices * sizeof(ExtendedVertexStruct));
//
//			if (!_vertexBuffer)
//				throw exception("Cannot create vertex buffer");
//
//			// Create index buffer
//			_indexBuffer = (uint32_t*)malloc(numIndices * sizeof(uint32_t));
//
//			if (!_indexBuffer)
//				throw exception("Cannot create index buffer");
//
//			// Copy vertex data into single buffer
//			ExtendedVertexStruct *vptr = _vertexBuffer;
//			uint32_t *indexPtr = _indexBuffer;
//
//			for (uint32_t i = 0; i < numMeshes; ++i) 
//			{
//
//				aiMesh* mesh = scene->mMeshes[i];
//				//printf("Mesh %d\n", i);
//				//if (mesh->HasNormals())
//				//	printf("has normals");
//				uint32_t j = 0;
//				for ( j = 0; j < mesh->mNumFaces; ++j)		
//				{
//					const aiFace& face = mesh->mFaces[j];
//					for (int k = 0; k < 3; ++k)
//					{
//						int VIndex = baseVertexOffset[i] + face.mIndices[k];
//						aiVector3D pos = mesh->mVertices[face.mIndices[k]];
//						aiVector3D uv = mesh->mTextureCoords[0][face.mIndices[k]];
//						aiVector3D normal = mesh->HasNormals() ? mesh->mNormals[face.mIndices[k]] : aiVector3D(1.0f, 1.0f, 1.0f);
//						//Flip normal.x for OBJ & GSF (might be required for other files too?)
//						if (0 == ext.compare(L".obj") || 0 == ext.compare(L".gsf"))// || 0 == ext.compare(L".3ds")
//						{
//							normal.x = -normal.x;
//
//							
//							pos.x = -pos.x;
//						}
//						vptr[VIndex].pos = XMFLOAT3(pos.x, pos.y, pos.z);
//						vptr[VIndex].normal = XMFLOAT3(normal.x, normal.y, normal.z);
//						vptr[VIndex].texCoord = XMFLOAT2(uv.x, 1-uv.y);
//						vptr[VIndex].matDiffuse = material[]->getColour()->diffuse;//XMCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
//						vptr[VIndex].matSpecular = material->getColour()->specular;// XMCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
//						indexPtr[0] = face.mIndices[k];
//						indexPtr++;
//						/*printf("index %d = %d \n", j * 3 + k, face.mIndices[k]);*/
//					}
//				}//for each face
//				//printf("faces %d\n", j);
//			}//for each mesh
//
//	
//			// Setup DX vertex buffer interfaces
//			D3D11_BUFFER_DESC vertexDesc;
//			D3D11_SUBRESOURCE_DATA vertexData;
//
//			ZeroMemory(&vertexDesc, sizeof(D3D11_BUFFER_DESC));
//			ZeroMemory(&vertexData, sizeof(D3D11_SUBRESOURCE_DATA));
//
//			vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
//			vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//			vertexDesc.ByteWidth = numVertices * sizeof(ExtendedVertexStruct);
//			vertexData.pSysMem = _vertexBuffer;
//
//			HRESULT hr = device->CreateBuffer(&vertexDesc, &vertexData, &vertexBuffer);
//
//			if (!SUCCEEDED(hr))
//				throw exception("Vertex buffer cannot be created");
//
//			// Setup index buffer
//			D3D11_BUFFER_DESC indexDesc;
//			D3D11_SUBRESOURCE_DATA indexData;
//
//			ZeroMemory(&indexDesc, sizeof(D3D11_BUFFER_DESC));
//			ZeroMemory(&indexData, sizeof(D3D11_SUBRESOURCE_DATA));
//
//			indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
//			indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
//			indexDesc.ByteWidth = numIndices * sizeof(uint32_t);
//			indexData.pSysMem = _indexBuffer;
//
//			hr = device->CreateBuffer(&indexDesc, &indexData, &indexBuffer);
//
//			if (!SUCCEEDED(hr))
//				throw exception("Index buffer cannot be created");
//
//			// Dispose of local resources
//			if (_vertexBuffer)
//			free(_vertexBuffer);
//			if (_vertexBuffer)
//			free(_indexBuffer);
//
//			//printf("done\n");
//
//		}
//		catch (exception& e)
//		{
//			cout << "Model could not be instantiated due to:\n";
//			cout << e.what() << endl;
//
//			if (_vertexBuffer)
//				free(_vertexBuffer);
//
//			if (_indexBuffer)
//				free(_indexBuffer);
//
//			return-1;
//		}
//
//	return 0;
//}

//Model::Model(ID3D11Device *device, Effect *_effect, const std::wstring& filename, ID3D11ShaderResourceView *tex_view, Material *_material) {
//
//	Num_Textures = 1;
//	load(device, _effect, filename,  _material);
//	// Setup texture interfaces
//	textureResourceViewArray[0] = tex_view;
//	if (textureResourceViewArray[0])
//		textureResourceViewArray[0]->AddRef();
//
//
//}
//
//Model::Model(ID3D11Device *device, Effect *_effect, const std::wstring& filename, ID3D11ShaderResourceView *_tex_view_array[], int _num_textures, Material *_material) {
//
//	load(device, _effect, filename,  _material);
//	setTextures(0,_num_textures, _tex_view_array);
//	//Num_Textures = min(MAX_TEXTURES, _num_textures);
//	//for (int i = 0; i < Num_Textures; i++)
//	//	textureResourceViewArray[i] = _tex_view_array[i];
//	
//
//
////}
//void Model::setTextures(int _start_slot, int _num_textures, ID3D11ShaderResourceView *_tex_view_array[]){
//	if (_start_slot > MAX_TEXTURES - 1 | _start_slot <0 | _num_textures<1)return;
//	int num_textures = min(_num_textures + _start_slot, MAX_TEXTURES) - _start_slot;
//	Num_Textures = max(num_textures + _start_slot, Num_Textures);
//
//	for (int i = _start_slot; i < num_textures + _start_slot; i++)
//		textureResourceViewArray[i] = _tex_view_array[i - _start_slot];
//};



//
//XMMATRIX Model::update(ID3D11DeviceContext *context, Camera *camera, double time) {
//	// //Update castle cBuffer
//	cBufferCPU->worldMatrix = update(time);
//	cBufferCPU->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferCPU->worldMatrix));
//	cBufferCPU->WVPMatrix = cBufferCPU->worldMatrix*camera->getViewMatrix() * *(camera->getProjMatrix());
//	mapCbuffer(context, cBufferCPU, cBufferGPU,sizeof(CBufferModel));
//	return worldMatrix;
//}
//
//XMMATRIX Model::update(ID3D11DeviceContext *context, Camera *camera) {
//	// //Update castle cBuffer
//	cBufferCPU->worldMatrix = worldMatrix;
//	cBufferCPU->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferCPU->worldMatrix));
//	cBufferCPU->WVPMatrix = cBufferCPU->worldMatrix*camera->getViewMatrix() * *(camera->getProjMatrix());
//	mapCbuffer(context, cBufferCPU, cBufferGPU, sizeof(CBufferModel));
//	return worldMatrix;
//}

//
//void Model::renderBasic(ID3D11DeviceContext *context) {
//
//
//
//	// Validate Model before rendering (see notes in constructor)
//	if (!context || !vertexBuffer || !indexBuffer || !effect)
//		return;
//
//	// Set vertex layout
//	context->IASetInputLayout(effect->getVSInputLayout());
//
//	// Set Model vertex and index buffers for IA
//	ID3D11Buffer* vertexBuffers[] = { vertexBuffer };
//	UINT vertexStrides[] = { sizeof(DXVertexExt) };
//	UINT vertexOffsets[] = { 0 };
//
//	context->IASetVertexBuffers(0, 1, vertexBuffers, vertexStrides, vertexOffsets);
//	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
//
//	// Set primitive topology for IA
//	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//
//	context->PSSetSamplers(0, 1, &sampler);
//	// Apply the cBuffer.
//	context->VSSetConstantBuffers(4, 1, &cBufferGPU);
//	context->PSSetConstantBuffers(4, 1, &cBufferGPU);
//
//
//	// Draw Model
//	for (uint32_t indexOffset = 0, i = 0; i < numMeshes; indexOffset += indexCount[i], ++i)
//		context->DrawIndexed(indexCount[i], indexOffset, baseVertexOffset[i]);
//}