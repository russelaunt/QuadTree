#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <string>
#include <algorithm>
#include <iostream>
#include <memory>
#include <directxtk/SimpleMath.h>
#include <windows.h>
#include <wrl/client.h>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector2;

class Circle;

struct MyRect
{
	float top;
	float right;
	float bottom;
	float left;
};
struct Vertex
{
	Vertex() : pos(Vector3()), color(Vector3()) {}
	Vertex(const Vector3& _pos, const Vector3& _color) : pos(_pos), color(_color) {}
	Vector3 pos;
	Vector3 color;
};

namespace Colors
{
	const Vector3 White = Vector3(1.f, 1.f, 1.f);
	const Vector3 Red = Vector3(1.f, 0.f, 0.f);
	const Vector3 Green = Vector3(0.f, 1.f, 0.f);
	const Vector3 Blue = Vector3(0.f, 0.f, 1.f);
	const Vector3 Aqua = Vector3(0.f, 1.f, 1.f);
}

using Microsoft::WRL::ComPtr;
using std::shared_ptr;
using std::unique_ptr;
using std::vector;
using std::list;
using std::wstring;
using std::set;
using std::map;

#define NW 0
#define NE 1
#define SE 2
#define SW 3

#define MAX_DENSITY 2

#define COL_INSIDE 0
#define COL_INTERSECT 1
#define COL_OUTSIDE 2

class QuadTree : public std::enable_shared_from_this<QuadTree>
{
public:
	QuadTree(int indepth, float inscale, Vector2 incenter);

	void Initialize(ComPtr<ID3D11Device>& device, vector<shared_ptr<QuadTree>>& treeArray);
	bool AddCircle(const shared_ptr<Circle>& circle);
	int CheckCollision(const shared_ptr<Circle>& circle);

	void Render(ComPtr<ID3D11DeviceContext>& context);
protected:
	void CreateVBandIB(ComPtr<ID3D11Device>& device);
	
	void RenderAllChilds(bool bRender);
public:
	shared_ptr<QuadTree> childs[4];
	vector<Vertex> vertices;
	vector<uint16_t> indices;

	list<shared_ptr<Circle>> circles;
	int NumCircles = 0;

	Vector2 center;
	MyRect rect;
	int depth;
	float scale;
	bool bRender = false;
	bool bLeaf = false;

	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
};

