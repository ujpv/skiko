package org.jetbrains.skiko.swing

import com.jetbrains.Extensions
import org.jetbrains.skia.*
import org.jetbrains.skiko.*
import java.awt.Graphics2D
import com.jetbrains.SharedTextures
import com.jetbrains.JBR

internal class WindowsOpenGLSwingRedrawer(
    swingLayerProperties: SwingLayerProperties,
    private val renderDelegate: SkikoRenderDelegate,
    analytics: SkiaLayerAnalytics
) : SwingRedrawerBase(swingLayerProperties, analytics, GraphicsApi.OPENGL) {
    val sharedTextureService = JBR.getSharedTextures(Extensions.SHARED_TEXTURES_OPENGL)
        ?: throw RenderException(
            "OpenGL shared textures are not supported"
        )

    init {
        if (sharedTextureService.getTextureType(swingLayerProperties.graphicsConfiguration) != SharedTextures.OPENGL_TEXTURE_TYPE) {
            throw RenderException("Cannot create OpenGL context: Shared textures are not OpenGL textures")
        }
    }

    val sharedTextures: SharedTextures = JBR.getSharedTextures(Extensions.SHARED_TEXTURES_OPENGL).also {
        if (it == null) {
            throw RenderException("Cannot get shared textures")
        }
    }

    val profiler = Profiler("WindowsOpenGLSwingRedrawer")
    init {
        Library.load()
        loadOpenGLLibrary()

        onDeviceChosen("WGL OffScreen") // Choosing WGL for offscreen rendering
    }

    private val offScreenContextPtr: Long = makeOffScreenContext(
        sharedTextures.getSharedGLContext(swingLayerProperties.graphicsConfiguration),
        sharedTextures.getGLPixelFormat(swingLayerProperties.graphicsConfiguration)
    ).also {
        if (it == 0L) {
            throw RenderException("Cannot create GL context")
        }
    }

    private val painter: SwingPainter = AcceleratedSwingPainter(SharedTextures.OPENGL_TEXTURE_TYPE)

    private var offScreenBufferPtr: Long = 0L

    init {
        if (!initNative()) {
            throw RenderException("Cannot initialize native")
        }

        onContextInit()
    }

    override fun dispose() {
        disposeOffScreenContext(offScreenContextPtr)
        disposeOffScreenBuffer(offScreenBufferPtr)
        painter.dispose()
    }

    override fun onRender(g: Graphics2D, width: Int, height: Int, nanoTime: Long) {
        profiler.onFrameBegin()
        profiler.onRenderBegin()
        val oldBufferPtr = offScreenBufferPtr
        offScreenBufferPtr = makeOffScreenBuffer(offScreenContextPtr, offScreenBufferPtr, width, height)
        if (oldBufferPtr != offScreenBufferPtr) {
            println("offScreenBufferPtr changed $oldBufferPtr -> $offScreenBufferPtr")
        }

        if (offScreenBufferPtr == 0L) {
            throw RenderException("Cannot create offScreen OpenGL buffer")
        }

        startRendering(offScreenContextPtr, offScreenBufferPtr)
        try {
            autoCloseScope {
                val fbId = getFboId(offScreenBufferPtr)
                val renderTarget = makeGLRenderTarget(
                    width,
                    height,
                    0,
                    8,
                    fbId,
                    FramebufferFormat.GR_GL_RGBA8
                ).autoClose()

                val directContext = makeGLContext().autoClose()
                val surface = Surface.makeFromBackendRenderTarget(
                    directContext,
                    renderTarget,
                    SurfaceOrigin.TOP_LEFT,
                    SurfaceColorFormat.BGRA_8888,
                    ColorSpace.sRGB,
                    SurfaceProps(pixelGeometry = PixelGeometry.UNKNOWN)
                )?.autoClose() ?: throw RenderException("Cannot create surface")

                val canvas = surface.canvas
                canvas.clear(Color.TRANSPARENT)
                renderDelegate.onRender(canvas, width, height, nanoTime)
                flush(surface, g)
            }
        } finally {
            finishRendering(offScreenContextPtr)
        }
        profiler.onFrameEnd()
        profiler.printEvery10Sec()
    }

    private fun flush(surface: Surface, g: Graphics2D) {
        surface.flushAndSubmit(syncCpu = false)
        profiler.onRenderEnd()
        profiler.onDrawBegin()
        var texture = getTextureId(offScreenBufferPtr)
        painter.paint(g, surface, texture)
        profiler.onDrawEnd()
    }

    private external fun initNative(): Boolean
    private external fun makeOffScreenContext(sharedContext: Long, pixelFormat: Int): Long
    private external fun disposeOffScreenContext(contextPtr: Long): Long

    private external fun makeOffScreenBuffer(contextPtr: Long, oldBufferPtr: Long, width: Int, height: Int): Long
    private external fun disposeOffScreenBuffer(bufferPtr: Long)

    private external fun startRendering(contextPtr: Long, bufferPtr: Long)
    private external fun finishRendering(contextPtr: Long)

//    private external fun createAndBindTexture(width: Int, height: Int): Long
    private external fun getFboId(offscreenBuffer: Long): Int
    private external fun getTextureId(offscreenBuffer: Long): Long
//    private external fun unbindAndDisposeTexture(texturePtr: Long)
}