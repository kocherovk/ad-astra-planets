#include "Camera.h"

ObservingCamera::ObservingCamera() 
{
	_projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, 16.0 / 9, 0.01f, 100000.0f);
	_position = XMFLOAT3(0,0,0);
};

void ObservingCamera::update()
{
	XMVECTOR pos;
	XMVECTOR lookAt;
	XMVECTOR up;

	DWORD t = GetTickCount();

	_position.z =  40 * sin(t * 2e-4);
	_position.x =  40 * cos(t * 2e-4);
	_position.y =  10 * sin(t * 2e-4);

	pos = XMVectorSet(_position.x, _position.y, _position.z, 0.0f);
	lookAt = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	_view = XMMatrixLookAtLH(pos, lookAt, up);
}

XMFLOAT3 ObservingCamera::getPosition() const
{
	return _position;
}

XMMATRIX ObservingCamera::getViewMatrix() const
{
	return _view;
}

XMMATRIX ObservingCamera::getProjectionMatrix() const
{
	return _projection;
}