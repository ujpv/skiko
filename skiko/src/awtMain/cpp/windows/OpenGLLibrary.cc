#include <windows.h>
#include <gl/GL.h>
#include <jawt_md.h>
#include "exceptions_handler.h"

static HINSTANCE OpenGL32Library = nullptr;

extern "C" {
    void glFinish(void) {
        typedef void (*PROC_glFinish) (void);
        static auto glFinish = (PROC_glFinish) GetProcAddress(OpenGL32Library, "glFinish");
        glFinish();
    }

    void glFlush(void) {
        typedef void (*PROC_glFlush) (void);
        static auto glFlush = (PROC_glFlush) GetProcAddress(OpenGL32Library, "glFlush");
        glFlush();
    }

    void glGetIntegerv(GLenum pname, GLint *data) {
        typedef void (*PROC_glGetIntegerv) (GLenum pname, GLint *data);
        static auto glGetIntegerv = (PROC_glGetIntegerv) GetProcAddress(OpenGL32Library, "glGetIntegerv");
        glGetIntegerv(pname, data);
    }

    const GLubyte * glGetString(GLenum name) {
        typedef const GLubyte *(*PROC_glGetString) (GLenum name);
        static auto glGetString = (PROC_glGetString) GetProcAddress(OpenGL32Library, "glGetString");
        return glGetString(name);
    }

    HGLRC WINAPI wglCreateContext(HDC hDc) {
        typedef HGLRC (WINAPI * PROC_wglCreateContext) (HDC hDc);
        static auto wglCreateContext = (PROC_wglCreateContext) GetProcAddress(OpenGL32Library, "wglCreateContext");
        return wglCreateContext(hDc);
    }

    BOOL WINAPI wglDeleteContext(HGLRC oldContext) {
        typedef BOOL (WINAPI * PROC_wglDeleteContext) (HGLRC oldContext);
        static auto wglDeleteContext = (PROC_wglDeleteContext) GetProcAddress(OpenGL32Library, "wglDeleteContext");
        return wglDeleteContext(oldContext);
    }

    PROC WINAPI wglGetProcAddress(LPCSTR lpszProc) {
        typedef PROC (WINAPI * PROC_wglGetProcAddress) (LPCSTR lpszProc);
        static auto wglGetProcAddress = (PROC_wglGetProcAddress) GetProcAddress(OpenGL32Library, "wglGetProcAddress");
        return wglGetProcAddress(lpszProc);
    }

    BOOL WINAPI wglShareLists(HGLRC context1, HGLRC context2) {
        typedef BOOL (WINAPI * PROC_wglShareLists) (HGLRC context1, HGLRC context2);
        static auto wglShareLists = (PROC_wglShareLists) GetProcAddress(OpenGL32Library, "wglShareLists");
        return wglShareLists(context1, context2);
    }

    BOOL WINAPI wglMakeCurrent(HDC hDc, HGLRC newContext) {
        typedef BOOL (WINAPI * PROC_wglMakeCurrent) (HDC hDc, HGLRC newContext);
        static auto wglMakeCurrent = (PROC_wglMakeCurrent) GetProcAddress(OpenGL32Library, "wglMakeCurrent");
        return wglMakeCurrent(hDc, newContext);
    }

    HGLRC WINAPI wglGetCurrentContext() {
        typedef HGLRC (WINAPI * PROC_wglGetCurrentContext) (void);
        static auto wglGetCurrentContext = (PROC_wglGetCurrentContext) GetProcAddress(OpenGL32Library, "wglGetCurrentContext");
        return wglGetCurrentContext();
    }

    void WINAPI glGenTextures(GLsizei n, GLuint* textures) {
        typedef void (APIENTRY* PROC_glGenTextures)(GLsizei n, GLuint* textures);
        static auto glGenTextures = (PROC_glGenTextures)GetProcAddress(OpenGL32Library, "glGenTextures");
        glGenTextures(n, textures);
    }

    void WINAPI glBindTexture(GLenum target, GLuint texture) {
        typedef void (APIENTRY* PROC_glBindTexture)(GLenum target, GLuint texture);
        static auto glBindTexture = (PROC_glBindTexture)GetProcAddress(OpenGL32Library, "glBindTexture");
        glBindTexture(target, texture);
    }

    void WINAPI glTexParameteri(GLenum target, GLenum pname, GLint param) {
        typedef void (APIENTRY* PROC_glTexParameteri)(GLenum target, GLenum pname, GLint param);
        static auto glTexParameteri = (PROC_glTexParameteri)GetProcAddress(OpenGL32Library, "glTexParameteri");
        glTexParameteri(target, pname, param);
    }

//    void WINAPI glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
//        typedef void (APIENTRY* PROC_glClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
//        static auto glClearColor = (PROC_glClearColor)GetProcAddress(OpenGL32Library, "glClearColor");
//        glClearColor(red, green, blue, alpha);
//    }
//
//    void WINAPI glClear(GLbitfield mask) {
//        typedef void (APIENTRY* PROC_glClear)(GLbitfield mask);
//        static auto glClear = (PROC_glClear)GetProcAddress(OpenGL32Library, "glClear");
//        glClear(mask);
//    }

    GLenum WINAPI glGetError() {
        typedef GLenum (APIENTRY* PROC_glGetError)(void);
        static auto glGetError = (PROC_glGetError)GetProcAddress(OpenGL32Library, "glGetError");
        return glGetError();
    }

    void WINAPI glDeleteTextures(GLsizei n, const GLuint* textures) {
        typedef void (APIENTRY* PROC_glDeleteTextures)(GLsizei n, const GLuint* textures);
        static auto glDeleteTextures = (PROC_glDeleteTextures)GetProcAddress(OpenGL32Library, "glDeleteTextures");
        glDeleteTextures(n, textures);
    }

    void WINAPI glTexImage2D(GLenum target, GLint level, GLint internalFormat,
                      GLsizei width, GLsizei height, GLint border,
                      GLenum format, GLenum type, const void* data) {
        typedef void (APIENTRY* PROC_glTexImage2D)(GLenum target, GLint level, GLint internalFormat,
                                                   GLsizei width, GLsizei height, GLint border,
                                                   GLenum format, GLenum type, const void* data);
        static auto glTexImage2D = (PROC_glTexImage2D)GetProcAddress(OpenGL32Library, "glTexImage2D");
        glTexImage2D(target, level, internalFormat, width, height, border, format, type, data);
    }

    void WINAPI glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers) {
        typedef void (APIENTRY* PROC_glDeleteFramebuffers)(GLsizei n, const GLuint* framebuffers);
        static auto glDeleteFramebuffers = (PROC_glDeleteFramebuffers)wglGetProcAddress("glDeleteFramebuffers");

        glDeleteFramebuffers(n, framebuffers);
    }

    void WINAPI glBindFramebuffer(GLenum target, GLuint framebuffer) {
        typedef void (APIENTRY* PROC_glBindFramebuffer)(GLenum target, GLuint framebuffer);
        static auto glBindFramebuffer = (PROC_glBindFramebuffer)wglGetProcAddress("glBindFramebuffer");
        glBindFramebuffer(target, framebuffer);
    }

    void WINAPI glGenFramebuffers(GLsizei n, GLuint* framebuffers) {
        typedef void (APIENTRY* PROC_glGenFramebuffers)(GLsizei n, GLuint* framebuffers);
        static auto glGenFramebuffers = (PROC_glGenFramebuffers)wglGetProcAddress("glGenFramebuffers");
        glGenFramebuffers(n, framebuffers);
    }

    void WINAPI glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
        typedef void (APIENTRY* PROC_glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
        static auto glFramebufferTexture2D = (PROC_glFramebufferTexture2D)wglGetProcAddress("glFramebufferTexture2D");
        glFramebufferTexture2D(target, attachment, textarget, texture, level);
    }

    GLenum WINAPI glCheckFramebufferStatus(GLenum target) {
        typedef GLenum (APIENTRY* PROC_glCheckFramebufferStatus)(GLenum target);
        static auto glCheckFramebufferStatus = (PROC_glCheckFramebufferStatus)wglGetProcAddress("glCheckFramebufferStatus");
        return glCheckFramebufferStatus(target);
    }

    JNIEXPORT void JNICALL Java_org_jetbrains_skiko_OpenGLLibrary_1jvmKt_loadOpenGLLibraryWindows(JNIEnv *env, jobject obj) {
        OpenGL32Library = LoadLibrary("opengl32.dll");
        if (OpenGL32Library == nullptr) {
            auto code = GetLastError();
            throwJavaRenderExceptionByErrorCode(env, __FUNCTION__, code);
        }
    }
}