#import "jawt.h"
#import "jawt_md.h"
#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>

#include <iostream>

extern "C"
{

/*
 * Class:     org_jetbrains_skiko_swing_SwingOffscreenDrawer
 * Method:    renderQueueFlushAndInvokeNow
 * Signature: (Ljava/lang/Runnable;)V
 */
JNIEXPORT void JNICALL Java_org_jetbrains_skiko_swing_SwingOffscreenDrawer_renderQueueFlushAndInvokeNow
  (JNIEnv * env, jobject self, jobject runnable) {
    // Get the MTLRenderQueue instance (assuming it has a static method)
    jclass renderQueueClass = env->FindClass("sun/java2d/metal/MTLRenderQueue");
    if (!renderQueueClass) {
        std::cerr << "Error: Unable to find MTLRenderQueue class." << std::endl;
        return;
    }

    jmethodID getInstanceMethod = env->GetStaticMethodID(renderQueueClass, "getInstance", "()Lsun/java2d/metal/MTLRenderQueue;");
    if (!getInstanceMethod) {
        std::cerr << "Error: Unable to find getInstance method in MTLRenderQueue." << std::endl;
        return;
    }

    jobject renderQueueInstance = env->CallStaticObjectMethod(renderQueueClass, getInstanceMethod);
    if (!renderQueueInstance) {
        std::cerr << "Error: Unable to get instance of MTLRenderQueue." << std::endl;
        return;
    }

    // Lock the render queue
    jmethodID lockMethod = env->GetMethodID(renderQueueClass, "lock", "()V");
    if (!lockMethod) {
        std::cerr << "Error: Unable to find lock method in MTLRenderQueue." << std::endl;
        return;
    }

    env->CallVoidMethod(renderQueueInstance, lockMethod);

    // Try block to flush and invoke runnable
    bool success = false;
    jmethodID flushAndInvokeNowMethod = env->GetMethodID(renderQueueClass, "flushAndInvokeNow", "(Ljava/lang/Runnable;)V");
    if (!flushAndInvokeNowMethod) {
        std::cerr << "Error: Unable to find flushAndInvokeNow method in MTLRenderQueue." << std::endl;
    } else {
        env->CallVoidMethod(renderQueueInstance, flushAndInvokeNowMethod, runnable);
        success = true;
    }

    // Unlock the render queue (finally block equivalent)
    jmethodID unlockMethod = env->GetMethodID(renderQueueClass, "unlock", "()V");
    if (!unlockMethod) {
        std::cerr << "Error: Unable to find unlock method in MTLRenderQueue." << std::endl;
    } else {
        env->CallVoidMethod(renderQueueInstance, unlockMethod);
    }

    // Clean up references
    if (renderQueueInstance) {
        env->DeleteLocalRef(renderQueueInstance);
    }
    env->DeleteLocalRef(renderQueueClass);

    // Handle if flushAndInvokeNow failed
    if (!success) {
        std::cerr << "Error: flushAndInvokeNow call failed." << std::endl;
    }
  }

/*
 * Class:     org_jetbrains_skiko_swing_SwingOffscreenDrawer
 * Method:    copyTexture
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_org_jetbrains_skiko_swing_SwingOffscreenDrawer_copyTexture
  (JNIEnv *env, jobject, jlong pSrc, jlong pDst) {
       if (!pSrc || !pDst) {
            NSLog(@"Error: Invalid texture pointers.");
            return;
        }

        id<MTLTexture> srcTexture = (__bridge_transfer id <MTLTexture>) (void *) pSrc;
        id<MTLTexture> dstTexture = (__bridge_transfer id <MTLTexture>) (void *)pDst;

        if (!srcTexture || !dstTexture || srcTexture.device != dstTexture.device) {
            NSLog(@"Error: Invalid Metal textures or mismatched devices.");
            return;
        }

//        if (srcTexture.width != dstTexture.width || srcTexture.height != dstTexture.height) {
//            NSLog(@"Error: Source and destination texture dimensions do not match.");
//            return JNI_FALSE;
//        }

        @autoreleasepool {
            id<MTLCommandQueue> commandQueue = [srcTexture.device newCommandQueue];
            if (!commandQueue) {
                NSLog(@"Error: Failed to create Metal command queue.");
                return;
            }

            id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
            if (!commandBuffer) {
                NSLog(@"Error: Failed to create Metal command buffer.");
                return;
            }

            id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];
            if (!blitEncoder) {
                NSLog(@"Error: Failed to create Metal blit command encoder.");
                return;
            }

            MTLSize size = MTLSizeMake(srcTexture.width, srcTexture.height, 1);
            [blitEncoder copyFromTexture:srcTexture
                             sourceSlice:0
                             sourceLevel:0
                            sourceOrigin:MTLOriginMake(0, 0, 0)
                              sourceSize:size
                               toTexture:dstTexture
                        destinationSlice:0
                        destinationLevel:0
                       destinationOrigin:MTLOriginMake(0, 0, 0)];

            [blitEncoder endEncoding];
            [commandBuffer commit];
            [commandBuffer waitUntilCompleted];

            if (commandBuffer.error) {
                NSLog(@"Error: Command buffer failed with error: %@", commandBuffer.error);
                return;
            }
        }
  }

/*
 * Class:     org_jetbrains_skiko_swing_SwingOffscreenDrawer
 * Method:    scaleTexture
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_org_jetbrains_skiko_swing_SwingOffscreenDrawer_scaleTexture
  (JNIEnv *, jobject, jlong pSrc, jlong pDst) {
    // TODO if needed
  }

/*
 * Class:     org_jetbrains_skiko_swing_SwingOffscreenDrawer
 * Method:    getVolatileImageTexture
 * Signature: (Ljava/awt/image/VolatileImage;)J
 */
JNIEXPORT jlong JNICALL Java_org_jetbrains_skiko_swing_SwingOffscreenDrawer_getVolatileImageTexture
  (JNIEnv * env, jobject, jobject vi) {
        if (!vi) {
            NSLog(@"Error: VolatileImage object is null.");
            return 0;
        }

        jclass volatileImageClass = env->GetObjectClass(vi);
        if (!volatileImageClass) {
            NSLog(@"Error: Failed to retrieve VolatileImage class.");
            return 0;
        }

        jfieldID volSurfaceManagerField = env->GetFieldID(volatileImageClass, "volSurfaceManager", "Lsun/awt/image/VolatileSurfaceManager;");
        if (!volSurfaceManagerField) {
            NSLog(@"Error: Failed to find volSurfaceManager field in VolatileImage.");
            return 0;
        }

        jobject volSurfaceManager = env->GetObjectField(vi, volSurfaceManagerField);
        if (!volSurfaceManager) {
            NSLog(@"Error: volSurfaceManager is null.");
            return 0;
        }

        jclass volSurfaceManagerClass = env->GetObjectClass(volSurfaceManager);
        if (!volSurfaceManagerClass) {
            NSLog(@"Error: Failed to retrieve VolatileSurfaceManager class.");
            return 0;
        }

        jmethodID getPrimarySurfaceDataMethod = env->GetMethodID(volSurfaceManagerClass, "getPrimarySurfaceData", "()Lsun/java2d/SurfaceData;");
        if (!getPrimarySurfaceDataMethod) {
            NSLog(@"Error: Failed to retrieve getPrimarySurfaceData method in VolatileSurfaceManager.");
            return 0;
        }

        jobject surfaceData = env->CallObjectMethod(volSurfaceManager, getPrimarySurfaceDataMethod);
        if (!surfaceData) {
            NSLog(@"Error: SurfaceData object retrieval failed.");
            return 0;
        }

        jclass surfaceDataClass = env->GetObjectClass(surfaceData);
        if (!surfaceDataClass) {
            NSLog(@"Error: Failed to retrieve SurfaceData class.");
            return 0;
        }

        jmethodID getNativeOpsMethod = env->GetMethodID(surfaceDataClass, "getNativeOps", "()J");
        if (!getNativeOpsMethod) {
            NSLog(@"Error: Failed to retrieve getNativeOps method.");
            return 0;
        }

        jlong nativeOpsHandle = env->CallLongMethod(surfaceData, getNativeOpsMethod);
        if (nativeOpsHandle == 0) {
            NSLog(@"Error: getNativeOps returned an invalid handle.");
            return 0;
        }

        jmethodID getMTLTexturePointerMethod = env->GetMethodID(surfaceDataClass, "getMTLTexturePointer", "(J)J");
        if (!getMTLTexturePointerMethod) {
            NSLog(@"Error: Failed to retrieve getMTLTexturePointer method from SurfaceData.");
            return 0;
        }

        jlong texturePointer = env->CallLongMethod(surfaceData, getMTLTexturePointerMethod, nativeOpsHandle);
        if (!texturePointer) {
            NSLog(@"Error: getMTLTexturePointer returned an invalid Metal texture pointer.");
            return 0;
        }

        NSLog(@"Successfully retrieved Metal texture pointer: %ld", texturePointer);
        return texturePointer;
    }
}
