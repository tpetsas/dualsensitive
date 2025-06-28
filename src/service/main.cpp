
#define UNICODE
#define _UNICODE
#include <dualsensitive.h>
#include <windows.h>
#include <shellapi.h>
#include <thread>

// Tray icon definitions
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001

NOTIFYICONDATAW nid = {};
HWND hwnd = nullptr;

// Basic window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_TRAYICON) {
        if (lParam == WM_RBUTTONUP) {
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");

            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hwnd);
            TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, nullptr);
            DestroyMenu(hMenu);
        }
    } else if (msg == WM_COMMAND && LOWORD(wParam) == ID_TRAY_EXIT) {
        Shell_NotifyIconW(NIM_DELETE, &nid);
        PostQuitMessage(0);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    // Register window class
    const wchar_t CLASS_NAME[] = L"DualSensitive";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    hwnd = CreateWindowEx(
        0, CLASS_NAME, L"DualSensitive", 0,
        0, 0, 0, 0, HWND_MESSAGE, nullptr, hInstance, nullptr
    );

    // Add system tray icon
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(nullptr, IDI_INFORMATION);
    wcscpy_s(nid.szTip, L"Dualsensitive Service");
    Shell_NotifyIconW(NIM_ADD, &nid);

    // Start DualSensitive UDP server
    std::thread([]() {
        OutputDebugStringW(L"Starting Dualsensitive Service...\n");
        auto status = dualsensitive::init(AgentMode::SERVER);
        if (status != dualsensitive::Status::Ok) {
            OutputDebugStringW(L"Failed to initialize Dualsensitive in SERVER mode\n");
        }
    }).detach();

    // Message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    dualsensitive::terminate();
    return 0;
}
