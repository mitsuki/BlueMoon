/*
SPDX-License-Identifier: BSD 2-Clause

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

#pragma once

#include <windows.h>
#include <shellapi.h>

#define WM_NOTIFYICON       (WM_APP + 0x0999)

/* void onNotifyIcon(HWND hwnd, UINT id, UINT msg) */
#define HANDLE_WM_NOTIFYICON(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (wParam), (lParam)), 0L)
#define FORWARD_WM_NOTIFYICON(hwnd, wParam, lParam, fn) \
    (void)(fn)((hwnd), WM_NOTIFYICON, (wParam), (lParam))

class CNotifyIcon
{
public:
    CNotifyIcon(
        HWND hwnd,
        HINSTANCE hinst,
        UINT id = 1000,
        UINT msg = WM_NOTIFYICON
    ){
        m_NotifyIcon = {};
        m_NotifyIcon.cbSize = sizeof(NOTIFYICONDATA);
        m_NotifyIcon.hWnd = hwnd;
        m_NotifyIcon.uID = id;
        m_NotifyIcon.uFlags = NIF_MESSAGE;
        m_NotifyIcon.uCallbackMessage = msg;
        m_hInst = hinst;
        m_bEnable = false;
    }
    virtual ~CNotifyIcon()
    {
        Release();
    }

    UINT GetID(void) { return m_NotifyIcon.uID; }
    
    void SetTip(LPCTSTR text)
    {
        ::StringCbCopy(m_NotifyIcon.szTip, sizeof(m_NotifyIcon.szTip), text);
        m_NotifyIcon.uFlags |= NIF_TIP;
    }

    void SetIconByHandle (HICON icon)
    {
        m_NotifyIcon.hIcon = icon;
        m_NotifyIcon.uFlags |= NIF_ICON;
    }
    void SetIconByID(UINT icon)
    {
        m_NotifyIcon.hIcon = (HICON)LoadIcon(m_hInst, MAKEINTRESOURCE(icon));
        m_NotifyIcon.uFlags |= NIF_ICON;
    }

    void SetInfo(LPCTSTR text, LPCTSTR title = _T(""), DWORD flag = NIIF_NONE)
    {
        ::StringCbCopy(m_NotifyIcon.szInfo,      sizeof(m_NotifyIcon.szInfo),      text);
        ::StringCbCopy(m_NotifyIcon.szInfoTitle, sizeof(m_NotifyIcon.szInfoTitle), title);
        m_NotifyIcon.dwInfoFlags = flag;
        m_NotifyIcon.uFlags |= NIF_INFO;
    }
    void ClearInfo(void)
    {
        SetInfo(TEXT(""));
    }

    BOOL Update(void)
    {
        BOOL ret = ::Shell_NotifyIcon(m_bEnable ? NIM_MODIFY : NIM_ADD, &m_NotifyIcon);
        m_bEnable = true;
        return ret;
    }

    BOOL Release(void)
    {
        if (m_bEnable) {
            m_bEnable = false;
            return ::Shell_NotifyIcon(NIM_DELETE, &m_NotifyIcon);
        }
        else {
            return 0;
        }
    }
    
protected:
    NOTIFYICONDATA      m_NotifyIcon;
    HINSTANCE           m_hInst;
    bool                m_bEnable;
};


