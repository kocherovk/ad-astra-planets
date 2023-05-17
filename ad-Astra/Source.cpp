#include <Windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <d3dcompiler.h>
#include <vector>
#include <dinput.h>
#include <cstdlib>
#include <string>
#include <Mesh.h>

#include "Planet.h"
#include "Camera.h"

#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "adAstra Game Libriary.lib")

#ifdef NDEBUG
#pragma optimize( "g", off )
#endif

using namespace std;
using namespace Game;
bool flag;

#pragma region Macro

#define HEIGHT 1024
#define WIDTH  1280
#define SLICES 50
#define STACKS 50
#define INDICES_COUNT (SLICES * STACKS * 6)
#define VERTICES_COUNT (SLICES * STACKS + 2)
#define STARS_COUNT 5000
#define STARS_R     1000

#define ORBIT_OFFSET 20
#define ORBIT_SCALE 10
#define SELF_ROTATION_SPEED_SCALE 0.3
#define ORBIT_ROTATION_SPEED_SCALE 0.1
#define RADIUS_SCALE 1

#define angle_speed(T) ( 2 * XM_2PI / T )
#define RANDOM_ORBIT_ANGLE  (XM_2PI * rand() / RAND_MAX)

#pragma endregion

#pragma region Globals

HINSTANCE               hInst = NULL;
HWND                    hWnd = NULL;
IDirectInput8*          directInput;           
IDirectInputDevice8*    keyboard;  
ID3D11Device*           device = NULL;
ID3D11DeviceContext*    deviceContext = NULL;
IDXGISwapChain*         swapChain = NULL;
ID3D11RenderTargetView* targetView = NULL;
ID3D11Texture2D*        depthStencil = NULL;
ID3D11DepthStencilView* depthStencilView = NULL;
ID3D11VertexShader*     vertexShader = NULL;
ID3D11VertexShader*     starsVertexShader = NULL;
ID3D11VertexShader*     vertexShaderNoise = NULL;
ID3D11PixelShader*      pixelShader = NULL;
ID3D11PixelShader*      starsPixelShader = NULL;
ID3D11PixelShader*      pixelShaderNoise = NULL;
ID3D11InputLayout*      vertexLayout = NULL;
ID3D11Buffer*           planetBuffer = NULL;
ID3D11Buffer*           starsBuffer = NULL;
ID3D11Buffer*           indexBuffer = NULL;
ID3D11Buffer*           constantBuffer = NULL;
XMMATRIX                view;
XMMATRIX                projection;

ID3D11Buffer*           monkeyVerts   = NULL;
ID3D11Buffer*           monkeyIndices = NULL;

vector<Planet>          planets;
ObservingCamera         camera;

#pragma endregion

#pragma region Data Structures

struct ConstantBuffer
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
	XMMATRIX rotation;

	XMFLOAT4 color;
	XMFLOAT4 noiseColor;
};

#pragma endregion

#pragma region Forward Declarations

LRESULT WINAPI manageMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
HRESULT initializeWindow(HINSTANCE hInstance, int nCmdShow);
void initializeWorld();
HRESULT initializeDirectX11();
HRESULT createDeviceAndSwapChain();
HRESULT setPipelineOutput();
HRESULT initializeShaders();
void handleKeyboardInput();
HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
HRESULT loadBuffers();
HRESULT loadMyBuffers();
bool checkMessages();
void gameLoop();
void nextUniverseState();
void renderScene();
void startRecord();
void cleanup();

#pragma endregion

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (FAILED(initializeWindow(hInstance, nCmdShow)))
	{
		MessageBox(hWnd, L"Failed to initialize window.", L"Critical Error", 0);
		return 0;
	}

	if (FAILED(initializeDirectX11()))
	{
		MessageBox(hWnd, L"Failed to initialize DirectX.", L"Critical Error", 0);
		return 0;
	}

	initializeWorld();

	gameLoop();
	return 0;
}

HRESULT initializeWindow(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	WNDCLASS  wc;
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = manageMessages;
	wc.cbClsExtra = wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) 6;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"GameWindow";
	RegisterClass(&wc);

	hWnd = CreateWindow(L"GameWindow", L"adAstra", WS_OVERLAPPEDWINDOW, 100, 100, WIDTH, HEIGHT, NULL, NULL, hInstance, NULL);

	if (!hWnd) return E_FAIL;
	ShowWindow(hWnd, nCmdShow);
	return S_OK;
}

