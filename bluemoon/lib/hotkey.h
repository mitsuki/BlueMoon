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

class CHotKey
{
public:
    CHotKey(HWND hwnd) : m_hwnd{ hwnd }, m_ids{} {}
    ~CHotKey()
    {
        Clear();
        m_hwnd = nullptr;
    }

    bool IsRegistered(int id)
    {
        return std::find(m_ids.begin(), m_ids.end(), id) != m_ids.end();
    }

    bool Check(int id, UINT mod, UINT key)
    {
        if (::RegisterHotKey(m_hwnd, id, mod, key))
        {
            if (::UnregisterHotKey(m_hwnd, id))
            {
                return true;
            }
        }
        return false;
    }

    bool Add(int id, UINT mod, UINT key)
    {
        Remove(id);
        if (::RegisterHotKey(m_hwnd, id, mod, key))
        {
            m_ids.push_back(id);
            return true;
        }
        return false;
    }

    void Remove(int id)
    {
        auto it = std::find(m_ids.begin(), m_ids.end(), id);
        if (it != m_ids.end())
        {
            if (::UnregisterHotKey(m_hwnd, id))
            {
                m_ids.erase(it);
            }
        }
    }

    void Clear(void)
    {
        while (!m_ids.empty())
        {
            Remove(m_ids.back());
        }
    }

protected:
    HWND m_hwnd;
    std::list<int> m_ids;
};
