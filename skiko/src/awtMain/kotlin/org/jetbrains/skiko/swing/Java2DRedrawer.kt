package org.jetbrains.skiko.swing

import org.jetbrains.skiko.GraphicsApi
import org.jetbrains.skiko.MetalAdapter
import org.jetbrains.skiko.SkiaLayerAnalytics
import org.jetbrains.skiko.SkikoRenderDelegate
import org.jetbrains.skiko.chooseMetalAdapter
import java.awt.Graphics2D

internal class Java2DRedrawer(val swingLayerProperties: SwingLayerProperties,
                         private val renderDelegate: SkikoRenderDelegate,
                         analytics: SkiaLayerAnalytics) : SwingRedrawerBase(swingLayerProperties, analytics, GraphicsApi.SOFTWARE_FAST) {
    private val adapter: MetalAdapter = chooseMetalAdapter(swingLayerProperties.adapterPriority).also {
        onDeviceChosen(it.name)
    }

    override fun onRender(g: Graphics2D, width: Int, height: Int, nanoTime: Long) {
        g.scale(1 / swingLayerProperties.scale.toDouble(), 1 / swingLayerProperties.scale.toDouble())
        val canvas = Java2DCanvas(g)
        renderDelegate.onRender(canvas, width, height, nanoTime)
    }
}