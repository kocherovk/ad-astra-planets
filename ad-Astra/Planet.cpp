#include "Planet.h"
#include <d3d11.h>
#include <d3dx11.h>

#pragma optimize ("g", off)

Planet::Planet(float radius, XMFLOAT4 color, float orbitRotationAngle, float orbitRotationSpeed, float selfRotationSpeed, float orbitRadius, ID3D11VertexShader* VS, ID3D11PixelShader* PS) : 
	_radius(radius), _color(color), _orbitRorationSpeed(orbitRotationSpeed),
	_selfRotatiinSpeed(selfRotationSpeed), _orbitRadius(orbitRadius),
	_t(GetTickCount()), _orbitRotationAngle(orbitRotationAngle), _VS(VS), _PS(PS) {};

Planet::Planet() : _center(nullptr), _radius(1),  _color(XMFLOAT4(0.5,0.5,0.5,1)) {};

float Planet::getRadius()
{
	return _radius;
}

XMFLOAT3 Planet::getPosition()
{
	return _position;
}

XMMATRIX Planet::getWorld()
{

	return XMMatrixTranspose(XMMatrixScaling(_radius, _radius, _radius) * XMMatrixTranslation(_position.x, _position.y, _position.z));
}

XMMATRIX Planet::getRotation()
{
	return XMMatrixRotationY(_selfRotationAngle);
}

XMFLOAT4 Planet::getColor()
{
	return _color;
}

void Planet::setOrbitCenter(Planet* center)
{
	this->_center = center;
	step();
}

ID3D11VertexShader* Planet::getVertexShader()
{
	return _VS;
}

ID3D11PixelShader* Planet::getPixelShader()
{
	return _PS;
}

void Planet::step()
{
	if (!_center) return;

	DWORD t = GetTickCount();
	float delta = (t - _t) * TIME_STAMP;

	_orbitRotationAngle += _orbitRorationSpeed * delta;
	_selfRotationAngle += _selfRotatiinSpeed * delta;
	_position.x = _center->getPosition().x + _orbitRadius * cos(_orbitRotationAngle); 
	_position.z = _center->getPosition().z + _orbitRadius * sin(_orbitRotationAngle);
	
    _t = t;
}