HRESULT initializeDirectX11()
{
	createDeviceAndSwapChain();
	setPipelineOutput();
	initializeShaders();
	loadMyBuffers();
	srand(GetTickCount());

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -3.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	view = XMMatrixLookAtLH(Eye, At, Up);

	// Initialize the projection matrix
	projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, WIDTH / (FLOAT) HEIGHT, 0.01f, 100000.0f);

	D3D11_RASTERIZER_DESC wfdesc;

	ID3D11RasterizerState* WireFrame;

	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_SOLID;
	wfdesc.CullMode = D3D11_CULL_NONE;
	device->CreateRasterizerState(&wfdesc, &WireFrame);
	deviceContext->RSSetState(WireFrame);

	DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**) &directInput, NULL);
	directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);

	keyboard->SetDataFormat(&c_dfDIKeyboard);
	keyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	keyboard->Acquire();

	return S_OK;
}

void initializeWorld()
{
	planets = vector<Planet>();
	planets.reserve(100);
	camera = ObservingCamera();
	camera.update();

	planets.push_back(Planet(RADIUS_SCALE * 15.0f, XMFLOAT4(1.0f, 0.75f, 0.0f, 1.0f), RANDOM_ORBIT_ANGLE, 0, SELF_ROTATION_SPEED_SCALE * 0.01f, 0, vertexShaderNoise, pixelShaderNoise)); //Sun
	planets.push_back(Planet(RADIUS_SCALE * 0.382f, XMFLOAT4(0.58f, 0.29f, 0.0f, 1.0f), RANDOM_ORBIT_ANGLE, ORBIT_ROTATION_SPEED_SCALE * angle_speed(0.241f), SELF_ROTATION_SPEED_SCALE * 58.6f, ORBIT_OFFSET + ORBIT_SCALE * 0.38f, vertexShader, pixelShader)); // Mercury
	planets.push_back(Planet(RADIUS_SCALE * 0.949f, XMFLOAT4(0.6f, 0.4f, 0.4f, 1.0f), RANDOM_ORBIT_ANGLE, ORBIT_ROTATION_SPEED_SCALE * angle_speed(0.615f), SELF_ROTATION_SPEED_SCALE * 243.0f, ORBIT_OFFSET + ORBIT_SCALE * 0.72f, vertexShader, pixelShader));        // Venus
	planets.push_back(Planet(RADIUS_SCALE * 1, XMFLOAT4(0.2f, 0.2f, 0.7f, 1.0f), RANDOM_ORBIT_ANGLE, ORBIT_ROTATION_SPEED_SCALE, SELF_ROTATION_SPEED_SCALE, ORBIT_OFFSET + ORBIT_SCALE, vertexShader, pixelShader));          // Earth
	planets.push_back(Planet(RADIUS_SCALE * 0.53f, XMFLOAT4(0.9f, 0.1f, 0.1f, 1.0f), RANDOM_ORBIT_ANGLE, ORBIT_ROTATION_SPEED_SCALE * angle_speed(1.88f), SELF_ROTATION_SPEED_SCALE * 1.03f, ORBIT_OFFSET + ORBIT_SCALE * 1.52f, vertexShader, pixelShader));    // Mars
	planets.push_back(Planet(RADIUS_SCALE * 11.2f, XMFLOAT4(0.6f, 0.4f, 0.2f, 1.0f), RANDOM_ORBIT_ANGLE, ORBIT_ROTATION_SPEED_SCALE * angle_speed(11.86f), SELF_ROTATION_SPEED_SCALE * 0.414f, ORBIT_OFFSET + ORBIT_SCALE * 5.2f, vertexShader, pixelShader));       // Jupiter
	planets.push_back(Planet(RADIUS_SCALE * 9.41f, XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f), RANDOM_ORBIT_ANGLE, ORBIT_ROTATION_SPEED_SCALE * angle_speed(29.46f), SELF_ROTATION_SPEED_SCALE * 0.426f, ORBIT_OFFSET + ORBIT_SCALE * 9.54f, vertexShader, pixelShader));     // Saturn
	planets.push_back(Planet(RADIUS_SCALE * 3.98f, XMFLOAT4(0.9f, 0.1f, 0.1f, 1.0f), RANDOM_ORBIT_ANGLE, ORBIT_ROTATION_SPEED_SCALE * angle_speed(84.01f), SELF_ROTATION_SPEED_SCALE * 0.718f, ORBIT_OFFSET + ORBIT_SCALE * 19.22f, vertexShader, pixelShader));     // Uranus
	planets.push_back(Planet(RADIUS_SCALE * 3.81f, XMFLOAT4(0.1f, 0.1f, 0.9f, 1.0f), RANDOM_ORBIT_ANGLE, ORBIT_ROTATION_SPEED_SCALE * angle_speed(164.79f), SELF_ROTATION_SPEED_SCALE * 0.671f, ORBIT_OFFSET + ORBIT_SCALE * 30.06f, vertexShader, pixelShader));    // Neptune

	planets.push_back(Planet(RADIUS_SCALE * 0.2f, XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), 0, 10, 1.5, 3, vertexShader, pixelShader));    // Moon

	for (int i = 0; i < 9; i++)
		planets[i].setOrbitCenter(&planets[0]);

	planets[9].setOrbitCenter(&planets[3]);

}

