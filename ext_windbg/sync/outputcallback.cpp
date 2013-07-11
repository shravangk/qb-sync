/*
Copyright (C) 2012-2013, Quarkslab.

This file is part of qb-sync.

qb-sync is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 Based on out.cpp from WinDDK's dumpstk sample
*/

#include <dbgeng.h>
#include "outputcallbacks.h"
#include "sync.h"
#include "tunnel.h"

#define MAX_CMD 4096

StdioOutputCallbacks g_OutputCb;
bool g_OutputCbLocal = false;

STDMETHODIMP
StdioOutputCallbacks::QueryInterface(
    THIS_
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    )
{
    *Interface = NULL;

    if (IsEqualIID(InterfaceId, __uuidof(IUnknown)) ||
        IsEqualIID(InterfaceId, __uuidof(IDebugOutputCallbacks)))
    {
        *Interface = (IDebugOutputCallbacks *)this;
        AddRef();
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG)
StdioOutputCallbacks::AddRef(THIS)
{
    return 1;
}

STDMETHODIMP_(ULONG)
StdioOutputCallbacks::Release(THIS)
{
    return 0;
}

STDMETHODIMP
StdioOutputCallbacks::Output(
    THIS_
    IN ULONG Mask,
    IN PCSTR Text
    )
{
    UNREFERENCED_PARAMETER(Mask);
    HRESULT hRes;
    errno_t err;
    size_t cbBinary;
    LPTSTR pszString;

    cbBinary = strlen(Text);
    
    if (g_OutputCbLocal)
    {
        err = strcpy_s(g_CommandBuffer+4, MAX_CMD-4, Text);
        if (err) 
            *((HRESULT *)g_CommandBuffer) = E_FAIL;
        else
            *((HRESULT *)g_CommandBuffer) = S_OK;
    }
    else
    {
        hRes = ToBase64((const byte *)Text, (unsigned int)cbBinary, &pszString);
        if (SUCCEEDED(hRes)) {
            TunnelSend("[sync] {\"type\":\"cmd\",\"msg\":\"%s\", \"base\":%llu,\"offset\":%llu}\n", pszString, g_Base, g_Offset);
            free(pszString);
        }
    }

    return S_OK;
}
