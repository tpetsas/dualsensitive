
#define UNICODE
#define _UNICODE
#include <dualsensitive.h>
#include <windows.h>
#include <shellapi.h>
#include <thread>

#include "resource.h"
// Tray icon definitions
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT    1001
#define ID_TRAY_ENABLE  1002
#define ID_TRAY_DISABLE 1003
#define TRAY_UID        1337

NOTIFYICONDATAW g_nid = {};
HINSTANCE g_hInstance;
HMENU g_hMenu;
HWND g_hWnd;

void updateTrayIcon() {
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = g_hWnd;
    g_nid.uID = TRAY_UID;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(101)); // IDI_ICON1
    wcscpy_s(g_nid.szTip, L"Dualsensitive Service");
    Shell_NotifyIcon(NIM_ADD, &g_nid);
}

void updateMenuState() {
    CheckMenuItem(g_hMenu, ID_TRAY_ENABLE, MF_BYCOMMAND | (dualsensitive::isEnabled() ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(g_hMenu, ID_TRAY_DISABLE, MF_BYCOMMAND | (!dualsensitive::isEnabled() ? MF_CHECKED : MF_UNCHECKED));
}

void showContextMenu() {
    POINT pt;
    GetCursorPos(&pt);

    updateMenuState();
    SetForegroundWindow(g_hWnd);
    TrackPopupMenu(g_hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, g_hWnd, NULL);
}


// Basic window procedurej
LRESULT CALLBACK WndProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP) {
				showContextMenu();
            }
            break;
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case ID_TRAY_ENABLE:
                    dualsensitive::enable();
					updateMenuState();
                    break;
                case ID_TRAY_DISABLE:
                    dualsensitive::reset();
                    dualsensitive::disable();
					updateMenuState();
                    break;
                case ID_TRAY_EXIT:
                    Shell_NotifyIconW(NIM_DELETE, &g_nid);
                    PostQuitMessage(0);
                    break;
            }
            break;
		case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &g_nid);
            PostQuitMessage(0);
			break;
    }

    return DefWindowProc(g_hWnd, msg, wParam, lParam);
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    g_hInstance = hInstance;
    // Register window class
    const wchar_t CLASS_NAME[] = L"DualSensitive";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    g_hWnd = CreateWindowEx(
        0, CLASS_NAME, L"DualSensitive", 0,
        0, 0, 0, 0, HWND_MESSAGE, nullptr, hInstance, nullptr
    );

// Setup menu
    g_hMenu = CreatePopupMenu();
    AppendMenu(g_hMenu, MF_STRING, ID_TRAY_ENABLE, L"Enable Adaptive Triggers");
    AppendMenu(g_hMenu, MF_STRING, ID_TRAY_DISABLE, L"Disable Adaptive Triggers");
    AppendMenu(g_hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(g_hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");

    updateTrayIcon();
    updateMenuState();

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
