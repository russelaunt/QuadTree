#pragma comment(lib, "d3d11.lib")
#include "App.h"
#include "QuadTree.h"

App::App(HWND window, int width, int height)
    : screenWidth(width), screenHeight(height), mainWindow(window)
{
}

void App::Update()
{
    for (auto& circle : circleArray)
    {
        circle->Move(root->rect);
        circle->Update(context);
        circle->Owners.clear();
    }
    for (auto& tree : treeArray)
    {
        tree->circles.clear();
        tree->NumCircles = 0;
        tree->bRender = false;
    }
    for (auto& circle : circleArray)
    {
        root->AddCircle(circle);
    }
    for (auto& tree : treeArray)
    {
        list<shared_ptr<Circle>>::iterator iter_c1 = tree->circles.begin();
        list<shared_ptr<Circle>>::iterator iter_c2 = tree->circles.begin();
        for (; iter_c1 != tree->circles.end(); ++iter_c1)
        {
            for (; iter_c2 != tree->circles.end(); ++iter_c2)
            {
                if (*iter_c1 == *iter_c2) continue;

                shared_ptr<Circle> c1 = *iter_c1;
                shared_ptr<Circle> c2 = *iter_c2;

                float sumRadius = c1->radius + c2->radius;
                float distanceBetweenCenter = (c1->pos - c2->pos).Length();

                if (sumRadius >= distanceBetweenCenter)
                {
                    static auto l = [](Vector2 n, Vector2 velocity) {
                        Vector2 v1 = -velocity;
                        Vector2 v2 = v1.Dot(n) * n;
                        return 2 * v2 - v1; };

                    Vector2 n1 = c1->pos - c2->pos;
                    n1.Normalize();
                    c1->speed = l(n1, c1->speed);

                    Vector2 n2 = c2->pos - c1->pos;
                    n2.Normalize();
                    c2->speed = l(n2, c2->speed);
                }
            }
        }
    }
}

void App::Render()
{
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    context->ClearRenderTargetView(renderTargetView.Get(), clearColor);
    context->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);
    context->RSSetState(rasterizerState.Get());

    context->VSSetShader(treeVertexShader.Get(), 0, 0);
    context->PSSetShader(treePixelShader.Get(), 0, 0);

    context->IASetInputLayout(inputLayout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    // tree
    for (auto& node : treeArray)
    {
        if (node->bRender)
            node->Render(context);
    }

    context->VSSetShader(circleVertexShader.Get(), 0, 0);
    context->PSSetShader(circlePixelShader.Get(), 0, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // circle
    for (auto& circle : circleArray)
    {
        circle->Render(context);
    }
}

bool App::Initialize()
{
    if (!InitD3D()) return false;

    if (!InitShader()) return false;

    // quadtree
    root = std::make_shared<QuadTree>(7, 2.f, Vector2(0.f, 0.f));
    root->Initialize(device, treeArray);
    root->bRender = true;

    return true;
}

void App::AddCicle(Vector2 position, float radius, Vector2 speed)
{
    circleArray.push_back(std::make_shared<Circle>());
    circleArray.back()->Init(radius, position, speed, 20, device);
    root->AddCircle(circleArray.back());
}

bool App::InitD3D()
{
    const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;

    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ComPtr<ID3D11Device> tmpdevice;
    ComPtr<ID3D11DeviceContext> tmpcontext;

    const D3D_FEATURE_LEVEL featureLevels[2] = {
        D3D_FEATURE_LEVEL_11_0, // 더 높은 버전이 먼저 오도록 설정
        D3D_FEATURE_LEVEL_9_3 };
    D3D_FEATURE_LEVEL featureLevel;

    if (FAILED(D3D11CreateDevice(
        nullptr,    // Specify nullptr to use the default adapter.
        driverType, // Create a device using the hardware graphics driver.
        0, // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
        createDeviceFlags, // Set debug and Direct2D compatibility flags.
        featureLevels,     // List of feature levels this app can support.
        ARRAYSIZE(featureLevels), // Size of the list above.
        D3D11_SDK_VERSION,     // Always set this to D3D11_SDK_VERSION for
        // Microsoft Store apps.
        tmpdevice.GetAddressOf(), // Returns the Direct3D device created.
        &featureLevel,         // Returns feature level of device created.
        tmpcontext.GetAddressOf() // Returns the device immediate context.
    ))) {
        std::cout << "D3D11CreateDevice() failed." << std::endl;
        return false;
    }

    if (featureLevel != D3D_FEATURE_LEVEL_11_0) {
        std::cout << "D3D Feature Level 11 unsupported." << std::endl;
        return false;
    }

    // 4X MSAA 지원하는지 확인
    tmpdevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4,
        &numQualityLevels);
    if (numQualityLevels <= 0) {
        std::cout << "MSAA not supported." << std::endl;
    }

    // numQualityLevels = 0; // MSAA 강제로 끄기

    if (FAILED(tmpdevice.As(&device))) {
        std::cout << "device.AS() failed." << std::endl;
        return false;
    }

    if (FAILED(tmpcontext.As(&context))) {
        std::cout << "context.As() failed." << std::endl;
        return false;
    }

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferDesc.Width = screenWidth;   // set the back buffer width
    sd.BufferDesc.Height = screenHeight; // set the back buffer height
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // use 32-bit color
    sd.BufferCount = 2;                                // Double-buffering
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;

    // DXGI_USAGE_SHADER_INPUT 쉐이더에 입력으로 넣어주기 위해 필요
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = mainWindow; // the window to be used
    sd.Windowed = TRUE;             // windowed/full-screen mode
    sd.Flags =
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // allow full-screen switching
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    if (numQualityLevels > 0) {
        sd.SampleDesc.Count = 4; // how many multisamples
        sd.SampleDesc.Quality = numQualityLevels - 1;
    }
    else {
        sd.SampleDesc.Count = 1; // how many multisamples
        sd.SampleDesc.Quality = 0;
    }

    if (FAILED(D3D11CreateDeviceAndSwapChain(
        0, // Default adapter
        driverType,
        0, // No software device
        createDeviceFlags, featureLevels, 1, D3D11_SDK_VERSION, &sd,
        swapChain.GetAddressOf(), device.GetAddressOf(), &featureLevel,
        context.GetAddressOf()))) {
        std::cout << "D3D11CreateDeviceAndSwapChain() failed." << std::endl;
        return false;
    }

    tmpdevice->Release();
    tmpcontext->Release();

    // Set the viewport
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.TopLeftX = 0.f;
    viewport.TopLeftY = 0.f;
    viewport.Width = float(screenWidth);
    viewport.Height = float(screenHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f; // Note: important for depth buffering

    context->RSSetViewports(1, &viewport);

    ComPtr<ID3D11Texture2D> backBuffer;
    swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    if (backBuffer) {
        device->CreateRenderTargetView(backBuffer.Get(), nullptr,
            renderTargetView.GetAddressOf());
    }
    else {
        std::cout << "CreateRenderTargetView() failed." << std::endl;
        return false;
    }

    // Create a rasterizer state
    D3D11_RASTERIZER_DESC rastDesc;
    ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC)); // Need this
    rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    // rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
    rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.DepthClipEnable = true; // <- zNear, zFar 확인에 필요

    device->CreateRasterizerState(&rastDesc, rasterizerState.GetAddressOf());


    return true;
}

