#pragma once

#include "Circle.h"

#define NUM_CIRCLE 10

class App
{
public:
	App(HWND window, int width, int height);
	
	void Update();
	void Render();

	bool Initialize();

	void AddCicle(Vector2 position, float radius, Vector2 speed);

protected:
	bool InitD3D();
	bool InitShader();

	bool CreateVS();
	bool CreatePS();
	
	void CheckCircleBoundary();
public:
	int screenWidth, screenHeight;
	HWND mainWindow;
	UINT numQualityLevels = 0;

	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<IDXGISwapChain> swapChain;
	ComPtr<ID3D11RenderTargetView> renderTargetView;
	D3D11_VIEWPORT viewport;
	ComPtr<ID3D11RasterizerState> rasterizerState;

	shared_ptr<QuadTree> root;
	vector<shared_ptr<QuadTree>> treeArray;
	vector<shared_ptr<Circle>> circleArray;

	// tree ·»´õ¿ë
	ComPtr<ID3D11VertexShader> treeVertexShader;
	ComPtr<ID3D11PixelShader> treePixelShader;
	ComPtr<ID3D11InputLayout> inputLayout;

	// circle ·»´õ¿ë
	ComPtr<ID3D11VertexShader> circleVertexShader;
	ComPtr<ID3D11PixelShader> circlePixelShader;
};

