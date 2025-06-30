
#define UNICODE
#define _UNICODE
#include <dualsensitive.h>
#include <windows.h>
#include <shellapi.h>
#include <thread>
#include <atomic>

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

std::thread monitorThread;
std::atomic<bool> monitorRunning = true;

void setTrayIcon() {
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = g_hWnd;
    g_nid.uID = TRAY_UID;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON_ACTIVE));
    wcscpy_s(g_nid.szTip, L"Dualsensitive Service");
    Shell_NotifyIcon(NIM_ADD, &g_nid);
}

void updateTrayIcon() {
    g_nid.hIcon = LoadIcon(g_hInstance,
            MAKEINTRESOURCE (
                dualsensitive::isEnabled() ? IDI_ICON_ACTIVE : IDI_ICON_MUTED
            )
    );
    Shell_NotifyIcon(NIM_MODIFY, &g_nid);
}

void updateMenuState() {
     if (dualsensitive::isEnabled()) {
        EnableMenuItem(g_hMenu, ID_TRAY_ENABLE, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(g_hMenu, ID_TRAY_DISABLE, MF_BYCOMMAND | MF_ENABLED);
        CheckMenuItem(g_hMenu, ID_TRAY_ENABLE, MF_BYCOMMAND | MF_CHECKED);
        CheckMenuItem(g_hMenu, ID_TRAY_DISABLE, MF_BYCOMMAND | MF_UNCHECKED);
    } else {
        EnableMenuItem(g_hMenu, ID_TRAY_ENABLE, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(g_hMenu, ID_TRAY_DISABLE, MF_BYCOMMAND | MF_GRAYED);
        CheckMenuItem(g_hMenu, ID_TRAY_ENABLE, MF_BYCOMMAND | MF_UNCHECKED);
        CheckMenuItem(g_hMenu, ID_TRAY_DISABLE, MF_BYCOMMAND | MF_CHECKED);
    }
}

void showContextMenu() {
    POINT pt;
    GetCursorPos(&pt);

    updateMenuState();
    SetForegroundWindow(g_hWnd);

    HMENU hMenu = g_hMenu;

    int cmd = TrackPopupMenu (
        hMenu,
        TPM_RETURNCMD | TPM_NONOTIFY | TPM_BOTTOMALIGN | TPM_LEFTALIGN,
        pt.x, pt.y, 0, g_hWnd, NULL
    );
    // if a command was selected, dispatch it manually
    if (cmd != 0) {
        PostMessage(g_hWnd, WM_COMMAND, cmd, 0);
    }
}


// Basic window procedurej
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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
                    updateTrayIcon();
                    updateMenuState();
                    break;
                case ID_TRAY_DISABLE:
                    dualsensitive::reset();
                    dualsensitive::disable();
                    updateTrayIcon();
                    updateMenuState();
                    break;
                case ID_TRAY_EXIT:
                    // Remove tray icon BEFORE window is destroyed to avoid ghosting
                    monitorRunning = false;
                    Shell_NotifyIcon(NIM_DELETE, &g_nid);
                    PostMessage(g_hWnd, WM_CLOSE, 0, 0);
                    break;
            }
            break;
        case WM_CLOSE:
            DestroyWindow(g_hWnd); // triggers WM_DESTROY
            break;
        case WM_DESTROY:
            monitorRunning = false;
            dualsensitive::reset();
            dualsensitive::terminate();
            if (monitorThread.joinable()) monitorThread.join();
            PostQuitMessage(0);
            break;
        case WM_QUERYENDSESSION:
        case WM_ENDSESSION:
            if (wParam == TRUE) {  // Session is ending
                dualsensitive::reset();
            }
            break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}


bool isProcessAlive(DWORD pid) {
    HANDLE h = OpenProcess(SYNCHRONIZE, FALSE, pid);
    if (!h) return false;
    DWORD result = WaitForSingleObject(h, 0);
    CloseHandle(h);
    return result == WAIT_TIMEOUT;
}

void startMonitorThread() {
    monitorThread = std::thread([] {
        while (monitorRunning) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            DWORD pid = (DWORD) dualsensitive::getClientPid();
            if (pid && !isProcessAlive(pid)) {
                // Client process exited, shutting down DualSensitive service...
                PostMessage(g_hWnd, WM_CLOSE, 0, 0);
                break;
            }
        }
    });
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

    DWORD style = WS_OVERLAPPED;
    DWORD exStyle = WS_EX_TOOLWINDOW;

    g_hWnd = CreateWindowEx(
        exStyle, CLASS_NAME, L"DualSensitive", style,
        0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr
    );

    // Setup menu
    g_hMenu = CreatePopupMenu();
    AppendMenu (
            g_hMenu, MF_STRING, ID_TRAY_ENABLE,
            L"Enable Adaptive Triggers"
    );
    AppendMenu (
            g_hMenu, MF_STRING, ID_TRAY_DISABLE,
            L"Disable Adaptive Triggers"
    );
    AppendMenu(g_hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(g_hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");

    setTrayIcon();
    updateMenuState();

    // Start DualSensitive UDP server
    OutputDebugStringW(L"Starting Dualsensitive Service...\n");
    auto status = dualsensitive::init(AgentMode::SERVER, "dualsensitive-service.log", false);
    if (status != dualsensitive::Status::Ok) {
        OutputDebugStringW (
            L"Failed to initialize Dualsensitive in SERVER mode\n"
        );
    }
    updateTrayIcon();

    // Start monitor thread to make sure we shutdown the DualSensitive Service
    // when the client app turns off
    startMonitorThread();

    // Message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
