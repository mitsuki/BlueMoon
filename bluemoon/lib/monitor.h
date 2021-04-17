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

#include "window.h"

class CMonitorInfo
{
public:
    HMONITOR m_handle;
    MONITORINFO m_info;

    CMonitorInfo() : m_handle{}, m_info{} {};
    CMonitorInfo(HMONITOR hmon)
    {
        m_handle = hmon;
        m_info.cbSize = sizeof(MONITORINFO);
        ::GetMonitorInfo(m_handle, &m_info);
    }

    CRect rcMonitor(void) { return CRect(m_info.rcMonitor); }
    CRect rcWork(void) { return CRect(m_info.rcWork); }
};

class CMonitor
{
public:
    static inline void Update(void)
    {
        m_monitors.clear();
        ::EnumDisplayMonitors(nullptr, nullptr, enumProc, 0);
    }
    
    static inline int Count(void)
    {
        return m_monitors.size();
    }

    static inline bool FindMonitor(size_t num, CMonitorInfo* mi)
    {
        if (1 <= num && num <= m_monitors.size())
        {
            *mi = m_monitors[num - 1];
            return true;
        }
        return false;
    }

    static inline bool FindMonitor(HMONITOR mon, CMonitorInfo* mi)
    {
        auto result = std::find_if(m_monitors.begin(), m_monitors.end(),
            [mon](CMonitorInfo v) {return v.m_handle == mon; });
        if (result != m_monitors.end())
        {
            *mi = *result;
            return true;
        }
        return false;
    }

    static inline bool FindMonitor(HWND hwnd, CMonitorInfo* mi)
    {
        return FindMonitor(
            ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), mi);
    }

protected:
    static inline std::vector<CMonitorInfo> m_monitors;

    static inline BOOL APIENTRY enumProc(HMONITOR hmon, HDC hdc, LPRECT lprect, LPARAM lparam)
    {
        m_monitors.emplace_back(hmon);
        return TRUE;
    }
};
