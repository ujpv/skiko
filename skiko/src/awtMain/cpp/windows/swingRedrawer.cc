#include <windows.h>
#include <GL/gl.h>

#include <iostream>

#include "jni_helpers.h"

#define GL_FRAMEBUFFER                      0x8D40
#define GL_COLOR_ATTACHMENT0                0x8CE0
#define GL_FRAMEBUFFER_COMPLETE             0x8CD5

namespace {
    // typedef void (APIENTRY *PFNGLDELETEFRAMEBUFFERSPROC) (GLsizei, const GLuint*);
    // typedef void (APIENTRY* PFNGLGENFRAMEBUFFERSPROC) (GLsizei, GLuint*);
    // typedef void (APIENTRY* PFNGLBINDFRAMEBUFFERPROC) (GLenum, GLuint);
    // typedef void (APIENTRY* PFNGLFRAMEBUFFERTEXTURE2DPROC) (GLenum, GLenum, GLenum, GLuint, GLint);
    // typedef GLenum (APIENTRY* PFNGLCHECKFRAMEBUFFERSTATUSPROC)(GLenum);
    //
    // typedef void (APIENTRY *PFNGLDELETETEXTURESPROC)(GLsizei n, const GLuint* textures);
    // typedef void (APIENTRY *PFNGLGENTEXTURESPROC)(GLsizei n, GLuint* textures);
    // typedef void (APIENTRY *PFNGLBINDTEXTUREPROC)(GLenum target, GLuint texture);
    // typedef void (APIENTRY *PFNGLTEXIMAGE2DPROC)(GLenum target, GLint level, GLint internalFormat,
    //                                              GLsizei width, GLsizei height, GLint border,
    //                                              GLenum format, GLenum type, const void* data);



    // PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = nullptr;
    // PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = nullptr;
    // PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = nullptr;
    // PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = nullptr;
    // PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = nullptr;
    //
    // PFNGLDELETETEXTURESPROC glDeleteTexturesDl = nullptr;
    // PFNGLGENTEXTURESPROC glGenTexturesDl = nullptr;
    // PFNGLBINDTEXTUREPROC glBindTextureDl = nullptr;
    // PFNGLTEXIMAGE2DPROC glTexImage2DDl = nullptr;

    #include <GL/gl.h>
#include <windows.h>
#include <iostream>

// Wrapper function for dynamic loading and calling OpenGL functions
// void glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers) {
//     static PFNGLDELETEFRAMEBUFFERSPROC pfnDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");
//     if (!pfnDeleteFramebuffers) {
//         std::cerr << "Failed to call glDeleteFramebuffers (not loaded)" << std::endl;
//         return;
//     }
//     pfnDeleteFramebuffers(n, framebuffers);
// }
//
// void glGenFramebuffers(GLsizei n, GLuint* framebuffers) {
//     static PFNGLGENFRAMEBUFFERSPROC pfnGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
//     if (!pfnGenFramebuffers) {
//         std::cerr << "Failed to call glGenFramebuffers (not loaded)" << std::endl;
//         return;
//     }
//     pfnGenFramebuffers(n, framebuffers);
// }
//
// void glBindFramebuffer(GLenum target, GLuint framebuffer) {
//     static PFNGLBINDFRAMEBUFFERPROC pfnBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
//     if (!pfnBindFramebuffer) {
//         std::cerr << "Failed to call glBindFramebuffer (not loaded)" << std::endl;
//         return;
//     }
//     pfnBindFramebuffer(target, framebuffer);
// }
//
// void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
//     static PFNGLFRAMEBUFFERTEXTURE2DPROC pfnFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D");
//     if (!pfnFramebufferTexture2D) {
//         std::cerr << "Failed to call glFramebufferTexture2D (not loaded)" << std::endl;
//         return;
//     }
//     pfnFramebufferTexture2D(target, attachment, textarget, texture, level);
// }
//
// GLenum glCheckFramebufferStatus(GLenum target) {
//     static PFNGLCHECKFRAMEBUFFERSTATUSPROC pfnCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
//     if (!pfnCheckFramebufferStatus) {
//         std::cerr << "Failed to call glCheckFramebufferStatus (not loaded)" << std::endl;
//         return 0;
//     }
//     return pfnCheckFramebufferStatus(target);
// }
//
// void glDeleteTexturesDl(GLsizei n, const GLuint* textures) {
//     static PFNGLDELETETEXTURESPROC pfnDeleteTextures = (PFNGLDELETETEXTURESPROC)wglGetProcAddress("glDeleteTextures");
//     if (!pfnDeleteTextures) {
//         std::cerr << "Failed to call glDeleteTextures (not loaded)" << std::endl;
//         return;
//     }
//     pfnDeleteTextures(n, textures);
// }
//
// void glGenTexturesDl(GLsizei n, GLuint* textures) {
//     static PFNGLGENTEXTURESPROC pfnGenTextures = (PFNGLGENTEXTURESPROC)wglGetProcAddress("glGenTextures");
//     if (!pfnGenTextures) {
//         std::cerr << "Failed to call glGenTextures (not loaded)" << std::endl;
//         return;
//     }
//     pfnGenTextures(n, textures);
// }
//
// void glBindTextureDl(GLenum target, GLuint texture) {
//     static PFNGLBINDTEXTUREPROC pfnBindTexture = (PFNGLBINDTEXTUREPROC)wglGetProcAddress("glBindTexture");
//     if (!pfnBindTexture) {
//         std::cerr << "Failed to call glBindTexture (not loaded)" << std::endl;
//         return;
//     }
//     pfnBindTexture(target, texture);
// }
//
// void glTexImage2DDl(GLenum target, GLint level, GLint internalFormat,
//                   GLsizei width, GLsizei height, GLint border,
//                   GLenum format, GLenum type, const void* data) {
//     static PFNGLTEXIMAGE2DPROC pfnTexImage2D = (PFNGLTEXIMAGE2DPROC)wglGetProcAddress("glTexImage2D");
//     if (!pfnTexImage2D) {
//         std::cerr << "Failed to call glTexImage2D (not loaded)" << std::endl;
//         return;
//     }
//     pfnTexImage2D(target, level, internalFormat, width, height, border, format, type, data);
// }

}

