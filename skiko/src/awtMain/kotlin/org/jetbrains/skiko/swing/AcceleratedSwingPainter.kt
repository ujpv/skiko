package org.jetbrains.skiko.swing

import com.jetbrains.Extensions
import org.jetbrains.skia.Surface
import java.awt.Graphics2D
import com.jetbrains.JBR
import com.jetbrains.SharedTextures
import org.jetbrains.skiko.RenderException
import java.awt.GraphicsConfiguration
import java.awt.Image


internal class AcceleratedSwingPainter(val redrawerTextureType: Int) : SwingPainter {
    private val sharedTextures = JBR.getSharedTextures(Extensions.SHARED_TEXTURES_OPENGL)

    private var imageWrapper: Image? = null
    private var texturePtr: Long = 0L
    private var gc: GraphicsConfiguration? = null

    override fun paint(g: Graphics2D, surface: Surface, texture: Long) {
        val deviceTextureType = sharedTextures.getTextureType(g.deviceConfiguration)
        if (redrawerTextureType != deviceTextureType) {
            throw RenderException("Incompatible texture type. Device: $deviceTextureType, Painter: $redrawerTextureType")
        }
        if (g.deviceConfiguration != gc || texturePtr != texture || imageWrapper == null) {
            gc = g.deviceConfiguration
            texturePtr = texture
            imageWrapper = sharedTextures.wrapTexture(gc, texturePtr)
        }

        g.drawImage(imageWrapper, 0, 0, null)
    }

    override fun dispose() {
    }
}