HRESULT createDeviceAndSwapChain()
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	return D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION,
		                                 &sd, &swapChain, &device, NULL, &deviceContext);
}

HRESULT setPipelineOutput()
{
	ID3D11Texture2D* backBuffer = NULL;
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*) &backBuffer);

	device->CreateRenderTargetView(backBuffer, NULL, &targetView);
	backBuffer->Release();

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = WIDTH;
	descDepth.Height = HEIGHT;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	device->CreateTexture2D(&descDepth, NULL, &depthStencil);

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(depthStencil, &descDSV, &depthStencilView);

	deviceContext->OMSetRenderTargets(1, &targetView, depthStencilView);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT) WIDTH;
	vp.Height = (FLOAT) HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	deviceContext->RSSetViewports(1, &vp);

	return S_OK;
}

HRESULT initializeShaders()
{
	// Light vertex shader
	ID3DBlob* pVSBlob = NULL;
	if (FAILED(CompileShaderFromFile(L"shader.fx", "VS", "vs_5_0", &pVSBlob)))
		MessageBox(hWnd, L"Failed to load vertex shader", L"Error", 0);
	device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &vertexShader);

	// Light pixel shader
	ID3DBlob* pPSBlob = NULL;
	if (FAILED(CompileShaderFromFile(L"shader.fx", "PS", "ps_5_0", &pPSBlob)))
		MessageBox(hWnd, L"Failed to load pixel shader", L"Error", 0);
	device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &pixelShader);
	pPSBlob->Release();

	// Noisy vertexShader
	ID3DBlob* pVSNoiseBlob = NULL;
	if (FAILED(CompileShaderFromFile(L"NoisyShader.fx", "VS", "vs_5_0", &pVSNoiseBlob)))
		MessageBox(hWnd, L"Failed to load vertex shader", L"Error", 0);
	device->CreateVertexShader(pVSNoiseBlob->GetBufferPointer(), pVSNoiseBlob->GetBufferSize(), NULL, &vertexShaderNoise);
	pVSNoiseBlob->Release();

	// Noisy pixel shader
	ID3DBlob* pPSNoiseBlob = NULL;
	if (FAILED(CompileShaderFromFile(L"NoisyShader.fx", "PS", "ps_5_0", &pPSNoiseBlob)))
		MessageBox(hWnd, L"Failed to load pixel shader", L"Error", 0);
	device->CreatePixelShader(pPSNoiseBlob->GetBufferPointer(), pPSNoiseBlob->GetBufferSize(), NULL, &pixelShaderNoise);
	pPSNoiseBlob->Release();

	// Stars vertexShader
	ID3DBlob* pVSStarsBlob = NULL;
	if (FAILED(CompileShaderFromFile(L"starsShader.fx", "VS", "vs_5_0", &pVSStarsBlob)))
		MessageBox(hWnd, L"Failed to load vertex shader", L"Error", 0);
	device->CreateVertexShader(pVSStarsBlob->GetBufferPointer(), pVSStarsBlob->GetBufferSize(), NULL, &starsVertexShader);
	pVSStarsBlob->Release();

	// Stars pixel shader
	ID3DBlob* pPSStarsBlob = NULL;
	if (FAILED(CompileShaderFromFile(L"starsShader.fx", "PS", "ps_5_0", &pPSStarsBlob)))
		MessageBox(hWnd, L"Failed to load pixel shader", L"Error", 0);
	device->CreatePixelShader(pPSStarsBlob->GetBufferPointer(), pPSStarsBlob->GetBufferSize(), NULL, &starsPixelShader);
	pPSStarsBlob->Release();


	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{

		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * sizeof(float), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	device->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &vertexLayout);
	pVSBlob->Release();

	// Set the input layout
	deviceContext->IASetInputLayout(vertexLayout);

	return S_OK;
}

HRESULT CompileShaderFromFile(WCHAR* filename, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** blob)
{
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	ID3DBlob* errorBlob;

	HRESULT hr = D3DX11CompileFromFile(filename, NULL, NULL, entryPoint, shaderModel,
		                               dwShaderFlags, 0, NULL, blob, &errorBlob, NULL);
	if (FAILED(hr))
	{
		if (errorBlob != NULL)
			OutputDebugStringA((char*) errorBlob->GetBufferPointer());
		if (errorBlob) errorBlob->Release();
		return hr;
	}
	if (errorBlob) errorBlob->Release();

	return S_OK;
}

