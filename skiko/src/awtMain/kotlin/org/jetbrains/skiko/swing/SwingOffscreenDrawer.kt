package org.jetbrains.skiko.swing

import com.jetbrains.JBR
import org.jetbrains.skia.Bitmap
import java.awt.Graphics2D
import java.awt.Transparency
import java.awt.image.VolatileImage

internal class SwingOffscreenDrawer(
    private val swingLayerProperties: SwingLayerProperties
) {
    @Volatile
    private var volatileImage: VolatileImage? = null

    init {
        if (!JBR.isNativeRasterLoaderSupported()) {
            throw RuntimeException("Native raster loader is not supported")
        }
    }

    /**
     * Draws rendered image that is represented by [bytes] on [g].
     *
     * If size of the rendered image is bigger than size from [swingLayerProperties]
     * then only part of the image will be drawn on [g].
     *
     * @param g graphics where rendered picture given in [bytes] should be drawn
     * @param bytes bytes of rendered picture in little endian order
     * @param width width of rendered picture in real pixels
     * @param height height of rendered picture in real pixels
     */
    fun draw(g: Graphics2D, bitmap: Bitmap, width: Int, height: Int) {
        if (width <= 0 || height <= 0) {
            return
        }
        drawVolatileImage(g, bitmap)
    }

    fun drawVolatileImage(g: Graphics2D, bitmap: Bitmap) {
        if (volatileImage == null ||
            volatileImage?.width != bitmap.width || volatileImage?.height != bitmap.height ||
            volatileImage!!.validate(g.deviceConfiguration) == VolatileImage.IMAGE_INCOMPATIBLE) {
            volatileImage = g.deviceConfiguration.createCompatibleVolatileImage(bitmap.width, bitmap.height, Transparency.TRANSLUCENT)
        }

        do {
            val nativeRasterLoader = JBR.getNativeRasterLoader()
            nativeRasterLoader.loadNativeRaster(volatileImage, bitmap.getNativeImagePtr(), bitmap.width, bitmap.height, 0, 0)
            g.drawImage(volatileImage, 0, 0, null)
        } while (volatileImage!!.contentsLost())
    }
}