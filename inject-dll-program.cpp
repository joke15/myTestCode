#include "windows.h"
#include "tlhelp32.h"
#include "tchar.h"
#include "iostream"

bool m_init = false;
DWORD m_pid;
LPCTSTR m_dllPath;

bool checkDllInProcess()
{
    if (!m_init)
    {
        std::cout << "MIPDirectXInjectManager::checkDllInProcess: directX inject manager not inited." << std::endl;
        return false;
    }

    HANDLE snapShot = INVALID_HANDLE_VALUE;
    MODULEENTRY32 moduleEntry = {sizeof(moduleEntry)};
    BOOL have_module = FALSE;

    /*获取进程快照*/
    if (INVALID_HANDLE_VALUE == (snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_pid)))
    {
        std::cout << "MIPDirectXInjectManager::checkDllInProcess: directX inject manager get process module snapshot failed." << std::endl;
        return false;
    }

    /*遍历进程模块*/
    for (have_module = Module32First(snapShot, &moduleEntry); have_module; have_module = Module32Next(snapShot, &moduleEntry))
    {
        std::cout << moduleEntry.szExePath << std::endl;
        if (!_tcsicmp(moduleEntry.szModule, m_dllPath) || !_tcsicmp(moduleEntry.szExePath, m_dllPath))
        {
            return false;
        }
    }

    CloseHandle(snapShot);
    return true;
};

bool injectDllToProcess()
{
    HANDLE process = nullptr;
    LPVOID remoteBuffer = nullptr;
    DWORD bufferSize = (DWORD)(_tcslen(m_dllPath) + 1) * sizeof(TCHAR);
    HMODULE module = nullptr;
    LPTHREAD_START_ROUTINE threadProc = nullptr;
    bool ret = false;

    if (!(process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_pid)))
    {
        std::cout << "MIPDirectXInjectManager::injectDllToProcess: inject manager open dest process failed" << std::endl;
        goto release_resource;
    }

    remoteBuffer = VirtualAllocEx(process, NULL, bufferSize, MEM_COMMIT, PAGE_READWRITE);

    if (remoteBuffer == nullptr)
    {
        std::cout << "MIPDirectXInjectManager::injectDllToProcess: inject manager virtual alloc faield" << std::endl;
        goto release_resource;
    }

    if (!WriteProcessMemory(process, remoteBuffer, (LPVOID)m_dllPath, bufferSize, NULL))
    {
        std::cout << "MIPDirectXInjectManager::injectDllToProcess: inject manager write process memory failed" << std::endl;
        goto release_resource;
    }

    module = GetModuleHandleW(L"KERNEL32");
    if (module == nullptr)
    {
        std::cout << "MIPDirectXInjectManager::injectDllToProcess: inject manager get kerenl32.dll fialed" << std::endl;
        goto release_resource;
    }

    threadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(module, "LoadLibraryA");
    if (threadProc == nullptr)
    {
        std::cout << "MIPDirectXInjectManager::injectDllToProcess: inject manager get proc address of LoadLibraryW failed" << std::endl;
        goto release_resource;
    }

    if (!CreateRemoteThread(process, NULL, 0, threadProc, remoteBuffer, 0, NULL))
    {
        std::cout << "MIPDirectXInjectManager::injectDllToProcess: inject manager create remote thread failed" << std::endl;
        goto release_resource;
    }

    ret = true;

release_resource:
    // if (remoteBuffer)
    // {
    //     VirtualFreeEx(process, remoteBuffer, 0, MEM_RELEASE);
    // }
    if (process)
    {
        CloseHandle(process);
    }
    return ret;
};

bool freeDllInProcess()
{
    HANDLE process = nullptr;
    HMODULE module = nullptr;
    LPTHREAD_START_ROUTINE threadProc = nullptr;
    HANDLE snapShot = INVALID_HANDLE_VALUE;
    MODULEENTRY32 moduleEntry = {sizeof(moduleEntry)};
    BOOL have_module = FALSE;
    bool ret = false;

    /*获取进程快照*/
    if (INVALID_HANDLE_VALUE == (snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_pid)))
    {
        std::cout << "MIPDirectXInjectManager::checkDllInProcess: directX inject manager get process module snapshot failed." << std::endl;
        goto release_resource;
    }

    /*遍历进程模块*/
    for (have_module = Module32First(snapShot, &moduleEntry); have_module; have_module = Module32Next(snapShot, &moduleEntry))
    {
        if (!_tcsicmp(moduleEntry.szModule, m_dllPath) || !_tcsicmp(moduleEntry.szExePath, m_dllPath))
        {
            goto free_start;
        }
    }
    goto release_resource;