extern "C" void APIENTRY glBindFramebuffer(GLenum target, GLuint framebuffer);
extern "C" void APIENTRY glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers);
extern "C" void APIENTRY glGenFramebuffers(GLsizei n, GLuint* framebuffers);
extern "C" void APIENTRY glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern "C" GLenum APIENTRY glCheckFramebufferStatus(GLenum target);


class OffScreenContext {
public:
    HDC hdc;
    HGLRC context;

    OffScreenContext(HDC _hdc, HGLRC _context) : hdc(_hdc), context(_context) {}

    ~OffScreenContext() {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(context);
        ReleaseDC(nullptr, hdc);
    }

    static OffScreenContext* create(HGLRC sharedContext, int pixelFormat) {
        if (!pixelFormat) {
            return nullptr;
        }

        HDC hdc = GetDC(nullptr); // Get device context for the whole screen
        if (!hdc) {
            return nullptr;
        }

        if (!SetPixelFormat(hdc, pixelFormat, nullptr)) {
            ReleaseDC(nullptr, hdc);
            return nullptr;
        }

        HGLRC context = wglCreateContext(hdc);
        if (!context || !wglMakeCurrent(hdc, context)) {
            if (context) wglDeleteContext(context);
            ReleaseDC(nullptr, hdc);
            return nullptr;
        }

        if (!wglShareLists(sharedContext, context)) {
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(context);
            ReleaseDC(nullptr, hdc);
        }

        return new OffScreenContext(hdc, context);
    }

    // bool makeShared(HDC _hdc, HGLRC _context) {
    //     if (hdc != _hdc || context != _context) {
    //         wglMakeCurrent(nullptr, nullptr);
    //         wglDeleteContext(context);
    //         ReleaseDC(nullptr, hdc);
    //         hdc = _hdc;
    //         context = _context;
    //         return true;
    //     }
    //     return false;
    // }
};

class OffScreenBuffer {
public:
    const GLuint fbo;
    const GLuint texture;
    const int width;
    const int height;

    OffScreenBuffer(const GLuint fbo, const GLuint _texture, int width, int height) : fbo(fbo), texture(_texture), width(width), height(height) {
    }

    ~OffScreenBuffer() {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &texture);
    }

    static OffScreenBuffer* create(const int width, const int height) {
        GLuint texture, fbo;

        glGenTextures(1, &texture);
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            fprintf(stderr, "Failed to create texture: %d\n", err);;
        } else {
            fprintf(stderr, "Created texture: %d\n", texture);;

        }

        glBindTexture(GL_TEXTURE_2D, texture);

        if (err != GL_NO_ERROR) {
            fprintf(stderr, "Failed to bind texture: %d\n", err);;
        }
        // remote it
//        unsigned char color[4] = {255, 0, 0, 255};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr /*color*/);
        if (err != GL_NO_ERROR) {
            fprintf(stderr, "Failed to glTexImage2D");
        }
        if (!wglGetCurrentContext()) {
            fprintf(stderr, "Error: No OpenGL context is active!\n");
            return nullptr;
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F /*GL_CLAMP_TO_EDGE*/);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F /*GL_CLAMP_TO_EDGE*/);
        // remove it
