#include "Circle.h"

void Circle::Init(float _radius, Vector2 initialPos, Vector2 _speed, int segment, ComPtr<ID3D11Device>& device)
{
	radius = _radius;
	pos = initialPos;
    speed = _speed;

	float PI = 2.0f * 3.141592f;
	float delta = PI / float(segment);

    Vector2 start(radius, 0.f);
	Vertex vCenter;
	vCenter.pos = Vector3(0.f, 0.f, 0.f);
	vCenter.color = Colors::Aqua;
	vertices.push_back(vCenter);
	for (int i = 0; i < segment; ++i)
	{
		float theta = delta * float(i);
        Vector2 rotate(start.x * cos(theta) - start.y * sin(theta), start.x * sin(theta) + start.y * cos(theta));
		Vertex v;
		v.pos = Vector3(rotate.x, rotate.y, 0.f);
		v.color = Colors::Aqua;
		vertices.push_back(v);
	}

	for (int i = 1; i <= segment; ++i)
	{
		indices.push_back(0);
        indices.push_back(uint16_t((i % (vertices.size() - 1)) + 1));
        indices.push_back(i);
	}

	CreateVBandIB(device);
    CreateConstantBuffer(device);
}

void Circle::Render(ComPtr<ID3D11DeviceContext>& context)
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->VSSetConstantBuffers(0, 1, vertexConstantBuffer.GetAddressOf());
    context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(),
        &stride, &offset);
    context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT,
        0);
    context->DrawIndexed(UINT(indices.size()), 0, 0);
}

void Circle::Update(ComPtr<ID3D11DeviceContext>& context)
{
    vertexConstantBufferData.model = Matrix::CreateTranslation(Vector3(pos.x, pos.y, 0.f));
    vertexConstantBufferData.model = vertexConstantBufferData.model.Transpose();
    UpdateConstantBuffer(context);
}

void Circle::Move(const MyRect& rect)
{
    pos += speed / 300.f;

    static auto l = [](Vector2 n, Vector2 velocity) {
        Vector2 v1 = -velocity;
        Vector2 v2 = v1.Dot(n) * n;
        return 2 * v2 - v1; };

    if (pos.x - radius <= rect.left)
    {
        Vector2 n(1.f, 0.f);
        speed = l(n, speed);
    }
    else if (pos.x + radius >= rect.right)
    {
        Vector2 n(-1.f, 0.f);
        speed = l(n, speed);
    }
    else if (pos.y - radius <= rect.bottom)
    {
        Vector2 n(0.f, 1.f);
        speed = l(n, speed);
    }
    else if (pos.y + radius >= rect.top)
    {
        Vector2 n(0.f, -1.f);
        speed = l(n, speed);
    }

}

void Circle::CreateVBandIB(ComPtr<ID3D11Device>& device)
{
    D3D11_BUFFER_DESC vertexBufferDesc;
    ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
    vertexBufferDesc.ByteWidth = UINT(sizeof(Vertex) * vertices.size());
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
    vertexBufferDesc.StructureByteStride = sizeof(Vertex);

    D3D11_SUBRESOURCE_DATA vertexBufferData = {
        0 }; // MS 예제에서 초기화하는 방식
    vertexBufferData.pSysMem = vertices.data();
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;

    const HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &vertexBufferData,
        vertexBuffer.GetAddressOf());
    if (FAILED(hr)) {
        std::cout << "CreateBuffer() failed. " << std::hex << hr
            << std::endl;
    };

    // index buffer
    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
    indexBufferDesc.ByteWidth = UINT(sizeof(uint16_t) * indices.size());
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
    indexBufferDesc.StructureByteStride = sizeof(uint16_t);

    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = indices.data();
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;

    if (FAILED(device->CreateBuffer(&indexBufferDesc, &indexBufferData,
        indexBuffer.GetAddressOf())))
    {
        std::cout << "CreateBuffer() failed. " << std::hex << hr
            << std::endl;
    }
}

void Circle::CreateConstantBuffer(ComPtr<ID3D11Device>& device)
{
    D3D11_BUFFER_DESC cbDesc;
    cbDesc.ByteWidth = sizeof(vertexConstantBufferData);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbDesc.MiscFlags = 0;
    cbDesc.StructureByteStride = 0;

    // Fill in the subresource data.
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = &vertexConstantBufferData;
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;

    auto hr = device->CreateBuffer(&cbDesc, &initData,
        vertexConstantBuffer.GetAddressOf());
    if (FAILED(hr)) {
        std::cout << "CreateConstantBuffer() CreateBuffer failed()."
            << std::endl;
    }
}

void Circle::UpdateConstantBuffer(ComPtr<ID3D11DeviceContext>& context)
{
    D3D11_MAPPED_SUBRESOURCE ms;
    context->Map(vertexConstantBuffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    memcpy(ms.pData, &vertexConstantBufferData, sizeof(vertexConstantBufferData));
    context->Unmap(vertexConstantBuffer.Get(), NULL);
}