HRESULT loadMyBuffers()
{
	Vertex* planetVertices;
	Vertex* starsVertices;
	WORD* indices;

	vector <Vertex> mVerts;
	vector <WORD>   mindices;

	createSphere(planetVertices, indices, SLICES, STACKS);
	loadStars(starsVertices, STARS_COUNT, STARS_R);
	loadMeshFromFile("monkey.ply", mVerts, mindices);

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * VERTICES_COUNT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = planetVertices;
	device->CreateBuffer(&bd, &InitData, &planetBuffer);

	bd.ByteWidth = sizeof(Vertex) * STARS_COUNT;
	InitData.pSysMem = starsVertices;
	device->CreateBuffer(&bd, &InitData, &starsBuffer);

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * INDICES_COUNT;       
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	device->CreateBuffer(&bd, &InitData, &indexBuffer);

	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	device->CreateBuffer(&bd, NULL, &constantBuffer);

	return S_OK;
}

void gameLoop()
{
	while (checkMessages())
	{
		keyboard->Acquire();
		handleKeyboardInput();
		
		nextUniverseState();
		
		renderScene();
	}
	cleanup();
}

void nextUniverseState()
{
	for (int i = 0; i < planets.size(); i++)
	{
		planets[i].step();
	}
	camera.update();
}

void handleKeyboardInput()
{
	char buffer[256];
	keyboard->GetDeviceState(sizeof(buffer), buffer);

	if (buffer[DIK_ESCAPE])
		PostQuitMessage(0);

	if (buffer[DIK_LCONTROL] && buffer[DIK_R])
		flag = true;
}

void startRecord()
{
	ID3D11Texture2D* buffer, *image = NULL;
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*) &buffer);

	if (image == NULL)
	{
		D3D11_TEXTURE2D_DESC td;
		buffer->GetDesc(&td);
		device->CreateTexture2D(&td, NULL, &image);
	}

	deviceContext->CopyResource(image, buffer);
	HRESULT hr = D3DX11SaveTextureToFile(deviceContext, image, D3DX11_IFF_JPG, L"ad-Astra.jpg");
}

void renderScene()
{
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	deviceContext->ClearRenderTargetView(targetView, ClearColor);
	deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	
	deviceContext->VSSetShader(vertexShaderNoise, NULL, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
	deviceContext->PSSetShader(pixelShaderNoise, NULL, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &constantBuffer);

	ConstantBuffer cb1;
	cb1.view = XMMatrixTranspose(camera.getViewMatrix());
	cb1.projection = XMMatrixTranspose(projection);
	
	deviceContext->VSSetShader(vertexShader, NULL, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
	deviceContext->PSSetShader(pixelShader, NULL, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &constantBuffer);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &planetBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	
	for (int i = 0; i < planets.size(); i++)
	{
		cb1.world = planets[i].getWorld();
		cb1.rotation = planets[i].getRotation();
		cb1.color = planets[i].getColor();

		deviceContext->VSSetShader(planets[i].getVertexShader(), NULL, 0);
		deviceContext->PSSetShader(planets[i].getPixelShader(), NULL, 0);

		deviceContext->UpdateSubresource(constantBuffer, 0, NULL, &cb1, 0, 0);
		deviceContext->DrawIndexed(INDICES_COUNT, 0, 0);
	}

	deviceContext->VSSetShader(starsVertexShader, NULL, 0);
	deviceContext->PSSetShader(starsPixelShader, NULL, 0);

	deviceContext->IASetVertexBuffers(0, 1, &starsBuffer, &stride, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	deviceContext->Draw(STARS_COUNT, 0);

	cb1.world = XMMatrixIdentity();
	cb1.rotation = XMMatrixIdentity();
	cb1.color = XMFLOAT4(1.0f, 0.75f, 0.0f, 1.0f);
	deviceContext->UpdateSubresource(constantBuffer, 0, NULL, &cb1, 0, 0);

	if (flag) startRecord();

	swapChain->Present(0, 0);
}

void cleanup()
{
	if (deviceContext) deviceContext->ClearState();

	if (keyboard) keyboard->Release();
	if (directInput) directInput->Release();
	if (constantBuffer) constantBuffer->Release();
	if (planetBuffer) planetBuffer->Release();
	if (starsBuffer) starsBuffer->Release();
	if (indexBuffer) indexBuffer->Release();
	if (vertexLayout) vertexLayout->Release();
	if (vertexShader) vertexShader->Release();
	if (vertexShaderNoise) vertexShaderNoise->Release();
	if (pixelShader) pixelShader->Release();
	if (pixelShaderNoise) pixelShaderNoise->Release();
	if (depthStencil) depthStencil->Release();
	if (depthStencilView) depthStencilView->Release();
	if (targetView) targetView->Release();
	if (swapChain) swapChain->Release();
	if (deviceContext) deviceContext->Release();
	if (device) device->Release();
}

bool checkMessages()
{
	MSG msg;
	if (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return false;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}

LRESULT WINAPI manageMessages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}