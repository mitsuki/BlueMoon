/*
SPDX-License-Identifier: BSD 2-Clause

BlueMoon

Copyright (c) 2021 mitsuki@engawa.org
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
*/

#include "bluemoon.h"
#include "lib/util.h"
#include "lib/notifyicon.h"
#include "lib/monitor.h"
#include "lib/hotkey.h"
#include "lib/gdip.h"


const TCHAR gszTargetTitle[] = TEXT("umamusume");

#define MAX_LOADSTRING 128
TCHAR gszTitle[MAX_LOADSTRING];
HINSTANCE ghInstance;

AppSetting gSetting;
CNotifyIcon* gpNotify;
CHotKey* gpHotKey;
CMonitor gMonitor;
CGdip gGdip;

static void adjustWindow(HWND hwnd)
{
    HWND target = FindWindow(nullptr, gszTargetTitle);

    if (target)
    {
        // register screenshot hot key
        if (gSetting.ssKey != Option3way::OFF &&
            !gpHotKey->IsRegistered(IDH_SCREENSHOT))
        {
            gpHotKey->Add(IDH_SCREENSHOT, 0, VK_SNAPSHOT);
        }

        // monitor select
        CMonitorInfo mon;
        if (gSetting.monitor == 0)
        {
            // use current
            gMonitor.FindMonitor(target, &mon);
        }
        else if (gSetting.monitor <= gMonitor.Count())
        {
            // use specified
            gMonitor.FindMonitor(gSetting.monitor, &mon);
        }else
        {
            // use primary
            gMonitor.FindMonitor(1, &mon);
        }

        POINT pos;
        SIZE size;
        CRect crect;
        CRect drect;
        CRect wrect;
        if (mon.m_handle &&
            GetClientRect(target, &crect) &&
            GetWindowRect(target, &wrect) &&
            DwmGetWindowAttribute(target, DWMWA_EXTENDED_FRAME_BOUNDS, &drect, sizeof(CRect)) == S_OK)
        {
            CRect mrect{ mon.m_info.rcWork };
            
            // get window spec
            SIZE diff1{ drect.width() - crect.width() , drect.height() - crect.height() };
            SIZE diff2{ wrect.width() - drect.width() , wrect.height() - drect.height() };

            // check aspect ratio
            double mar = (double)mrect.width() / mrect.height();
            double war = (double)wrect.width() / wrect.height();
            LONG chnew, cwnew;
            if (mar > war)
            {
                // fit to vertical side
                chnew = mrect.height() - diff1.cy;
                cwnew = chnew * (war > 1.0f ? 16 : 9) / (war > 1.0 ? 9 : 16);
                size.cx = cwnew + diff1.cx + diff2.cx;
                size.cy = chnew + diff1.cy + diff2.cy;
                pos.x = gSetting.horizontal == FitHorizontal::RIGHT ?
                    mrect.right - size.cx + diff2.cx / 2 :
                    mrect.left - diff2.cx / 2;
                pos.y = mrect.top/* - diff2.cy / 2 */;
            }
            else
            {
                // fit to horizontal side
                cwnew = mrect.width() - diff1.cx;
                chnew = cwnew * (war > 1.0f ? 9 : 16) / (war > 1.0 ? 16 : 9);
                size.cx = cwnew + diff1.cx + diff2.cx;
                size.cy = chnew + diff1.cy + diff2.cy;
                pos.x = mrect.left - diff2.cx / 2;
                pos.y = gSetting.vertical == FitVertical::BOTTOM ?
                    mrect.bottom - size.cy + diff2.cy :
                    mrect.top/* - diff2.cy / 2 */;
            }
            // crect is content area rectangle and left and top are zero.
            // drect is DWM window rectangle and is window area as just it looks.
            // wrect is window rectangle and is including window shadow area.
            // These sizes are: wrect > drect > crect
            // Left posision in SetWindowPos() is effected by window shadow area, but top is not.

            SetWindowPos(target,
                gSetting.alwaysTop == Option2way::ON ? HWND_TOPMOST : HWND_NOTOPMOST,
                pos.x, pos.y, size.cx, size.cy, SWP_NOACTIVATE);
        }
    }
    else
    {
        // unregister screenshot hot key
        if (gSetting.ssKey != Option3way::OFF &&
            gpHotKey->IsRegistered(IDH_SCREENSHOT))
        {
            gpHotKey->Remove(IDH_SCREENSHOT);
        }
    }
}

