#include "QuadTree.h"
#include "Circle.h"

QuadTree::QuadTree(int indepth, float inscale, Vector2 incenter)
	: depth(indepth),
	center(incenter),
	scale(inscale)
{
	
}

void QuadTree::Initialize(ComPtr<ID3D11Device>& device, vector<shared_ptr<QuadTree>>& treeArray)
{
	rect.top = center.y + scale * 0.5f;
	rect.right = center.x + scale * 0.5f;
	rect.bottom = center.y - scale * 0.5f;
	rect.left = center.x - scale * 0.5f;

	treeArray.push_back(shared_from_this());
	vertices.push_back(std::move(Vertex(Vector3(rect.left, rect.bottom, 0.f), Colors::Green)));
	vertices.push_back(std::move(Vertex(Vector3(rect.left, rect.top, 0.f), Colors::Green)));
    vertices.push_back(std::move(Vertex(Vector3(rect.right, rect.top, 0.f), Colors::Green)));
    vertices.push_back(std::move(Vertex(Vector3(rect.right, rect.bottom, 0.f), Colors::Green)));

    indices.push_back(0); indices.push_back(1);
    indices.push_back(1); indices.push_back(2);
    indices.push_back(2); indices.push_back(3);
    indices.push_back(3); indices.push_back(0);

	CreateVBandIB(device);

    if (depth - 1 == 0)
    {
        bLeaf = true;
        return;
    }

    childs[NW] = std::make_shared<QuadTree>(depth - 1, scale * 0.5f, (Vector2(float(rect.left), float(rect.top)) + center) * 0.5f);
    childs[NE] = std::make_shared<QuadTree>(depth - 1, scale * 0.5f, (Vector2(float(rect.right), float(rect.top)) + center) * 0.5f);
    childs[SE] = std::make_shared<QuadTree>(depth - 1, scale * 0.5f, (Vector2(float(rect.right), float(rect.bottom)) + center) * 0.5f);
    childs[SW] = std::make_shared<QuadTree>(depth - 1, scale * 0.5f, (Vector2(float(rect.left), float(rect.bottom)) + center) * 0.5f);

	for (auto& child : childs) child->Initialize(device, treeArray);
}

bool QuadTree::AddCircle(const shared_ptr<Circle>& circle)
{
    int checkCollisionResult = CheckCollision(circle);

    if (checkCollisionResult == COL_OUTSIDE) return false;

    ++NumCircles;
    bRender = true;
    circles.push_back(circle);

    // 리프노드거나 비어있으면
    if (bLeaf || NumCircles == 1)
    {
        circle->Owners.push_back(shared_from_this());
        return true;
    }

    // 리프가 아님 
    list<shared_ptr<Circle>>::iterator iter = circles.begin();
    for (; iter != circles.end();)
    {
        int NumChildCollision = 0;
        for (int i = 0; i < 4; ++i)
        {
            if (childs[i]->AddCircle(*iter)) ++NumChildCollision;
        }

        if (NumChildCollision > 0) iter = circles.erase(iter);
        else ++iter;
    }

    return true;
}

void QuadTree::Render(ComPtr<ID3D11DeviceContext>& context)
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(),
        &stride, &offset);
    context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT,
        0);
    context->DrawIndexed(UINT(indices.size()), 0, 0);
}

void QuadTree::CreateVBandIB(ComPtr<ID3D11Device>& device)
{   // vertex buffer
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

int QuadTree::CheckCollision(const shared_ptr<Circle>& circle)
{
    Vector2 circlePos = circle->pos;
    float radius = circle->radius;

    if (circlePos.x - radius >= rect.left && circlePos.x + radius <= rect.right &&
        circlePos.y - radius >= rect.bottom && circlePos.y + radius <= rect.top)
    {
        return COL_INSIDE;
    }

    Vector2 Nearest;
    Nearest.x = std::clamp(circle->pos.x, rect.left, rect.right);
    Nearest.y = std::clamp(circle->pos.y, rect.bottom, rect.top);

    float length = (Nearest - circlePos).Length();
    if (length < circle->radius)
    {
        return COL_INTERSECT;
    }
    else
    {
        return COL_OUTSIDE;
    }
}

void QuadTree::RenderAllChilds(bool bRender)
{
    if (childs[0] == nullptr) return;

    for (auto& child : childs)
        child->bRender = bRender;
}
