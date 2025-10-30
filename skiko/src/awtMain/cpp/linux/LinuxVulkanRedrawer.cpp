#include <memory>
#include <vector>
#include <optional>
#include <jni.h>

#include "jni_helpers.h"

#include "third_party/vulkan/vulkan/vulkan.h"

#include "core/SkColorSpace.h"
#include "core/SkImageInfo.h"
#include "core/SkSurface.h"
#include "core/SkSurfaceProps.h"

#include "gpu/graphite/Context.h"
#include "gpu/graphite/ContextOptions.h"
#include "gpu/graphite/Recorder.h"
#include "gpu/graphite/Surface.h"
#include "gpu/graphite/vk/VulkanGraphiteContext.h"
#include "gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "gpu/vk/VulkanBackendContext.h"

namespace {
    void throwRuntimeException(JNIEnv *env, const std::string &message) {
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"), message.c_str());
    }

    struct Context {
        Context(const Context &) = delete;

        static std::unique_ptr<Context> make(JNIEnv *env) {
            auto context = std::unique_ptr<Context>(new Context());
            if (!initVulkan(env, *context) || !initGraphit(env, *context)) {
                return nullptr;
            }

            return context;
        }

        sk_sp<SkSurface> createSurface(int32_t width, int32_t height) {
            if (texture.isValid()) {
                graphiteContext->deleteBackendTexture(texture);
            }

            using namespace skgpu::graphite;
            VulkanTextureInfo vkTexInfo(1, /*sampleCount*/
                                        skgpu::Mipmapped::kNo,
                                        0, /*flags*/
                                        VK_FORMAT_B8G8R8A8_UNORM,
                                        VK_IMAGE_TILING_OPTIMAL,
                                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                        VK_SHARING_MODE_EXCLUSIVE,
                                        VK_IMAGE_ASPECT_COLOR_BIT,
                                        skgpu::VulkanYcbcrConversionInfo{});
            TextureInfo texInfo = TextureInfos::MakeVulkan(vkTexInfo);
            texture = graphiteRecorder->createBackendTexture(SkISize::Make(width, height), texInfo);

            imageInfo = SkImageInfo::Make(
                texture.dimensions().width(),
                texture.dimensions().height(),
                kBGRA_8888_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGB());

            SkSurfaceProps props(0, kRGB_H_SkPixelGeometry);
            surface = SkSurfaces::WrapBackendTexture(
                graphiteRecorder.get(),
                texture,
                kBGRA_8888_SkColorType,
                SkColorSpace::MakeSRGB(),
                &props);
            return surface;
        }

        bool flushAndReadback(std::function<SkImage::ReadPixelsCallback> callback) {
            auto recording = graphiteRecorder->snap();

            skgpu::graphite::InsertRecordingInfo recording_info;
            recording_info.fRecording = recording.get();
            recording_info.fTargetSurface = surface.get();
            recording_info.fTargetTranslation = {0, 0};
            recording_info.fTargetClip = {0, 0, 0, 0};
            recording_info.fTargetTextureState = nullptr;
            recording_info.fNumWaitSemaphores = 0;
            recording_info.fWaitSemaphores = nullptr;
            recording_info.fNumSignalSemaphores = 0;
            recording_info.fSignalSemaphores = nullptr;
            recording_info.fGpuStatsFlags = skgpu::GpuStatsFlags::kNone;
            recording_info.fFinishedContext = nullptr;
            recording_info.fFinishedProc = nullptr;
            recording_info.fFinishedWithStatsProc = nullptr;
            if (!graphiteContext->insertRecording(recording_info)) {
                return false;
            }

            auto context = reinterpret_cast<SkImage::ReadPixelsContext>(&callback);

            graphiteContext->asyncRescaleAndReadPixels(
                surface.get(),
                imageInfo,
                SkIRect::MakeWH(surface->width(), surface->height()),
                SkImage::RescaleGamma::kSrc,
                SkImage::RescaleMode::kNearest,
                &Context::forwardReadbackCallback,
                context
            );

            graphiteContext->submit(skgpu::graphite::SyncToCpu::kYes);

            return true;
        }

