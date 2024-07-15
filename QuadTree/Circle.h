#pragma once

#include "QuadTree.h"

struct BasicVertexConstantData 
{
	Matrix model;
};

class Circle : public std::enable_shared_from_this<Circle>
{
public:
	void Init(float _radius, Vector2 initialPos, Vector2 _speed, int segment, ComPtr<ID3D11Device>& device);
	void Render(ComPtr<ID3D11DeviceContext>& context);
	void Update(ComPtr<ID3D11DeviceContext>& context);

	void Move(const MyRect& rect);

protected:
	void CreateVBandIB(ComPtr<ID3D11Device>& device);
	void CreateConstantBuffer(ComPtr<ID3D11Device>& device);
	void UpdateConstantBuffer(ComPtr<ID3D11DeviceContext>& context);

public:
	list<shared_ptr<QuadTree>> Owners;

	float radius;
	Vector2 pos;
	Vector2 speed;

	vector<Vertex> vertices;
	vector<uint16_t> indices;

	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;

	BasicVertexConstantData vertexConstantBufferData;
	ComPtr<ID3D11Buffer> vertexConstantBuffer;
};

