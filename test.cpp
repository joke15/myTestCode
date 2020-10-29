#include "stdafx.h"
 
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <Psapi.h>
#include <detours.h>
//#include <d3d9.h>
//#include <d3dx9.h>
#include <D3D11.h>
#include <D3DX11.h>
#include <fstream>
using namespace std;
 
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "detours.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
 
int (__stdcall* origClearRenderTargetView)(ID3D11DeviceContext* DeviceContext, ID3D11RenderTargetView* TargetView, const FLOAT ColorRGBA[4]);
int (__stdcall* origPresent)(IDXGISwapChain* SwapChain, UINT _ui1, UINT _ui2);
int (__stdcall* origDraw)(UINT VertexCount,UINT StartVertexLocation);
int (__stdcall* origDrawIndexed)(ID3D11Device* g_Device,UINT IndexCount,UINT StartIndexLocation,INT BaseVertexLocation);
 
ID3D11DeviceContext* g_DeviceContext;
ID3D11Device*        g_Device;
ID3D11RenderTargetView* g_RenderTargetView;
ID3D11Texture2D* g_BackBuffer;
 
ofstream myfile; //Used for logging to a text file
char sBuffer[32];
 
int WINAPI myDraw(UINT VertexCount,UINT StartVertexLocation)
{
    return origDraw(VertexCount,StartVertexLocation);
}
 
int WINAPI myDrawIndexed(ID3D11Device* g_Device,UINT IndexCount,UINT StartIndexLocation,INT BaseVertexLocation)
{
    if(myfile)
    {
        sprintf(sBuffer,"%d\n",IndexCount);
        myfile << sBuffer; 
    }
    return origDrawIndexed(g_Device,IndexCount,StartIndexLocation,BaseVertexLocation);
}
 
int WINAPI myClearRenderTargetView(ID3D11DeviceContext* DeviceContext, ID3D11RenderTargetView* TargetView, const FLOAT ColorRGBA[4])
{
    return origClearRenderTargetView(DeviceContext, TargetView, ColorRGBA);
}
 
int WINAPI myPresent(IDXGISwapChain* SwapChain, UINT _ui1, UINT _ui2)
{
    return origPresent(SwapChain, _ui1, _ui2);
}
 
int WINAPI DllThread()
{
    myfile.open("Hooklog.txt");
 
    HMODULE hD3D11 = NULL;
    HMODULE hDXGI  = NULL;
 
    MODULEINFO Info_D3D11 = {0};
    MODULEINFO Info_DXGI = {0};
 
    int Addr_ClearRenderTargetView = 0;
    int Addr_Present = 0;
    int Addr_Draw = 0;
    int Addr_DrawIndexed = 0;
 
    while(hD3D11 == NULL)
    {
        hD3D11 = GetModuleHandle("d3d11.dll");
    }
 
    while(hDXGI == NULL)
    {
        hDXGI = GetModuleHandle("dxgi.dll");
    }
 
    GetModuleInformation(GetCurrentProcess(), hD3D11, &Info_D3D11, sizeof(MODULEINFO));
    GetModuleInformation(GetCurrentProcess(), hDXGI, &Info_DXGI, sizeof(MODULEINFO));
 
    /*
    char _r1[32] = {0};
    char _r2[32] = {0};
    
    itoa((int)Info_D3D11.EntryPoint, _r1, 10);
    itoa((int)Info_DXGI.EntryPoint, _r2, 10);
 
    MessageBoxA(NULL, _r1, NULL, NULL);
    MessageBoxA(NULL, _r2, NULL, NULL);
    */
 
    Addr_ClearRenderTargetView = (int)Info_D3D11.EntryPoint + 0x24334;
    Addr_Present = (int)Info_DXGI.EntryPoint + 0x1C621;
    Addr_Draw = (int)Info_D3D11.EntryPoint + 0x20E84;
    Addr_DrawIndexed = (int)Info_D3D11.EntryPoint + 0x20E64;
 
    origClearRenderTargetView = (int (__stdcall*)(ID3D11DeviceContext* DeviceContext, ID3D11RenderTargetView* TargetView, const FLOAT ColorRGBA[4]))Addr_ClearRenderTargetView;
    origPresent = (int (__stdcall*)(IDXGISwapChain* SwapChain, UINT _ui1, UINT _ui2))Addr_Present;
    origDraw = (int (__stdcall*)(UINT VertexCount,UINT StartVertexLocation))Addr_Draw;
    origDrawIndexed = (int (__stdcall*)(ID3D11Device* g_Device,UINT IndexCount,UINT StartIndexLocation,INT BaseVertexLocation))Addr_DrawIndexed;
 
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
 
    DetourAttach((PVOID*)&origClearRenderTargetView, myClearRenderTargetView);
    DetourAttach((PVOID*)&origPresent, myPresent);
    DetourAttach((PVOID*)&origDraw, myDraw);
    DetourAttach((PVOID*)&origDrawIndexed, myDrawIndexed);
    DetourTransactionCommit();
 
    
 
    return 0;
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE) DllThread, NULL, NULL, NULL);
        break;
 
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}