        void sync() {
            graphiteContext->submit(skgpu::graphite::SyncToCpu::kYes);
        }

        static void forwardReadbackCallback(
            SkImage::ReadPixelsContext context, std::unique_ptr<const SkImage::AsyncReadResult> result) {
            std::function<SkImage::ReadPixelsCallback> *callback = reinterpret_cast<std::function<
                SkImage::ReadPixelsCallback> *>(context);
            (*callback)(context, std::move(result));
        }

        int getSurfaceWidth() {
            return surface->width();
        }

        int getSurfaceHeight() {
            return surface->height();
        }

        ~Context() {
            if (texture.isValid()) {
                graphiteContext->deleteBackendTexture(texture);
            }
            surface.reset();
            graphiteRecorder.reset();
            graphiteContext.reset();
            vkDestroyDevice(device, nullptr);
            vkDestroyInstance(instance, nullptr);
        }

    private:
        static bool initVulkan(JNIEnv *env, Context &context) {
            VkApplicationInfo appInfo = {};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Skia Graphite Vulkan Offscreen";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_2;

            VkInstanceCreateInfo instanceInfo = {};
            instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            instanceInfo.pApplicationInfo = &appInfo;

            if (auto result = vkCreateInstance(&instanceInfo, nullptr, &context.instance)) {
                throwRuntimeException(env, "Failed to create Vulkan instance: " + std::to_string(result));
                return false;
            }

            uint32_t gpuCount = 0;
            if (auto result = vkEnumeratePhysicalDevices(context.instance, &gpuCount, nullptr)) {
                throwRuntimeException(env, "Failed to get Vulkan GPU count: " + std::to_string(result));
                return false;
            }
            if (gpuCount == 0) {
                throwRuntimeException(env, "Failed to find a Vulkan capable GPU");
                return false;
            }
            std::vector<VkPhysicalDevice> gpus(gpuCount);
            if (auto res = vkEnumeratePhysicalDevices(context.instance, &gpuCount, gpus.data());
                res != VK_SUCCESS && res != VK_INCOMPLETE) {
                throwRuntimeException(env, "Failed to enumerate Vulkan GPUs: " + std::to_string(res));
                return false;
            }
            if (gpuCount == 0) {
                throwRuntimeException(env, "Failed to get the first Vulkan GPU");
                return false;
            }
            context.physicalDevice = gpus[0];

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(context.physicalDevice, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(context.physicalDevice, &queueFamilyCount, queueFamilies.data());
            for (uint32_t i = 0; i < queueFamilyCount; i++) {
                if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    context.queueIndex = i;
                    break;
                }
            }

            float queuePriority = 1.0f;
            VkDeviceQueueCreateInfo queueInfo = {};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = context.queueIndex;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &queuePriority;

            VkDeviceCreateInfo deviceInfo = {};
            deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            deviceInfo.queueCreateInfoCount = 1;
            deviceInfo.pQueueCreateInfos = &queueInfo;

            if (auto result = vkCreateDevice(context.physicalDevice, &deviceInfo, nullptr, &context.device)) {
                throwRuntimeException(env, "Failed to create Vulkan device: " + std::to_string(result));
                return false;
            }
            vkGetDeviceQueue(context.device, context.queueIndex, 0, &context.queue);

            return true;
        }

        static bool initGraphit(JNIEnv *env, Context &context) {
            skgpu::VulkanBackendContext backendCtx;
            backendCtx.fInstance = context.instance;
            backendCtx.fPhysicalDevice = context.physicalDevice;
            backendCtx.fDevice = context.device;
            backendCtx.fQueue = context.queue;
            backendCtx.fGraphicsQueueIndex = context.queueIndex;
            backendCtx.fMaxAPIVersion = VK_API_VERSION_1_2;
            backendCtx.fGetProc = [](const char *name, VkInstance inst, VkDevice dev) -> PFN_vkVoidFunction {
                if (dev) {
                    if (auto p = vkGetDeviceProcAddr(dev, name)) return p;
                }
                if (auto p = vkGetInstanceProcAddr(inst, name)) return p;
                return vkGetInstanceProcAddr(VK_NULL_HANDLE, name);
            };
            skgpu::graphite::ContextOptions contextOptions;
            context.graphiteContext = skgpu::graphite::ContextFactory::MakeVulkan(backendCtx, contextOptions);
            if (!context.graphiteContext) {
                throwRuntimeException(env, "Failed to create Vulkan Graphite context");
                return false;
            }

            context.graphiteRecorder = context.graphiteContext->makeRecorder();
            if (!context.graphiteRecorder) {
                throwRuntimeException(env, "Failed to create Vulkan Graphite recorder");
                return false;
            }

            return true;
        }

