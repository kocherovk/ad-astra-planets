#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>

#pragma once

#define TIME_STAMP 1e-4;

class Planet
{
private:
	ID3D11VertexShader* _VS;
	ID3D11PixelShader* _PS;
	Planet* _center;
	XMFLOAT3 _position;
	XMFLOAT4 _color;
	float _radius;
	float _orbitRorationSpeed;
	float _selfRotatiinSpeed;
	float _orbitRotationAngle;
	float _selfRotationAngle;
	float _orbitRadius;
	DWORD _t;

public:
	Planet(float radius, XMFLOAT4 color, float orbitRotationAngle, float orbitRotationSpeed, float selfRotationSpeed, float orbitRadius, ID3D11VertexShader* VS, ID3D11PixelShader* PS);
	Planet();

	XMFLOAT3 getPosition();
	void setOrbitCenter(Planet* center);
	float getRadius();
	XMFLOAT4 getColor();
	XMMATRIX getWorld();
	XMMATRIX getRotation();
	ID3D11VertexShader* getVertexShader();
	ID3D11PixelShader* getPixelShader();
	void step();
};

