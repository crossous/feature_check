#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

#include "magic_enum.hpp"

int main() {
    // 初始化 GLFW 库
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 告诉 GLFW 不要创建 OpenGL 上下文
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // 创建一个隐藏的窗口以用于 Vulkan 初始化
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan Window", nullptr, nullptr);

    // 创建 Vulkan 实例
    VkInstance instance;
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Feature Check";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan instance" << std::endl;
        return -1;
    }

    // 查询支持的实例级扩展
    uint32_t instanceExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> instanceExtensions(instanceExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensions.data());

    std::cout << "Supported Instance Extensions: " << std::endl;
    for (const auto& ext : instanceExtensions) {
        std::cout << "\t" << ext.extensionName << std::endl;
    }

    // 获取可用的物理设备列表
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        std::cerr << "Failed to find GPUs with Vulkan support" << std::endl;
        return -1;
    }

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

    // 选择第一个物理设备进行查询
    VkPhysicalDevice physicalDevice = physicalDevices[0];

    // 查询物理设备的特性
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    std::cout << "Geometry Shader support: " << deviceFeatures.geometryShader << std::endl;
    std::cout << "Tessellation Shader support: " << deviceFeatures.tessellationShader << std::endl;

    // 查询物理设备的属性
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    std::cout << "Device Name: " << deviceProperties.deviceName << std::endl;
    std::cout << "Device Type: " << magic_enum::enum_name(deviceProperties.deviceType) << std::endl;//集显、独显、虚拟GPU等
    std::cout << "API Version: "
        << VK_VERSION_MAJOR(deviceProperties.apiVersion) << "."
        << VK_VERSION_MINOR(deviceProperties.apiVersion) << "."
        << VK_VERSION_PATCH(deviceProperties.apiVersion) << std::endl;

    // 查询物理设备支持的扩展
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());

    std::cout << "Supported Device Extensions: " << std::endl;
    for (const auto& ext : extensions) {
        std::cout << "\t" << ext.extensionName << std::endl;
    }

    // 查询物理设备的内存特性
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
    std::cout << "Memory Heaps: " << memoryProperties.memoryHeapCount << std::endl;
    for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++) {
        std::cout << "Heap " << i << " size: " << memoryProperties.memoryHeaps[i].size / (1024 * 1024) << " MB" << std::endl;
    }

    // 查询物理设备的队列族属性
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    std::cout << "Queue Families: " << std::endl;
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        std::cout << "Queue Family " << i << ": " << std::endl;
        std::cout << "\tQueue Count: " << queueFamilies[i].queueCount << std::endl;
        std::cout << "\tQueue Flags: ";

        for (VkQueueFlagBits bit = VK_QUEUE_GRAPHICS_BIT; bit <= VK_QUEUE_FLAG_BITS_MAX_ENUM && bit >= VK_QUEUE_GRAPHICS_BIT; bit = static_cast<VkQueueFlagBits>(bit << 1))
        {
            if (!magic_enum::enum_contains<VkQueueFlagBits>(bit))
                continue;

            if (queueFamilies[i].queueFlags & bit)
                std::cout << magic_enum::enum_name(bit) << " ";
        }

        std::cout << std::endl;
    }

    // 清理资源
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}