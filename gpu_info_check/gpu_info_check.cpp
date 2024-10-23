﻿#include <iostream>
#include <comdef.h>
#include <wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

void GetVideoControllerInfo() {
    HRESULT hres;

    // 初始化 COM 库
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        std::cerr << "Failed to initialize COM library." << std::endl;
        return;
    }

    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
    );

    // 创建 WMI 对象
    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL, NULL, 0,
        NULL, 0, 0, &pSvc);

    // 设置安全级别
    hres = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHN_WINNT,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE);

    // 查询 Win32_VideoController
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_VideoController"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    // 遍历结果
    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn) {
            break;
        }

        VARIANT vtProp;

        // 获取驱动版本
        hr = pclsObj->Get(L"DriverVersion", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr)) {
            std::wcout << L"Driver Version: " << vtProp.bstrVal << std::endl;
            VariantClear(&vtProp);
        }

        // 获取 GPU 名称
        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr)) {
            std::wcout << L"GPU Name: " << vtProp.bstrVal << std::endl;
            VariantClear(&vtProp);
        }

        pclsObj->Release();
    }

    // 清理
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
}

int main() {
    GetVideoControllerInfo();

    system("pause");
    return 0;
}