free_start:
    if (!(process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_pid)))
    {
        std::cout << "MIPDirectXInjectManager::freeDllInProcess: inject manager open process failed" << std::endl;
        goto release_resource;
    }

    module = GetModuleHandleW(L"KERNEL32");
    if (module == nullptr)
    {
        std::cout << "MIPDirectXInjectManager::freeDllInProcess： inject manager get module handle kernel32.dll failed" << std::endl;
        goto release_resource;
    }

    threadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(module, "FreeLibrary");
    if (threadProc == nullptr)
    {
        std::cout << "MIPDirectXInjectManager::freeDllInProcess: inject get proc address FreeLibrary failed" << std::endl;
        goto release_resource;
    }

    if (!(CreateRemoteThread(process, NULL, 0, threadProc, moduleEntry.modBaseAddr, 0, NULL)))
    {
        std::cout << "MIPDirectXInjectManager::freeDllInProcess: inject create remote thread failed" << std::endl;
        goto release_resource;
    }

    ret = true;
release_resource:
    if (snapShot != INVALID_HANDLE_VALUE)
        CloseHandle(snapShot);
    if (process)
        CloseHandle(process);
    return ret;
};

bool init(HWND hwnd, LPCTSTR dllPath)
{
    if (!m_init)
    {
        std::cout << "MIPDirectXInjectManager:: init start .." << std::endl;
        GetWindowThreadProcessId((HWND)hwnd, &m_pid);
        std::cout << "m_pid: " << m_pid << std::endl;
        m_dllPath = dllPath;
        m_init = true;
        return true;
    }
    else
    {
        std::cout << "MIPDirectXInjectManager:: directx inject manager has been inited.." << std::endl;
        return false;
    }
}

bool startInject()
{
    if (!checkDllInProcess())
    {
        std::cout << "MIPDirectXInjectManager:: check dll in process failed once. retry .." << std::endl;
        if (!checkDllInProcess())
        {
            std::cout << "MIPDirectXInjectManager:: check dll in process failed again, directx inject manager start failed" << std::endl;
            return false;
        }
    }

    std::cout << "MIPDirectXInjectManager:: check dll not in process, start inject dll to process .." << std::endl;
    return injectDllToProcess();
}

bool startFree()
{
    if (checkDllInProcess())
    {
        std::cout << "MIPDirectXInjectManager:: check dll not in process failed once. retry .." << std::endl;
        if (checkDllInProcess())
        {
            std::cout << "MIPDirectXInjectManager:: check dll not in process failed again, directx inject manager start failed" << std::endl;
            return false;
        }
    }

    std::cout << "MIPDirectXInjectManager:: check dll in process, start free dll from process .." << std::endl;
    return freeDllInProcess();
}

void end()
{
    std::cout << "please press any key to exit..." << std::endl;
    getchar();
    getchar();
    return;
}

int main()
{
    std::cout << "chose to free dll or inject dll" << std::endl
              << "0: inject dll" << std::endl
              << "1: free dll" << std::endl;
    int t;
    std::string dx11hooktest_dllpath = "D:\\Program Files\\project\\visual studio repo\\dx11hooktest\\x64\\Release\\dx11hooktest.dll";
    std::string dll_path = dx11hooktest_dllpath;
    while (std::cin >> t)
        if (t == 0)
        {
            std::cout << "enter hwnd:" << std::endl;
            long hwnd;
            std::cin >> std::hex >> hwnd;
            if (!init((HWND)hwnd, (LPCTSTR)dll_path.c_str()))
            {
                end();
                return 0;
            }

            if (startInject())
                std::cout << "inject success" << std::endl;
            else
                std::cout << "inject failed" << std::endl;

            std::cout << "need free dll?" << std::endl
                      << "0: free dll" << std::endl
                      << "1: don't free dll" << std::endl;
            int i;
            std::cin >> i;
            if (i == 0)
            {
                if (startFree())
                    std::cout << "free success" << std::endl;
                else
                    std::cout << "free failed" << std::endl;
            }
            end();
            return 0;
        }
        else if (t == 1)
        {
            std::cout << "enter hwnd:" << std::endl;
            long hwnd;
            std::cin >> std::hex >> hwnd;
            if (!init((HWND)hwnd, (LPCTSTR)dll_path.c_str()))
            {
                end();
                return 0;
            }
            if (startFree())
                std::cout << "free success" << std::endl;
            else
                std::cout << "free failed" << std::endl;
            end();
            return 0;
        }
        else
        {
            std::cout << "chose to free dll or inject dll" << std::endl
                      << "0: inject dll" << std::endl
                      << "1: free dll" << std::endl;
        }
}