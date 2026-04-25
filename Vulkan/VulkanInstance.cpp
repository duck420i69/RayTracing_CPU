#include "VulkanInstance.h"


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        std::cerr << "*** ERROR *** - validation layer: " << pCallbackData->pMessage << std::endl;
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        std::cerr << "** WARNING ** - validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

bool checkValidationLayerSupport() {
    auto availableLayers = vk::enumerateInstanceLayerProperties();

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

std::vector<const char*> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

vk::DebugUtilsMessengerCreateInfoEXT populateDebugMessengerCreateInfo() {
    vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.messageSeverity = 
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | 
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | 
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
    createInfo.messageType = 
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | 
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | 
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
    createInfo.pfnUserCallback = debugCallback;
    return createInfo;
}


InstanceBuilder::InstanceBuilder() {
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_API_VERSION_1_0;
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_API_VERSION_1_0;
    appInfo.apiVersion = VK_API_VERSION_1_3;

    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    if (enableValidationLayers) {
        insInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        insInfo.ppEnabledLayerNames = validationLayers.data();

        debugCreateInfo = populateDebugMessengerCreateInfo();
        insInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        insInfo.enabledLayerCount = 0;
        insInfo.pNext = nullptr;
    }
}

InstanceBuilder& InstanceBuilder::setAppName(char* name) {
    appInfo.pApplicationName = name;
    return *this;
}
InstanceBuilder& InstanceBuilder::setEngineName(char* name) {
    appInfo.pEngineName = name;
    return *this;
}
InstanceBuilder& InstanceBuilder::setAppVersion(int major, int minor, int patch) {
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, major, minor, patch);
    return *this;
}
InstanceBuilder& InstanceBuilder::setAPIVersion(int major, int minor, int patch) {
    appInfo.apiVersion = VK_MAKE_API_VERSION(0, major, minor, patch);
    return *this;
}
InstanceBuilder& InstanceBuilder::setEngineVersion(int major, int minor, int patch) {
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, major, minor, patch);
    return *this;
}
InstanceBuilder& InstanceBuilder::addExtention(const char* extension) {
    required_extensions.push_back(extension);
    return *this;
}
InstanceBuilder& InstanceBuilder::addExtentions(std::vector<const char*> extensions) {
    for (auto extension : extensions)
        required_extensions.push_back(extension);
    return *this;
}

Result<VkCustomInstance> InstanceBuilder::build() {
    insInfo.pApplicationInfo = &appInfo;
    insInfo.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    insInfo.ppEnabledExtensionNames = required_extensions.data();

    try {
        auto instance = vk::createInstance(insInfo);

        return std::move(VkCustomInstance(instance));
    }
    catch (vk::SystemError e) {
        return e;
    }
}

VkCustomInstance::VkCustomInstance(vk::Instance instance) : instance(instance) {
    surface = nullptr;
    debugMessenger = nullptr;
    
    if (instance && enableValidationLayers) {
        vk::DispatchLoaderDynamic dl(instance, vkGetInstanceProcAddr);

        VkDebugUtilsMessengerCreateInfoEXT createInfo = populateDebugMessengerCreateInfo();
        debugMessenger = instance.createDebugUtilsMessengerEXT(createInfo, nullptr, dl);
    }
}

VkCustomInstance::VkCustomInstance(VkCustomInstance&& instance) noexcept {
    this->instance = instance.instance;
    this->surface = instance.surface;
    this->debugMessenger = instance.debugMessenger;
    instance.instance = nullptr;
    instance.surface = nullptr;
    instance.debugMessenger = nullptr;
}

VkCustomInstance& VkCustomInstance::operator=(VkCustomInstance&& other) noexcept {
    this->instance = other.instance;
    this->surface = other.surface;
    this->debugMessenger = other.debugMessenger;
    other.instance = nullptr;
    other.surface = nullptr;
    other.debugMessenger = nullptr;
    return *this;
}

VkCustomInstance::~VkCustomInstance() {
    if (instance) {
        if (enableValidationLayers) {
            vk::DispatchLoaderDynamic dl(instance, vkGetInstanceProcAddr);
            instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dl);
        }
        if (surface) {
            instance.destroySurfaceKHR(surface);
        }
        instance.destroy();
    }
}

vk::Instance VkCustomInstance::get() {
    return instance;
}

Result<vk::SurfaceKHR> VkCustomInstance::createSurface(GLFWwindow* window) {
    VkSurfaceKHR surface;
    VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    
    if (result == VK_SUCCESS)
        return vk::SurfaceKHR(surface);

    vk::throwResultException(vk::Result(result), "");
}