static void onTimer(HWND hwnd, UINT id)
{
    if (id == IDT_MAIN)
    {
        adjustWindow(hwnd);
    }
}

static void onNotifyIcon(HWND hwnd, UINT id, UINT msg)
{
    if (id == gpNotify->GetID())
    {
        switch (msg)
        {
        case WM_LBUTTONDBLCLK:
            DestroyWindow(hwnd);
            break;
        }
    }
}

static void onHotKey(HWND hwnd, int id, UINT mod, UINT key)
{
    if (id == IDH_SCREENSHOT)
    {
        HWND target = FindWindow(nullptr, gszTargetTitle);
        if (target)
        {
            HDC sdc = GetDC(nullptr);
            HDC ddc = CreateCompatibleDC(nullptr);
            CRect rect;

            if (sdc && ddc && GetClientRect(target, &rect))
            {
                rect.ToScreen(target);

                LPBYTE* pbytes;
                BITMAPINFO bi
                {
                    sizeof(BITMAPINFOHEADER),
                    rect.width(), -rect.height(), 1, 24, BI_RGB,
                    0, 0, 0, 0, 0
                };
                HBITMAP hdib = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, (VOID**)&pbytes, nullptr, 0);
                if (hdib)
                {
                    HBITMAP old = SelectBitmap(ddc, hdib);
                    if (BitBlt(ddc, 0, 0, rect.width(), rect.height(), sdc, rect.left, rect.top, SRCCOPY))
                    {
                        SYSTEMTIME tm;
                        PWSTR kpath;
                        WCHAR path[MAX_PATH]{}, name[32]{};

                        GetLocalTime(&tm);
                        
                        SHGetKnownFolderPath(FOLDERID_Pictures, 0, nullptr, &kpath);
                        StringCchCat(path, MAX_PATH, kpath);
                        CoTaskMemFree(kpath);
                        
                        StringCchPrintf(name, 32, TEXT("\\%04d%02d%02d_%02d%02d%02d%03d.png"),
                            tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds);
                        StringCchCat(path, MAX_PATH, name);

                        CLSID encoder;
                        if (gGdip.GetImageEncoder(CGdip::Mime::PNG, &encoder))
                        {
                            Gdiplus::Bitmap bmp{ &bi, pbytes };
                            if (bmp.Save(path, &encoder) == Gdiplus::Ok)
                            {
                                if (gSetting.ssKey == Option3way::EX)
                                {
                                    TCHAR msg[64];
                                    StringCchPrintf(msg, 64, TEXT("[%04d%02d%02d_%02d%02d%02d%03d.png] is saved."),
                                    tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds);
                                    gpNotify->SetInfo(msg, TEXT("Screenshot"));
                                    gpNotify->Update();
                                }
                            }
                        }
                    }
                    SelectBitmap(ddc, old);
                    DeleteObject(hdib);
                }
            }

            ReleaseDC(nullptr, ddc);
            ReleaseDC(nullptr, sdc);
        }
    }
}

static void onPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);
    EndPaint(hwnd, &ps);
}

static void onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDM_ABOUT:
        DialogBox(ghInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd,
            [](HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
                switch (message)
                {
                case WM_INITDIALOG:
                    return (INT_PTR)TRUE;
                case WM_COMMAND:
                    if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
                    {
                        EndDialog(hDlg, LOWORD(wParam));
                        return (INT_PTR)TRUE;
                    }
                    break;
                }
                return (INT_PTR)FALSE;
            });
        break;
    case IDM_EXIT:
        DestroyWindow(hwnd);
        break;
    }
}

static BOOL onCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
    gMonitor.Update();
    gpHotKey = new CHotKey(hwnd);
    if (gSetting.ssKey == Option3way::OFF ||
        gpHotKey->Check(IDH_SCREENSHOT, 0, VK_SNAPSHOT))
    {
        gpNotify = new CNotifyIcon(hwnd, ghInstance);
        gpNotify->SetTip(gszTitle);
        gpNotify->SetIconByID(IDI_BLUEMOON);
        gpNotify->Update();
        SetTimer(hwnd, IDT_MAIN, IDT_MAIN_INTERVAL, nullptr);
    }
    else
    {
        MessageBox(hwnd, TEXT("Faild to register screenshot key."), gszTitle, MB_OK);
        DestroyWindow(hwnd);
    }
    return TRUE;
}

