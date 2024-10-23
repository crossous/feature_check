#include <iostream>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>

#include "magic_enum.hpp"

#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

void CheckDeviceSupportFeatures(ID3D12Device* device) {
    // 查询 D3D12_OPTIONS 支持情况
    D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
    if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)))) {
        std::cout << "Tiled Resources Tier: " << options.TiledResourcesTier << std::endl;
        std::cout << "Resource Binding Tier: " << options.ResourceBindingTier << std::endl;
        std::cout << "Conservative Rasterization Tier: " << options.ConservativeRasterizationTier << std::endl;
    }

    // 查询 D3D12_OPTIONS5 支持情况（光线追踪）
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
    if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)))) {
        std::cout << "Raytracing Tier: " << options5.RaytracingTier << std::endl;
    }

    // 查询设备格式支持情况
    D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = {};
    formatSupport.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport)))) {
        for (D3D12_FORMAT_SUPPORT1 support1Mask = D3D12_FORMAT_SUPPORT1_BUFFER; support1Mask <= D3D12_FORMAT_SUPPORT1_VIDEO_ENCODER && support1Mask > D3D12_FORMAT_SUPPORT1_NONE; support1Mask = static_cast<D3D12_FORMAT_SUPPORT1>(support1Mask << 1))
        {
            if (!magic_enum::enum_contains<D3D12_FORMAT_SUPPORT1>(support1Mask))
                continue;

            std::cout << "Format " << magic_enum::enum_name(formatSupport.Format) << " support " << magic_enum::enum_name(support1Mask) << ": " << static_cast<bool>(formatSupport.Support1 & support1Mask) << std::endl;
        }

        for (D3D12_FORMAT_SUPPORT2 support2Mask = D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_ADD; support2Mask <= D3D12_FORMAT_SUPPORT2_SAMPLER_FEEDBACK && support2Mask > D3D12_FORMAT_SUPPORT2_NONE; support2Mask = static_cast<D3D12_FORMAT_SUPPORT2>(support2Mask << 1))
        {
            if (!magic_enum::enum_contains<D3D12_FORMAT_SUPPORT2>(support2Mask))
                continue;

            std::cout << "Format " << magic_enum::enum_name(formatSupport.Format) << " support " << magic_enum::enum_name(support2Mask) << ": " << static_cast<bool>(formatSupport.Support2 & support2Mask) << std::endl;
        }
    }

    // 查询设备架构信息
    D3D12_FEATURE_DATA_ARCHITECTURE architecture = {};
    if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &architecture, sizeof(architecture)))) {
        std::cout << "UMA: " << (architecture.UMA ? "Yes" : "No") << std::endl;
    }
}

void CheckHardwareSupport(ID3D12Device* device) {
    // 查询硬件支持的描述符绑定信息
    D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT gpuVASupport = {};
    if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, &gpuVASupport, sizeof(gpuVASupport)))) {
        std::cout << "Maximum resource page size: " << gpuVASupport.MaxGPUVirtualAddressBitsPerResource << std::endl;
    }
    else {
        std::cout << "Failed to retrieve GPU Virtual Address support data." << std::endl;
    }

    // 查询Wave支持的信息
    D3D12_FEATURE_DATA_D3D12_OPTIONS1 options1 = {};
    if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &options1, sizeof(options1)))) {
        std::cout << "Wave Lane Count Min: " << options1.WaveLaneCountMin << std::endl;
        std::cout << "Wave Lane Count Max: " << options1.WaveLaneCountMax << std::endl;
    }
    else {
        std::cout << "Failed to retrieve D3D12_OPTIONS1 feature data." << std::endl;
    }
}

using namespace Microsoft::WRL;

int main() {
    // 创建 DXGI 工厂
    ComPtr<IDXGIFactory4> dxgiFactory;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(hr)) {
        std::cerr << "Failed to create DXGI Factory." << std::endl;
        return -1;
    }

    // 获取适配器（GPU）
    ComPtr<IDXGIAdapter1> hardwareAdapter;
    for (UINT adapterIndex = 0; dxgiFactory->EnumAdapters1(adapterIndex, &hardwareAdapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex) {
        DXGI_ADAPTER_DESC1 desc;
        hardwareAdapter->GetDesc1(&desc);

        // 检查是否为软件设备
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            continue; // 跳过软件设备
        }

        std::wcout << L"Using Adapter: " << desc.Description << std::endl;
        break;
    }

    // 创建 D3D12 设备
    ComPtr<ID3D12Device> device;
    hr = D3D12CreateDevice(
        hardwareAdapter.Get(),
        D3D_FEATURE_LEVEL_12_0, // 请求 Direct3D 12 的功能级别
        IID_PPV_ARGS(&device)
    );

    if (FAILED(hr)) {
        std::cerr << "Failed to create D3D12 Device." << std::endl;
        return -1;
    }


    CheckDeviceSupportFeatures(device.Get());
    CheckHardwareSupport(device.Get());

    system("pause");
    return 0;
}