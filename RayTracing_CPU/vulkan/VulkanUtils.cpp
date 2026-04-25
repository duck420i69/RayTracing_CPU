#include "vulkan.h"

/*
#ifdef NDEBUG
const static bool enableValidationLayers = false;
#else
const static bool enableValidationLayers = true;
#endif
*/

namespace vkUtils {
	template<class T> 
    struct Result {
		T value;
		VkResult err;

        T getValue() { 
            if (err != VK_SUCCESS)
                std::cerr << "Builder error:\ncode " << err;
            return value; 
        }
	};

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };


    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            std::cerr << "*** ERROR *** - validation layer: " << pCallbackData->pMessage << std::endl;
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            std::cerr << "** WARNING ** - validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

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
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    class InstanceBuilder {
    public:
        void setAppName(char* name) {
            appInfo.pApplicationName = name;
        }
        void setEngineName(char* name) {
            appInfo.pEngineName = name;
        }
        void setAppVersion(int major, int minor, int patch) {
            appInfo.applicationVersion = VK_MAKE_API_VERSION(0, major, minor, patch);
        }
        void setAPIVersion(int major, int minor, int patch) {
            appInfo.apiVersion = VK_MAKE_API_VERSION(0, major, minor, patch);
        }
        void setEngineVersion(int major, int minor, int patch) {
            appInfo.engineVersion = VK_MAKE_API_VERSION(0, major, minor, patch);
        }
        void addExtention(const char* extension) {
            required_extensions.push_back(extension);
        }
        void addExtentions(std::vector<const char*> extensions) {
            for (auto extension : extensions)
                required_extensions.push_back(extension);
        }
        Result<VkInstance> build() {
            if (enableValidationLayers && !checkValidationLayerSupport()) {
                throw std::runtime_error("validation layers requested, but not available!");
            }

            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Hello Triangle";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            VkInstanceCreateInfo createInfo{};
            insInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            insInfo.pApplicationInfo = &appInfo;

            insInfo.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
            insInfo.ppEnabledExtensionNames = required_extensions.data();

            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            if (enableValidationLayers) {
                insInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                insInfo.ppEnabledLayerNames = validationLayers.data();

                populateDebugMessengerCreateInfo(debugCreateInfo);
                insInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
            }
            else {
                insInfo.enabledLayerCount = 0;

                insInfo.pNext = nullptr;
            }

            Result<VkInstance> result;
            result.err = vkCreateInstance(&createInfo, nullptr, &result.value);
            return result;
        }

    private:
        std::vector<const char*> validation_layers;
        std::vector<const char*> required_extensions;
        VkApplicationInfo appInfo;
        VkInstanceCreateInfo insInfo;

        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
            createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = debugCallback;
        }
    };

    class DeviceBuilder {
    public:
        DeviceBuilder(VkInstance instance) : instance(instance) {
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

            if (deviceCount == 0) {
                throw std::runtime_error("failed to find GPUs with Vulkan support!");
            }

            listDevices.resize(deviceCount);
            vkEnumeratePhysicalDevices(instance, &deviceCount, listDevices.data());
        }

        void addQueue(uint32_t queueFamily, uint32_t queueCount = 1, float queuePriority = 1.0f) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = queueCount;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        void addExtention(const char* extension) {
            deviceExtensions.push_back(extension);
        }

        void addExtentions(std::vector<const char*> extensions) {
            for (auto extension : extensions) 
                deviceExtensions.push_back(extension);
        }

        void choosePhycalDevice() {
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

            if (deviceCount == 0) {
                throw std::runtime_error("failed to find GPUs with Vulkan support!");
            }

            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

            for (const auto& device : devices) {
                if (isDeviceSuitable(device)) {
                    physicalDevice = device;
                    break;
                }
            }

            if (physicalDevice == VK_NULL_HANDLE) {
                throw std::runtime_error("failed to find a suitable GPU!");
            }
        }

        bool requestSwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
            SwapChainSupportDetails swapChainSupport;

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainSupport.capabilities);

            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

            if (formatCount != 0) {
                swapChainSupport.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapChainSupport.formats.data());
            }

            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

            if (presentModeCount != 0) {
                swapChainSupport.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, swapChainSupport.presentModes.data());
            }

            return !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        Result<VkDevice> build() {
            VkPhysicalDeviceFeatures deviceFeatures{};
            deviceFeatures.samplerAnisotropy = VK_TRUE;

            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pQueueCreateInfos = queueCreateInfos.data();

            createInfo.pEnabledFeatures = &deviceFeatures;

            createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
            createInfo.ppEnabledExtensionNames = deviceExtensions.data();

            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            }
            else {
                createInfo.enabledLayerCount = 0;
            }

            Result<VkDevice> device;
            device.err = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device.value);
            return device;
        }
    private:
        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        std::vector<VkPhysicalDevice> listDevices;
        std::vector<const char*> deviceExtensions;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

            std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

            for (const auto& extension : availableExtensions) {
                requiredExtensions.erase(extension.extensionName);
            }

            return requiredExtensions.empty();
        }

        bool isDeviceSuitable(VkPhysicalDevice device) {
            /*
            QueueFamilyIndices indices = findQueueFamilies(device);

            bool extensionsSupported = checkDeviceExtensionSupport(device);

            bool swapChainAdequate = false;

            if (extensionsSupported) {

            }

            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

            return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
            */
            return true;
        }

        class ImageBuilder {
        public:
            static void setDefault() {};

            ImageBuilder() {
                VkImageCreateInfo imageInfo{};
                imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageInfo.imageType = VK_IMAGE_TYPE_2D;
                imageInfo.extent.width = width;
                imageInfo.extent.height = height;
                imageInfo.extent.depth = 1;
                imageInfo.mipLevels = 1;
                imageInfo.arrayLayers = 1;
                imageInfo.format = format;
                imageInfo.tiling = tiling;
                imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageInfo.usage = usage;
                imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            }

            void build(Image image) {
                if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create image!");
                }

                VkMemoryRequirements memRequirements;
                vkGetImageMemoryRequirements(device, image, &memRequirements);

                VkMemoryAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memRequirements.size;
                allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

                if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
                    throw std::runtime_error("failed to allocate image memory!");
                }

                vkBindImageMemory(device, image, imageMemory, 0);
            };

        private:
            static VkImageCreateInfo default_create_info;
            VkImageCreateInfo createInfo;
        };
    };

    void cmdBlitImage(VkCommandBuffer commandBuffer) {
        vkCmdBlitImage(commandBuffer, )
    }
}