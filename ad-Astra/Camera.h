#include <math.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>

#pragma once

class ObservingCamera
{
private:
	XMMATRIX _projection;
	XMMATRIX _view;

	XMFLOAT3 _position;

public:
	ObservingCamera();

	void update();
	XMFLOAT3 getPosition() const;
	XMMATRIX getViewMatrix() const;
	XMMATRIX getProjectionMatrix() const;
};