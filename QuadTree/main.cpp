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

    // 툴바까지 포함한 윈도우 전체 해상도가 아니라
    // 우리가 실제로 그리는 해상도가 width x height가 되도록
    // 윈도우를 만들 해상도를 다시 계산해서 CreateWindow()에서 사용

    // 우리가 원하는 그림이 그려질 부분의 해상도
    RECT wr = { 0, 0, width, height };

    // 필요한 윈도우 크기(해상도) 계산
    // wr의 값이 바뀜
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    wstring hwndName = L"QuadTree";
    // 윈도우를 만들때 위에서 계산한 wr 사용
    HWND hwnd = CreateWindow(wc.lpszClassName, hwndName.c_str(),
        WS_OVERLAPPEDWINDOW,
        100, // 윈도우 좌측 상단의 x 좌표
        100, // 윈도우 좌측 상단의 y 좌표
        wr.right - wr.left, // 윈도우 가로 방향 해상도
        wr.bottom - wr.top, // 윈도우 세로 방향 해상도
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
    // 커서가 화면 밖으로 나갔을 경우 범위 조절
    // 게임에서는 클램프를 안할 수도 있습니다.
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