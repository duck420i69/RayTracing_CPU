#include "VulkanDevice.h"

std::optional<uint32_t> getQueueIndex(vk::PhysicalDevice physicalDevice, VkQueueType type, int startIndex) {
    auto queueProperties = physicalDevice.getQueueFamilyProperties();

    for (int i = startIndex; i < queueProperties.size(); i++) {
        vk::QueueFlags flag = queueProperties[i].queueFlags;
        
        switch (type) {
        case VkQueueType::ALL_PORPOSE: {
            if ((flag & vk::QueueFlagBits::eGraphics) && 
                (flag & vk::QueueFlagBits::eCompute) && 
                (flag & vk::QueueFlagBits::eTransfer)) {
                return i;
            }
            break;
        }
        case VkQueueType::GRAPHIC: {
            if (flag & vk::QueueFlagBits::eGraphics)
                return i;
            break;
        }
        case VkQueueType::COMPUTE: {
            if (flag & vk::QueueFlagBits::eCompute)
                return i;
            break;
        }
        case VkQueueType::TRANSFER: {
            if (flag & vk::QueueFlagBits::eTransfer)
                return i;
            break;
        }
        case VkQueueType::TRASFER_ONLY: {
            if (!(flag & vk::QueueFlagBits::eGraphics) &&
                !(flag & vk::QueueFlagBits::eCompute) &&
                (flag & vk::QueueFlagBits::eTransfer))
                return i;
            break;
        }
        case VkQueueType::CONPUTE_ONLY: {
            if (!(flag & vk::QueueFlagBits::eGraphics) && 
                (flag & vk::QueueFlagBits::eCompute) && 
                !(flag & vk::QueueFlagBits::eTransfer))
                return i;
            break;
        }
        default:
            break;
        }
    }

    return {};
}


std::optional<uint32_t> getPresentQueue(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
    std::optional<uint32_t> index;

    while (true) {
        index = getQueueIndex(physicalDevice, VkQueueType::ALL_PORPOSE, index.value_or(0));
        if (!index.has_value()) return {};

        if (physicalDevice.getSurfaceSupportKHR(index.value(), surface)) {
            return index;
        }
    }

    while (true) {
        index = getQueueIndex(physicalDevice, VkQueueType::GRAPHIC, index.value_or(0));
        if (!index.has_value()) return {};

        if (physicalDevice.getSurfaceSupportKHR(index.value(), surface)) {
            return index;
        }
    }

    auto queueProperties = physicalDevice.getQueueFamilyProperties();
    for (int i = 0; i < queueProperties.size(); i++) {
        if (physicalDevice.getSurfaceSupportKHR(index.value(), surface)) {
            return i;
        }
    }
}


DeviceBuilder::DeviceBuilder(VkCustomInstance& instance) : instance(instance), physicalDevice(VK_NULL_HANDLE) {
    auto physicalDevices = instance.get().enumeratePhysicalDevices();

    listDevices = physicalDevices;
    suitableDevices = listDevices;

    if (listDevices.empty()) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
}

DeviceBuilder& DeviceBuilder::addQueue(VkQueueType type, uint32_t queueCount, float queuePriority) {
    std::vector<vk::PhysicalDevice> usable_devices;
    std::optional<uint32_t> queueIndex;
    
    for (auto& phyDevice : suitableDevices) {
        queueIndex = getQueueIndex(phyDevice, type);
        if (queueIndex.has_value())
            usable_devices.push_back(phyDevice);
    }
    
    suitableDevices = std::move(usable_devices);
    queueTypeRequired.push_back(type);

    vk::DeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.queueFamilyIndex = 0;
    queueCreateInfo.queueCount = queueCount;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    queueCreateInfos.push_back(queueCreateInfo);

    return *this;
}

DeviceBuilder& DeviceBuilder::addExtention(const char* extension) {
    deviceExtensions.push_back(extension);
    return *this;
}

DeviceBuilder& DeviceBuilder::addExtentions(std::vector<const char*> extensions) {
    for (auto extension : extensions)
        deviceExtensions.push_back(extension);
    return *this;
}