    private:
        Context() = default;

        VkInstance instance{};
        VkPhysicalDevice physicalDevice{};
        VkDevice device{};
        VkQueue queue{};
        uint32_t queueIndex{};

        std::unique_ptr<skgpu::graphite::Context> graphiteContext;
        std::unique_ptr<skgpu::graphite::Recorder> graphiteRecorder;

        skgpu::graphite::BackendTexture texture;
        sk_sp<SkSurface> surface;
        SkImageInfo imageInfo;
    };
}

extern "C" {
JNIEXPORT jlong JNICALL
Java_org_jetbrains_skiko_swing_LinuxVulkanRedrawer_makeNativeContext(
    JNIEnv *env, jobject self) {
    return reinterpret_cast<jlong>(Context::make(env).release());
}

JNIEXPORT jobject JNICALL
Java_org_jetbrains_skiko_swing_LinuxVulkanRedrawer_getSurface(
    JNIEnv *env, jobject self, jlong pNativeContext, jint width, jint height) {
    auto context = reinterpret_cast<Context *>(pNativeContext);

    jclass surfaceCls = env->FindClass("org/jetbrains/skia/Surface");
    if (!surfaceCls) {
        throwRuntimeException(env, "Failed to find org/jetbrains/skia/Surface class");
        return nullptr;
    }
    jmethodID ctor = env->GetMethodID(surfaceCls, "<init>", "(J)V");
    if (!ctor) {
        throwRuntimeException(env, "Failed to find Surface constructor");
        return nullptr;
    }

    auto sk_surface = context->createSurface(width, height);
    auto result = env->NewObject(surfaceCls, ctor, sk_surface.get());
    if (env->ExceptionCheck()) {
        return nullptr;
    }
    sk_surface->ref();
    return result;
}

JNIEXPORT jobject JNICALL
Java_org_jetbrains_skiko_swing_LinuxVulkanRedrawer_flushAndReadback(
    JNIEnv *env, jobject self, jlong pNativeContext, jobject image) {
    auto context = reinterpret_cast<Context *>(pNativeContext);
    jobject newImage = nullptr;

    auto callback = [=, &newImage](SkImage::ReadPixelsContext, std::unique_ptr<const SkImage::AsyncReadResult> result) {
        const auto w = context->getSurfaceWidth();
        const auto h = context->getSurfaceHeight();
        jclass cls = env->GetObjectClass(self);
        if (!cls) {
            throwRuntimeException(env, "Failed to find org/jetbrains/skia/SwingRedrawer class");
            return;
        }
        jmethodID mid = env->GetMethodID(cls, "createImage",
                                         "(Ljava/awt/image/BufferedImage;JII)Ljava/awt/image/BufferedImage;");
        if (!mid) {
            throwRuntimeException(env, "Failed to find createImage method");
            return;
        }
        jlong pBuffer = toJavaPointer(result->data(0));
        newImage = env->CallObjectMethod(self, mid, image, pBuffer, w, h);
    };

    context->flushAndReadback(callback);

    return newImage;
}

JNIEXPORT void JNICALL
Java_org_jetbrains_skiko_swing_LinuxVulkanRedrawer_disposeNativeContext(
    JNIEnv *env, jobject self, jlong pNativeContext) {
    if (auto context = reinterpret_cast<Context *>(pNativeContext)) {
        delete context;
    }
}

JNIEXPORT void JNICALL
Java_org_jetbrains_skiko_swing_LinuxVulkanRedrawer_sync(
    JNIEnv *env, jobject self, jlong pNativeContext) {
    auto context = reinterpret_cast<Context *>(pNativeContext);
    context->sync();
}

}
