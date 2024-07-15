#include "App.h"
#include <random>
#include <chrono>

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

const int width = 1280, height = 1280;

App* app = nullptr;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> dis(1, 3);
std::uniform_int_distribution<int> dis2(0, 360);
int main()
{
    

    WNDCLASSEX wc = { sizeof(WNDCLASSEX),
                     CS_CLASSDC,
                     WndProc,
                     0L,
                     0L,
                     GetModuleHandle(NULL),
                     NULL,
                     NULL,
                     NULL,
                     NULL,
                     L"QuadTree", // lpszClassName, L-string
                     NULL };

    RegisterClassEx(&wc);

    // ���ٱ��� ������ ������ ��ü �ػ󵵰� �ƴ϶�
    // �츮�� ������ �׸��� �ػ󵵰� width x height�� �ǵ���
    // �����츦 ���� �ػ󵵸� �ٽ� ����ؼ� CreateWindow()���� ���

    // �츮�� ���ϴ� �׸��� �׷��� �κ��� �ػ�
    RECT wr = { 0, 0, width, height };

    // �ʿ��� ������ ũ��(�ػ�) ���
    // wr�� ���� �ٲ�
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    wstring hwndName = L"QuadTree";
    // �����츦 ���鶧 ������ ����� wr ���
    HWND hwnd = CreateWindow(wc.lpszClassName, hwndName.c_str(),
        WS_OVERLAPPEDWINDOW,
        100, // ������ ���� ����� x ��ǥ
        100, // ������ ���� ����� y ��ǥ
        wr.right - wr.left, // ������ ���� ���� �ػ�
        wr.bottom - wr.top, // ������ ���� ���� �ػ�
        NULL, NULL, wc.hInstance, NULL);

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    app = new App(hwnd, width, height);
    if (!app->Initialize())
    {
        delete app;
        return 0;
    }

    double timeAcc = 0.0;
    // Main message loop
    MSG msg = {};
    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

            app->Update();
            app->Render();

            // switch the back buffer and the front buffer
            app->swapChain->Present(1, 0);

            std::chrono::duration<double>sec = std::chrono::system_clock::now() - start;

            timeAcc += sec.count();
            if (timeAcc > 1.0)
            {
                timeAcc = 0.0;
                wstring time = std::to_wstring(sec.count());
                wstring circles = std::to_wstring(app->circleArray.size());
                hwndName = L"Time : " + time + L", Object Count : " + circles;
                SetWindowText(hwnd, hwndName.c_str());
            }
        }
    }

    delete app;
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    return 0;
}

// Windows procedure
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    int integerX = GET_X_LPARAM(lParam);
    int integerY = GET_Y_LPARAM(lParam);

    float x = integerX * 2.0f / width - 1.0f;
    float y = -integerY * 2.0f / height + 1.0f;
    // Ŀ���� ȭ�� ������ ������ ��� ���� ����
    // ���ӿ����� Ŭ������ ���� ���� �ֽ��ϴ�.
    Vector2 pos(std::clamp(x, -1.0f, 1.0f), std::clamp(y, -1.0f, 1.0f));

    Vector2 speed;
    float amplifier;

    switch (msg) {
    case WM_SIZE:
        // Reset and resize swapchain
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_MOUSEMOVE:
        // std::cout << "Mouse " << LOWORD(lParam) << " " << HIWORD(lParam) <<
        // std::endl;
        break;
    case WM_LBUTTONUP:
        // std::cout << "WM_LBUTTONUP Left mouse button" << std::endl;
        break;
    case WM_RBUTTONUP:
        // std::cout << "WM_RBUTTONUP Right mouse button" << std::endl;
        break;
    case WM_KEYDOWN:
        //std::cout << "WM_KEYDOWN " << (int)wParam << std::endl;
        break;
    case WM_LBUTTONDOWN:
        for (int i = 0; i < 10; ++i)
        {
            amplifier = float(dis2(gen));
            speed.x = cos(amplifier);
            speed.y = sin(amplifier);
            speed.Normalize();
            app->AddCicle(pos, dis(gen) / 200.f, speed);
        }
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }

    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}