DeviceBuilder& DeviceBuilder::choosePhysicalDevice() {
    for (const auto& device : suitableDevices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }
    if (!physicalDevice) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
    return *this;
}

DeviceBuilder& DeviceBuilder::requestSwapchainSupport(VkSurfaceKHR surface) {
    std::vector<vk::PhysicalDevice> usable_devices;

    for (auto& phyDevice : suitableDevices) {
        auto capabilities = phyDevice.getSurfaceCapabilitiesKHR(surface);
        auto formats = phyDevice.getSurfaceFormatsKHR(surface);
        auto presentModes = phyDevice.getSurfacePresentModesKHR(surface);

        auto surfaceSupport = getPresentQueue(phyDevice, surface).has_value();

        if (!formats.empty() && !presentModes.empty() && surfaceSupport) {
            usable_devices.push_back(phyDevice);
        }
    }

    suitableDevices = std::move(usable_devices);

    presentSupport = true;
    this->surface = surface;

    if (!suitableDevices.empty()) {
        deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }
    return *this;
}

Result<VkCustomDevice> DeviceBuilder::build() {
    choosePhysicalDevice();

    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    std::optional<uint32_t> presentIndex;

    if (presentSupport) {
        presentIndex = getPresentQueue(physicalDevice, surface);
    }

    std::set<uint32_t> queueSet;
    std::vector<std::pair<VkQueueType, uint32_t>> queueTypeIndex;

    for (int i = 0; i < queueCreateInfos.size(); i++) {
        queueCreateInfos[i].queueFamilyIndex = getQueueIndex(physicalDevice, queueTypeRequired[i]).value();
        if (queueSet.find(queueCreateInfos[i].queueFamilyIndex) == queueSet.end()) {
            queueTypeIndex.push_back({ queueTypeRequired[i], queueCreateInfos[i].queueFamilyIndex });
            queueSet.insert(queueCreateInfos[i].queueFamilyIndex);
        }
        else {

        }
    }

    vk::DeviceCreateInfo createInfo{};

    createInfo.setQueueCreateInfos(queueCreateInfos);
    createInfo.setPEnabledExtensionNames(deviceExtensions);

    createInfo.pEnabledFeatures = &deviceFeatures;

    if (enableValidationLayers) {
        createInfo.setPEnabledLayerNames(validationLayers);
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    auto device = physicalDevice.createDevice(createInfo);

    return std::move(VkCustomDevice(physicalDevice, device, queueTypeIndex, presentIndex));
}

bool DeviceBuilder::checkDeviceExtensionSupport(vk::PhysicalDevice device) {
    auto availableExtensions = device.enumerateDeviceExtensionProperties();

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool DeviceBuilder::isDeviceSuitable(vk::PhysicalDevice device) {
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = true;

    VkPhysicalDeviceFeatures supportedFeatures = device.getFeatures();

    return extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

VkCustomDevice::VkCustomDevice(VkCustomDevice&& device) noexcept {
    this->device = device.device;
    this->physicalDevice = device.physicalDevice;
    this->queueIndexes = device.queueIndexes;
    this->presentIndex = device.presentIndex;
    device.device = VK_NULL_HANDLE;
}

VkCustomDevice& VkCustomDevice::operator=(VkCustomDevice&& device) noexcept {
    this->device = device.device;
    this->physicalDevice = device.physicalDevice;
    this->queueIndexes = device.queueIndexes;
    this->presentIndex = device.presentIndex;
    device.device = VK_NULL_HANDLE;
    return *this;
}

std::optional<vk::ShaderModule> VkCustomDevice::loadShader(const char* filePath) {
    //open the file. With cursor at the end
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        return {};
    }

    //find what the size of the file is by looking up the location of the cursor
    //because the cursor is at the end, it gives the size directly in bytes
    size_t fileSize = (size_t)file.tellg();
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
    file.seekg(0);
    file.read((char*)buffer.data(), fileSize);
    file.close();

    vk::ShaderModuleCreateInfo createInfo = {};
    createInfo.codeSize = buffer.size() * sizeof(uint32_t);
    createInfo.pCode = buffer.data();

    auto shader = device.createShaderModule(createInfo);

    return shader;
}