static void onDestroy(HWND hwnd)
{
    KillTimer(hwnd, IDT_MAIN);
    delete gpNotify;
    delete gpHotKey;
    PostQuitMessage(0);
}

static LRESULT analyzeArgs(int argc, LPTSTR* argv)
{
    std::basic_string<TCHAR> msg;

    for (int i = 1; i < argc; i++)
    {
        TCHAR* p = argv[i];

        if (p[0] == TEXT('/'))
        {
            switch (p[1])
            {
            case TEXT('M'):
                if (TEXT('0') <= p[2] && p[2] <= TEXT('9') && p[3] == TEXT('\0'))
                {
                    gSetting.monitor = p[2] - TEXT('0');
                }
                else
                {
                    msg += TEXT("Invalid monitor number at /M option.\r\n");
                }
                break;
            case TEXT('L'):
                gSetting.horizontal = FitHorizontal::LEFT;
                break;
            case TEXT('R'):
                gSetting.horizontal = FitHorizontal::RIGHT;
                break;
            case TEXT('T'):
                gSetting.vertical = FitVertical::TOP;
                break;
            case TEXT('B'):
                gSetting.vertical = FitVertical::BOTTOM;
                break;
            case TEXT('S'):
                gSetting.ssKey =
                    p[2] == TEXT('0') ? Option3way::OFF :
                    p[2] == TEXT('2') ? Option3way::EX :
                    Option3way::ON;
                break;
            case TEXT('A'):
                gSetting.alwaysTop = p[2] == TEXT('0') ? Option2way::OFF : Option2way::ON;
                break;
            default:
                msg += TEXT("Unknown option [/");
                msg += p[1];
                msg += TEXT("]\r\n");
                break;
            }
        }
    }
    if (!msg.empty())
    {
        MessageBox(nullptr, msg.c_str(), gszTitle, MB_OK);
        return FALSE;
    }
    return TRUE;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    HANDLE_MSG(hwnd, WM_TIMER, onTimer);
    HANDLE_MSG(hwnd, WM_PAINT, onPaint);
    HANDLE_MSG(hwnd, WM_HOTKEY, onHotKey);
    HANDLE_MSG(hwnd, WM_NOTIFYICON, onNotifyIcon);
    HANDLE_MSG(hwnd, WM_COMMAND, onCommand);
    HANDLE_MSG(hwnd, WM_DESTROY, onDestroy);
    HANDLE_MSG(hwnd, WM_CREATE, onCreate);
    case WM_DISPLAYCHANGE:
    case WM_DPICHANGED:
        gMonitor.Update();
        break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY _tWinMain(
    HINSTANCE hinst,
    HINSTANCE hPrevInst,
    LPTSTR    lpCmdLine,
    int       nCmdShow)
{
    ghInstance = hinst;

    TCHAR szWindowClass[MAX_LOADSTRING];
    LoadString(ghInstance, IDS_APP_TITLE, gszTitle, MAX_LOADSTRING);
    LoadString(ghInstance, IDC_BLUEMOON, szWindowClass, MAX_LOADSTRING);

    if (analyzeArgs(__argc, __targv))
    {
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = ghInstance;
        wcex.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_BLUEMOON));
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = MAKEINTRESOURCE(IDC_BLUEMOON);
        wcex.lpszClassName = szWindowClass;
        wcex.hIconSm = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_BLUEMOON));

        if (RegisterClassEx(&wcex))
        {
            HWND hwnd = CreateWindow(
                szWindowClass, gszTitle, WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT, 300, 300,
                nullptr, nullptr, ghInstance, nullptr);

            if (hwnd)
            {
                ShowWindow(hwnd, SW_HIDE/*nCmdShow*/);
                UpdateWindow(hwnd);
                HACCEL hAccelTable = LoadAccelerators(hinst, MAKEINTRESOURCE(IDC_BLUEMOON));

                MSG msg;
                while (GetMessage(&msg, nullptr, 0, 0))
                {
                    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }
                return (int)msg.wParam;
            }
        }
    }
    return FALSE;
}
