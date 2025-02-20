package org.jetbrains.skiko.swing

import com.jetbrains.JBR
import org.jetbrains.skia.Bitmap
import java.awt.Graphics2D
import java.awt.Transparency
import java.awt.image.VolatileImage

// TODO: extract this code to a library and share it with JCEF implementation in IntelliJ
//  since this code is mostly taken from intellij repository with some small changes
internal class SwingOffscreenDrawer(
    private val swingLayerProperties: SwingLayerProperties
) {
    @Volatile
    private var volatileImage: VolatileImage? = null

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
            JBR.getNativeRasterLoader().loadNativeRaster(volatileImage, bitmap.getNativeImagePtr(), bitmap.width, bitmap.height, 0, 0)
            g.drawImage(volatileImage, 0, 0, null)
        } while (volatileImage!!.contentsLost())
    }
}