bool App::InitShader()
{
    if (!CreateVS()) return false;

    if (!CreatePS()) return false;

    return true;
}

bool App::CreateVS()
{
    vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
         {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
          D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3,
          D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    {
        ComPtr<ID3DBlob> shaderBlob;
        ComPtr<ID3DBlob> errorBlob;

        UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
        compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        HRESULT hr = D3DCompileFromFile(
            L"VS.hlsl", 0, 0, "main",
            "vs_5_0", 0, 0, &shaderBlob, &errorBlob);

        if (FAILED(hr)) return false;

        device->CreateVertexShader(shaderBlob->GetBufferPointer(),
            shaderBlob->GetBufferSize(), NULL,
            &treeVertexShader);

        device->CreateInputLayout(inputElements.data(), UINT(inputElements.size()),
            shaderBlob->GetBufferPointer(),
            shaderBlob->GetBufferSize(), &inputLayout);
    }

    {
        ComPtr<ID3DBlob> shaderBlob;
        ComPtr<ID3DBlob> errorBlob;

        UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
        compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        HRESULT hr = D3DCompileFromFile(
            L"circleVS.hlsl", 0, 0, "main",
            "vs_5_0", 0, 0, &shaderBlob, &errorBlob);

        if (FAILED(hr)) return false;

        device->CreateVertexShader(shaderBlob->GetBufferPointer(),
            shaderBlob->GetBufferSize(), NULL,
            &circleVertexShader);
    }

    return true;
}

bool App::CreatePS()
{
    {
        ComPtr<ID3DBlob> shaderBlob;
        ComPtr<ID3DBlob> errorBlob;

        UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
        compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        // 쉐이더의 시작점의 이름이 "main"인 함수로 지정
        // D3D_COMPILE_STANDARD_FILE_INCLUDE 추가: 쉐이더에서 include 사용
        HRESULT hr = D3DCompileFromFile(
            L"PS.hlsl", 0, 0, "main",
            "ps_5_0", 0, 0, &shaderBlob, &errorBlob);

        if (FAILED(hr)) return false;

        device->CreatePixelShader(shaderBlob->GetBufferPointer(),
            shaderBlob->GetBufferSize(), NULL, &treePixelShader);
    }

    {
        ComPtr<ID3DBlob> shaderBlob;
        ComPtr<ID3DBlob> errorBlob;

        UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
        compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        // 쉐이더의 시작점의 이름이 "main"인 함수로 지정
        // D3D_COMPILE_STANDARD_FILE_INCLUDE 추가: 쉐이더에서 include 사용
        HRESULT hr = D3DCompileFromFile(
            L"circlePS.hlsl", 0, 0, "main",
            "ps_5_0", 0, 0, &shaderBlob, &errorBlob);

        if (FAILED(hr)) return false;

        device->CreatePixelShader(shaderBlob->GetBufferPointer(),
            shaderBlob->GetBufferSize(), NULL, &circlePixelShader);
    }
 

    return true;
}

void App::CheckCircleBoundary()
{
}
