#include "stdafx.h"
#include "FirstPersonCamera.h"
using namespace DirectX;


//// constructor
//FirstPersonCamera::FirstPersonCamera(ID3D11Device *device) : Camera(device)
//{
//		dir = XMVectorSet(-1, 0, 1, 1);
//}
//
//FirstPersonCamera::FirstPersonCamera(ID3D11Device *device, DirectX::XMVECTOR init_pos, DirectX::XMVECTOR init_up, DirectX::XMVECTOR init_dir)
//{
//		pos = init_pos;
//		up = init_up;
//		dir = init_dir;
//}
//
//FirstPersonCamera::~FirstPersonCamera(void)
//{
//}
//XMMATRIX FirstPersonCamera::getViewMatrix() 
//{
//	lookAt = pos + dir;
//	return 		DirectX::XMMatrixLookAtLH(pos, lookAt, up);
//}
void FirstPersonCamera::move(float d)
{
	pos += (dir*d);
	lookAt = pos + dir;
}
void FirstPersonCamera::turn(float theta)
{
	//| x'| | x cos ? + z sin ? |
	//| y'|=|		  y			| 
	//| z'| |?x sin ? + z cos ?	|
	XMFLOAT3 dirF3;
	XMStoreFloat3(&dirF3, dir);
	dirF3.x = dirF3.x * cos(theta) + dirF3.z*sin(theta);
	dirF3.z = -dirF3.x * sin(theta) + dirF3.z*cos(theta);
	dir = XMLoadFloat3(&dirF3);

	lookAt = pos + dir;

}
void FirstPersonCamera::setHeight(float y)
{
	pos.vector4_f32[1]=y;
	lookAt = pos + dir;
}
void FirstPersonCamera::elevate(float theta) {

	//| x'| |		  x			|
	//| y'|=| y cos ? ? z sin ?	| 
	//| z'| | y sin ? + z cos ?	|
	XMFLOAT3 dirF3;
	XMStoreFloat3(&dirF3, dir);
	dirF3.y = dirF3.y * cos(theta) - dirF3.z*sin(theta);
	dirF3.z = dirF3.y * sin(theta) + dirF3.z*cos(theta);
	dir = XMLoadFloat3(&dirF3);

	lookAt = pos + dir;
}