//        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        if (status != GL_FRAMEBUFFER_COMPLETE) {
            glDeleteFramebuffers(1, &fbo);
            glDeleteTextures(1, &texture);
            return nullptr;
        }

        return new OffScreenBuffer(fbo, texture, width, height);
    }
};

 extern "C" {
     JNIEXPORT jboolean JNICALL Java_org_jetbrains_skiko_swing_WindowsOpenGLSwingRedrawer_initNative(JNIEnv *env, jobject obj) {
         // if (!glFunctionsLoaded) {
         //     loadOpenGLFunctions();
         // }
         // return glFunctionsLoaded;
         const char* extensions = (const char*)glGetString(GL_EXTENSIONS);

         const char* version = (const char*)glGetString(GL_VERSION);
         std::cout << "OpenGL Version: " << version << std::endl;


         std::cout << "Supported Extensions: " << extensions << std::endl;

         return true;
     }

     JNIEXPORT jlong JNICALL Java_org_jetbrains_skiko_swing_WindowsOpenGLSwingRedrawer_makeOffScreenContext(JNIEnv* env, jobject redrawer, jlong sharedContextPtr, jint pixelFormat) {
         OffScreenContext* context = OffScreenContext::create(reinterpret_cast<HGLRC>(sharedContextPtr), pixelFormat);;
         return context ? toJavaPointer(context) : 0;
     }

     JNIEXPORT void JNICALL Java_org_jetbrains_skiko_swing_WindowsOpenGLSwingRedrawer_disposeOffScreenContext(JNIEnv* env, jobject redrawer, jlong contextPtr) {
         OffScreenContext* context = fromJavaPointer<OffScreenContext*>(contextPtr);
         delete context;
     }

     JNIEXPORT jlong JNICALL Java_org_jetbrains_skiko_swing_WindowsOpenGLSwingRedrawer_makeOffScreenBuffer(JNIEnv* env, jobject redrawer, jlong contextPtr, jlong oldBufferPtr, jint width, jint height) {
        OffScreenContext* context = fromJavaPointer<OffScreenContext *>(contextPtr);
         if (!wglMakeCurrent(context->hdc, context->context)) {
             std::cerr << "Failed to make OpenGL context current!" << std::endl;
             return 0;
         }


        auto wgl_get_current_context = wglGetCurrentContext();
         if (wgl_get_current_context != context->context) {
             std::cerr << "Current context is " << wgl_get_current_context << std::endl;
             return 0;
         }

        OffScreenBuffer* oldBuffer = fromJavaPointer<OffScreenBuffer *>(oldBufferPtr);
        OffScreenBuffer* buffer;
        if (oldBuffer == nullptr || oldBuffer->width != width || oldBuffer->height != height) {
            buffer = OffScreenBuffer::create(width, height);
            if (oldBuffer != nullptr) {
                delete oldBuffer;
            }
        } else {
            buffer = oldBuffer;
        }

        return toJavaPointer(buffer);
     }

     JNIEXPORT void JNICALL Java_org_jetbrains_skiko_swing_WindowsOpenGLSwingRedrawer_disposeOffScreenBuffer(JNIEnv* env, jobject redrawer, jlong bufferPtr) {
         OffScreenBuffer* buffer = fromJavaPointer<OffScreenBuffer *>(bufferPtr);
         if (buffer) {
             delete buffer;
         }
     }

     JNIEXPORT void JNICALL Java_org_jetbrains_skiko_swing_WindowsOpenGLSwingRedrawer_startRendering(JNIEnv* env, jobject redrawer, jlong contextPtr, jlong bufferPtr) {
         OffScreenContext* context = fromJavaPointer<OffScreenContext*>(contextPtr);
         wglMakeCurrent(context->hdc, context->context);
         auto offscreenBuffer = fromJavaPointer<OffScreenBuffer*>(bufferPtr);
         glBindFramebuffer(GL_FRAMEBUFFER, offscreenBuffer->fbo);
         glBindTexture(GL_TEXTURE_2D, offscreenBuffer->texture);
     }

     JNIEXPORT void JNICALL Java_org_jetbrains_skiko_swing_WindowsOpenGLSwingRedrawer_finishRendering(JNIEnv* env, jobject redrawer, jlong contextPtr) {
         wglMakeCurrent(nullptr, nullptr);
     }

     JNIEXPORT jint JNICALL Java_org_jetbrains_skiko_swing_WindowsOpenGLSwingRedrawer_getFboId(JNIEnv *env, jobject obj, jlong offscreenBuffer) {
         OffScreenBuffer* buffer = fromJavaPointer<OffScreenBuffer *>(offscreenBuffer);

         return (jint) buffer->fbo;
     }

     JNIEXPORT jlong JNICALL Java_org_jetbrains_skiko_swing_WindowsOpenGLSwingRedrawer_getTextureId(JNIEnv* env, jobject obj, jlong offscreenBuffer) {
         OffScreenBuffer* buffer = fromJavaPointer<OffScreenBuffer *>(offscreenBuffer);
         return (jlong) buffer->texture;
     }
 }
