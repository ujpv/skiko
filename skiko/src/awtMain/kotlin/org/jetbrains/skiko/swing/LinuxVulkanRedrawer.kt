package org.jetbrains.skiko.swing

import org.jetbrains.skia.Color
import org.jetbrains.skia.Surface
import org.jetbrains.skia.impl.BufferUtil
import org.jetbrains.skiko.GraphicsApi
import org.jetbrains.skiko.Library
import org.jetbrains.skiko.RenderException
import org.jetbrains.skiko.SkiaLayerAnalytics
import org.jetbrains.skiko.SkikoRenderDelegate
import java.awt.Graphics2D
import java.awt.Image
import java.awt.image.BufferedImage
import java.awt.image.DataBufferInt
import java.nio.ByteOrder.LITTLE_ENDIAN
import java.nio.IntBuffer
import kotlin.math.min

internal class LinuxVulkanRedrawer(
    swingLayerProperties: SwingLayerProperties,
    private val renderDelegate: SkikoRenderDelegate,
    analytics: SkiaLayerAnalytics
) : SwingRedrawerBase(swingLayerProperties, analytics, GraphicsApi.OPENGL) {
    private val painter = SoftwareSwingPainter(swingLayerProperties)
    private var pContext: Long = makeNativeContext().also {
        if (it == 0L) {
            throw RenderException("Cannot create Vulkan context")
        }
    }

    private var surface: Surface? = null
    private var bufferedImage: Image = BufferedImage(1, 1, BufferedImage.TYPE_INT_ARGB_PRE)

    private val profiler = Profiler("LinuxVulkanRedrawer")

    companion object {
        init {
            Library.load()
        }
    }

    init {
        onDeviceChosen("Vulkan OffScreen")
    }

    override fun onRender(g: Graphics2D, width: Int, height: Int, nanoTime: Long) {
        profiler.onFrameBegin()

        profiler.onBegin("Render")
        val surface = getSurface(pContext, width, height)
        val canvas = surface.canvas
        canvas.clear(Color.TRANSPARENT)
        renderDelegate.onRender(canvas, width, height, nanoTime)

        sync(pContext)
        profiler.onBegin("Readback")
        bufferedImage = flushAndReadback(pContext, bufferedImage)
        profiler.onEnd("Readback")
        profiler.onEnd("Render")

        profiler.onBegin("Paint")
        painter.paint(g, bufferedImage)
        profiler.onEnd("Paint")

        profiler.onFrameEnd()

        profiler.printEvery10Sec()
    }

    override fun dispose() {
        surface?.close()
        disposeNativeContext(pContext)
        super.dispose()
    }

    private fun createImage(oldImage: BufferedImage, pBuffer: Long, width: Int, height: Int): BufferedImage {
        var image = oldImage
        if (image.width != width || image.height != height) {
            image = BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB_PRE)
        }

        val dstData = (image.raster.dataBuffer as DataBufferInt).data
        val src = BufferUtil.getByteBufferFromPointer(pBuffer, image.width * image.height * 4)
        val srcData: IntBuffer = src.order(LITTLE_ENDIAN).asIntBuffer()
        srcData.position(0).get(dstData, 0, min(image.height * image.width, srcData.capacity()))

        return image
    }

    private fun getSurface(pNativeContext: Long, width: Int, height: Int, oldSurface: Surface?): Surface {
        if (oldSurface != null && oldSurface.width == width && oldSurface.height == height) {
            return oldSurface
        }

        oldSurface?.close()
        return getSurface(pNativeContext, width, height)
    }


    private external fun makeNativeContext(): Long
    private external fun getSurface(pNativeContext: Long, width: Int, height: Int): Surface
    private external fun flushAndReadback(pNativeContext: Long, oldImage: Image): Image
    private external fun disposeNativeContext(pContext: Long)
    private external fun sync(pNativeContext: Long): Long
}
