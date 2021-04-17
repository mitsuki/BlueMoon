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

using namespace Gdiplus;


class CGdip
{
public:
    CGdip()
    {
        GdiplusStartup(&m_token, &m_info, nullptr);
    }
    ~CGdip()
    {
        Gdiplus::GdiplusShutdown(m_token);
    }
    
    bool GetImageEncoder(LPCWSTR name, CLSID* clsid)
    {
        bool ret = false;
        UINT count{}, bytes{};

        if (GetImageEncodersSize(&count, &bytes) == Ok)
        {
            std::unique_ptr<BYTE[]> buf{ new BYTE[bytes] };
            auto codecs = (ImageCodecInfo*)buf.get();
            if (GetImageEncoders(count, bytes, codecs) == Ok)
            {
                for (UINT i = 0; i < count; i++)
                {
                    if (lstrcmpW(name, codecs[i].MimeType) == 0)
                    {
                        *clsid = codecs[i].Clsid;
                        ret = true;
                        break;
                    }
                }
            }
        }
        return ret;
    }

    struct Mime
    {
        inline static LPCWSTR BMP = L"image/bmp";
        inline static LPCWSTR PNG = L"image/png";
        inline static LPCWSTR GIF = L"image/gif";
        inline static LPCWSTR JPEG = L"image/jpeg";
        inline static LPCWSTR TIFF = L"image/tiff";
    };

protected:
    ULONG_PTR m_token;
    GdiplusStartupInput m_info;
};
