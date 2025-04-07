package org.jetbrains.skiko.swing

import org.jetbrains.skia.Bitmap
import org.jetbrains.skia.BlendMode
import org.jetbrains.skia.Canvas
import org.jetbrains.skia.ClipMode
import org.jetbrains.skia.Drawable
import org.jetbrains.skia.FilterMode
import org.jetbrains.skia.Font
import org.jetbrains.skia.IRect
import org.jetbrains.skia.Image
import org.jetbrains.skia.Matrix33
import org.jetbrains.skia.Matrix44
import org.jetbrains.skia.Paint
import org.jetbrains.skia.PaintMode
import org.jetbrains.skia.Path
import org.jetbrains.skia.PathVerb
import org.jetbrains.skia.Picture
import org.jetbrains.skia.Point
import org.jetbrains.skia.RRect
import org.jetbrains.skia.Rect
import org.jetbrains.skia.Region
import org.jetbrains.skia.SamplingMode
import org.jetbrains.skia.TextBlob
import org.jetbrains.skia.VertexMode
import org.jetbrains.skiko.toBufferedImage
import java.awt.BasicStroke
import java.awt.Color
import java.awt.Graphics2D
import java.awt.RenderingHints
import java.awt.Stroke
import kotlin.math.roundToInt

class Java2DCanvas(val g: Graphics2D) : Canvas(NullPointer, false, NullPointer) {
    private var currentColor: Color = Color.BLACK
    private var currentStroke: Stroke = BasicStroke(1.0F)
    private var renderingHints: RenderingHints = g.renderingHints

    private fun skiaColorToAwtColor(skiaColor: Int): Color {
        val red = (skiaColor shr 16) and 0xFF
        val green = (skiaColor shr 8) and 0xFF
        val blue = skiaColor and 0xFF
        val alpha = (skiaColor shr 24) and 0xFF
        return Color(red, green, blue, alpha)
    }

    private fun applyPaintToGraphics(paint: Paint) {
        currentColor = g.color
        currentStroke = g.stroke
        renderingHints = g.renderingHints
        g.color = skiaColorToAwtColor(paint.color)
        g.stroke = BasicStroke(paint.strokeWidth)
        if (paint.isAntiAlias) {
            g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON)
        }
    }

    private fun restorePaint() {
        g.color = currentColor
        g.stroke = currentStroke
        g.setRenderingHints(renderingHints)
    }

    private fun drawGlyph(
        canvas: Canvas,
        font: Font,
        glyphID: Short,
        position: Point,
        paint: Paint
    ) {
        val glyphPath = font.getPath(glyphID) // Get the path for the glyph
        if (glyphPath != null) {
            val translatedPath = glyphPath.apply {
                transform(Matrix33.makeTranslate(position.x, position.y))
            }
            canvas.drawPath(translatedPath, paint)
        }
    }

    override fun drawPoint(
        x: Float,
        y: Float,
        paint: Paint
    ): Canvas {
        applyPaintToGraphics(paint)
        g.drawLine(x.toInt(), y.toInt(), x.toInt(), y.toInt())
        restorePaint()
        return this
    }

    override fun drawPoints(coords: FloatArray, paint: Paint): Canvas {
        if (coords.size % 2 != 0) {
            throw IllegalArgumentException("Coordinates array must contain pairs of x and y values.")
        }
        applyPaintToGraphics(paint)
        for (i in coords.indices step 2) {
            val x = coords[i]
            val y = coords[i + 1]
            g.drawLine(x.toInt(), y.toInt(), x.toInt(), y.toInt()) // Simulate a single pixel point
        }
        restorePaint()
        return this
    }

    override fun drawLines(coords: FloatArray, paint: Paint): Canvas {
        if (coords.size % 4 != 0) {
            throw IllegalArgumentException("Coordinates array length must be a multiple of 4.")
        }
        applyPaintToGraphics(paint)
        for (i in coords.indices step 4) {
            val x0 = coords[i]
            val y0 = coords[i + 1]
            val x1 = coords[i + 2]
            val y1 = coords[i + 3]

            g.drawLine(x0.roundToInt(), y0.roundToInt(), x1.roundToInt(), y1.roundToInt())
        }
        restorePaint()
        return this
    }

    override fun drawPolygon(coords: FloatArray, paint: Paint): Canvas {
        if (coords.size % 2 != 0) {
            throw IllegalArgumentException("Coordinates array must contain pairs of x and y values.")
        }
        val xPoints = IntArray(coords.size / 2) { i -> coords[i * 2].toInt() }
        val yPoints = IntArray(coords.size / 2) { i -> coords[i * 2 + 1].toInt() }
        applyPaintToGraphics(paint)
        when (paint.mode) {
            PaintMode.FILL -> {
                g.fillPolygon(xPoints, yPoints, xPoints.size)
            }
            PaintMode.STROKE -> {
                g.drawPolygon(xPoints, yPoints, xPoints.size)
            }
            PaintMode.STROKE_AND_FILL -> {
                g.fillPolygon(xPoints, yPoints, xPoints.size)
                g.drawPolygon(xPoints, yPoints, xPoints.size)
            }
        }
        restorePaint()
        return this
    }

    override fun drawLine(x0: Float, y0: Float, x1: Float, y1: Float, paint: Paint): Canvas {
        applyPaintToGraphics(paint)
//        g.drawLine(x0.toInt(), y0.toInt(), x1.toInt(), y1.toInt())
        g.drawLine(x0.roundToInt(), y0.roundToInt(), x1.roundToInt(), y1.roundToInt())
        restorePaint()
        return this
    }

    override fun drawArc(
        left: Float,
        top: Float,
        right: Float,
        bottom: Float,
        startAngle: Float,
        sweepAngle: Float,
        includeCenter: Boolean,
        paint: Paint
    ): Canvas {
        val width = right - left
        val height = bottom - top
        if (width <= 0 || height <= 0) {
            throw IllegalArgumentException("Width and height of the arc must be greater than zero.")
        }
        applyPaintToGraphics(paint)
        val adjustedStartAngle = -startAngle  // Convert Skia counter-clockwise to Java clockwise
        val adjustedSweepAngle = -sweepAngle // Convert Skia counter-clockwise to Java clockwise

        if (includeCenter) {
            g.fillArc(left.toInt(), top.toInt(), width.toInt(), height.toInt(), adjustedStartAngle.toInt(), adjustedSweepAngle.toInt())
        } else {
            if (paint.mode == PaintMode.FILL || paint.mode == PaintMode.STROKE_AND_FILL) {
                g.fillArc(left.toInt(), top.toInt(), width.toInt(), height.toInt(), adjustedStartAngle.toInt(), adjustedSweepAngle.toInt())
            } else {
                g.drawArc(left.toInt(), top.toInt(), width.toInt(), height.toInt(), adjustedStartAngle.toInt(), adjustedSweepAngle.toInt())
            }
        }
        restorePaint()
        return this
    }

    override fun drawRect(r: Rect, paint: Paint): Canvas {
        applyPaintToGraphics(paint)
        g.drawRect(r.left.toInt(), r.top.toInt(), r.width.toInt(), r.height.toInt())
        restorePaint()
        return this
    }

    override fun drawOval(r: Rect, paint: Paint): Canvas {
        applyPaintToGraphics(paint)
        when (paint.mode) {
            PaintMode.FILL ->
                g.fillOval(r.left.roundToInt(), r.top.roundToInt(), r.width.roundToInt(), r.height.roundToInt())
            PaintMode.STROKE ->
                g.drawOval(r.left.roundToInt(), r.top.roundToInt(), r.width.roundToInt(), r.height.roundToInt())
            PaintMode.STROKE_AND_FILL ->
                g.fillOval(r.left.roundToInt(), r.top.roundToInt(), r.width.roundToInt(), r.height.roundToInt())
        }
        restorePaint()
        return this
    }

    override fun drawCircle(x: Float, y: Float, radius: Float, paint: Paint): Canvas {
        applyPaintToGraphics(paint)
        g.drawOval(
            (x - radius).toInt(),
            (y - radius).toInt(),
            (2 * radius).toInt(),
            (2 * radius).toInt()
        )
        restorePaint()
        return this
    }

    override fun drawRRect(r: RRect, paint: Paint): Canvas {
        val left = r.left
        val top = r.top
        val width = r.right - r.left
        val height = r.bottom - r.top

        if (width <= 0 || height <= 0) {
            throw IllegalArgumentException("Width and height of the rounded rectangle must be greater than zero.")
        }

        val rx = r.radii[0]
        val ry = r.radii[1]

        applyPaintToGraphics(paint)

        val arcWidth = (rx * 2).toInt()
        val arcHeight = (ry * 2).toInt()

        when (paint.mode) {
            PaintMode.FILL -> {
                g.fillRoundRect(left.toInt(), top.toInt(), width.toInt(), height.toInt(), arcWidth, arcHeight)
            }
            PaintMode.STROKE -> {
                g.drawRoundRect(left.toInt(), top.toInt(), width.toInt(), height.toInt(), arcWidth, arcHeight)
            }
            PaintMode.STROKE_AND_FILL -> {
                g.fillRoundRect(left.toInt(), top.toInt(), width.toInt(), height.toInt(), arcWidth, arcHeight)
                g.drawRoundRect(left.toInt(), top.toInt(), width.toInt(), height.toInt(), arcWidth, arcHeight)
            }
        }
        restorePaint()
        return this
    }

    override fun drawDRRect(
        outer: RRect,
        inner: RRect,
        paint: Paint
    ): Canvas {
        if (inner.left < outer.left || inner.top < outer.top ||
            inner.right > outer.right || inner.bottom > outer.bottom) {
            throw IllegalArgumentException("Inner RRect must be completely inside the Outer RRect.")
        }

        val outerShape = java.awt.geom.RoundRectangle2D.Float(
            outer.left,
            outer.top,
            outer.right - outer.left,
            outer.bottom - outer.top,
            outer.radii[0] * 2,
            outer.radii[1] * 2
        )

        val innerShape = java.awt.geom.RoundRectangle2D.Float(
            inner.left,
            inner.top,
            inner.right - inner.left,
            inner.bottom - inner.top,
            inner.radii[0] * 2,
            inner.radii[1] * 2
        )

        val combinedShape = java.awt.geom.Area(outerShape)
        combinedShape.subtract(java.awt.geom.Area(innerShape))

        applyPaintToGraphics(paint)

        when (paint.mode) {
            PaintMode.FILL -> {
                g.fill(combinedShape)
            }
            PaintMode.STROKE -> {
                g.draw(combinedShape)
            }
            PaintMode.STROKE_AND_FILL -> {
                g.fill(combinedShape)
                g.draw(combinedShape)
            }
        }

        restorePaint()
        return this
    }

    override fun drawRectShadow(
        r: Rect,
        dx: Float,
        dy: Float,
        blur: Float,
        spread: Float,
        color: Int
    ): Canvas {
        throw UnsupportedOperationException()
    }

    override fun drawRectShadowNoclip(
        r: Rect,
        dx: Float,
        dy: Float,
        blur: Float,
        spread: Float,
        color: Int
    ): Canvas {
        throw UnsupportedOperationException()
    }

    override fun drawPath(path: Path, paint: Paint): Canvas {
        applyPaintToGraphics(paint)
        val iterator = path.iterator(false)
        val shape = java.awt.geom.Path2D.Float()

        while (iterator.hasNext()) {
            val element = iterator.next()
            if (element == null) {
                break
            }
            when (element.verb) {
                PathVerb.MOVE -> element.p0?.let { shape.moveTo(it.x, it.y) }
                PathVerb.LINE -> element.p0?.let { shape.lineTo(it.x, it.y) }
                PathVerb.QUAD -> {
                    element.p0!!
                    element.p1!!
                    shape.quadTo(
                        element.p0.x,
                        element.p0.y,
                        element.p1.x,
                        element.p1.y
                    )
                }
                PathVerb.CUBIC -> {
                    element.p0!!
                    element.p1!!
                    element.p2!!
                    shape.curveTo(
                        element.p0.x,
                        element.p0.y,
                        element.p1.x,
                        element.p1.y,
                        element.p2.x,
                        element.p2.y
                    )
                }
                PathVerb.CONIC -> {
                    throw UnsupportedOperationException("Conic path verb is not supported")
                }
                PathVerb.CLOSE -> shape.closePath()
                else -> {}
            }
        }
        g.draw(shape)
        restorePaint()
        return this
    }

    override fun drawImage(image: Image, left: Float, top: Float): Canvas {
        val bitmap = Bitmap()
        image.readPixels(bitmap)
        val awtImage = bitmap.toBufferedImage()
        g.drawImage(awtImage, left.toInt(), top.toInt(), null)
        return this
    }

    override fun drawImage(
        image: Image,
        left: Float,
        top: Float,
        paint: Paint?
    ): Canvas {
        applyPaintToGraphics(paint!!)
        drawImage(image, left, top)
        restorePaint()
        return this
    }

    override fun drawImageRect(
        image: Image,
        dst: Rect
    ): Canvas {
        val bitmap = Bitmap()
        image.readPixels(bitmap)
        val awtImage = bitmap.toBufferedImage()
        g.drawImage(awtImage, dst.left.toInt(), dst.top.toInt(), dst.width.toInt(), dst.height.toInt(), null)
        return this
    }

    override fun drawImageRect(
        image: Image,
        dst: Rect,
        paint: Paint?
    ): Canvas {
        applyPaintToGraphics(paint!!)
        drawImageRect(image, dst)
        restorePaint()
        return this
    }

    override fun drawImageRect(
        image: Image,
        src: Rect,
        dst: Rect,
        samplingMode: SamplingMode,
        paint: Paint?,
        strict: Boolean
    ): Canvas {
        if (paint != null) applyPaintToGraphics(paint)
        val bitmap = Bitmap()
        image.readPixels(bitmap)
        val awtImage = bitmap.toBufferedImage()
        g.drawImage(awtImage,
            src.left.toInt(), src.top.toInt(),
            src.right.toInt(), src.bottom.toInt(),
            dst.left.toInt(), dst.top.toInt(),
            dst.right.toInt(), dst.bottom.toInt(),
            null)

        if (paint != null) restorePaint()

        return this
    }

    override fun drawImageNine(
        image: Image,
        center: IRect,
        dst: Rect,
        filterMode: FilterMode,
        paint: Paint?
    ): Canvas {
        throw UnsupportedOperationException()
    }

    override fun drawRegion(r: Region, paint: Paint): Canvas {
        return super.drawRegion(r, paint)
    }

    override fun drawString(s: String, x: Float, y: Float, font: Font?, paint: Paint): Canvas {
        applyPaintToGraphics(paint)
        g.drawString(s, x, y)
        restorePaint()
        return this
    }

    override fun drawTextBlob(
        blob: TextBlob,
        x: Float,
        y: Float,
        paint: Paint
    ): Canvas {
//        val glyphCount = blob.glyphsLength
//
//        val glyphIds = blob.glyphs
//        val positions = blob.positions
//
//        for (i in 0 until glyphCount) {
//            val glyphId = glyphIds[i]
//            val glyphX = x + positions[i * 2]
//            val glyphY = y + positions[i * 2 + 1]
//
//            // Render the glyph
//            drawGlyph(this, blob.font, glyphId, glyphX, glyphY, paint)
//        }

        return this
    }



//    override fun drawTextLine(
//        line: TextLine,
//        x: Float,
//        y: Float,
//        paint: Paint
//    ): Canvas {
//        applyPaintToGraphics(paint)
//        val text = line.glyphs[0]
//        g.drawString(text, x, y)
//        restorePaint()
//        return this
//    }

    override fun drawPicture(
        picture: Picture,
        matrix: Matrix33?,
        paint: Paint?
    ): Canvas {
        throw UnsupportedOperationException()
    }

    override fun drawTriangles(
        positions: Array<Point>,
        colors: IntArray?,
        texCoords: Array<Point>?,
        indices: ShortArray?,
        blendMode: BlendMode,
        paint: Paint
    ): Canvas {
        throw UnsupportedOperationException()
    }

    override fun drawTriangleStrip(
        positions: Array<Point>,
        colors: IntArray?,
        texCoords: Array<Point>?,
        indices: ShortArray?,
        blendMode: BlendMode,
        paint: Paint
    ): Canvas {
        throw UnsupportedOperationException()
    }

    override fun drawTriangleFan(
        positions: Array<Point>,
        colors: IntArray?,
        texCoords: Array<Point>?,
        indices: ShortArray?,
        blendMode: BlendMode,
        paint: Paint
    ): Canvas {
        throw UnsupportedOperationException()
    }

    override fun drawVertices(
        vertexMode: VertexMode,
        positions: FloatArray,
        colors: IntArray?,
        texCoords: FloatArray?,
        indices: ShortArray?,
        blendMode: BlendMode,
        paint: Paint
    ): Canvas {
        throw UnsupportedOperationException()
    }

    override fun drawPatch(
        cubics: Array<Point>,
        colors: IntArray,
        texCoords: Array<Point>?,
        blendMode: BlendMode,
        paint: Paint
    ): Canvas {
        throw UnsupportedOperationException()
    }

    override fun drawDrawable(
        drawable: Drawable,
        matrix: Matrix33?
    ): Canvas {
        throw UnsupportedOperationException()
    }

    override fun clear(color: Int): Canvas {
        g.color = skiaColorToAwtColor(color)
        g.fillRect(0, 0, g.clipBounds.width, g.clipBounds.height)
        return this
    }

    override fun drawPaint(paint: Paint): Canvas {
        throw UnsupportedOperationException()
    }

    override fun setMatrix(matrix: Matrix33): Canvas {
        throw UnsupportedOperationException()
    }

    override fun resetMatrix(): Canvas {
        throw UnsupportedOperationException()
    }

//    override val localToDevice: Matrix44 = TODO()

    override fun clipRect(
        r: Rect,
        mode: ClipMode,
        antiAlias: Boolean
    ): Canvas {
        throw UnsupportedOperationException()
    }

    override fun clipRRect(
        r: RRect,
        mode: ClipMode,
        antiAlias: Boolean
    ): Canvas {
        throw UnsupportedOperationException()
    }

    override fun clipPath(
        p: Path,
        mode: ClipMode,
        antiAlias: Boolean
    ): Canvas {
        throw UnsupportedOperationException()
    }

    override fun clipRegion(
        r: Region,
        mode: ClipMode
    ): Canvas {
        throw UnsupportedOperationException()
    }

    override fun translate(dx: Float, dy: Float): Canvas {
        g.translate(dx.toDouble(), dy.toDouble())
        return this
    }

    override fun scale(sx: Float, sy: Float): Canvas {
        g.scale(sx.toDouble(), sy.toDouble())
        return this
    }

    override fun rotate(deg: Float): Canvas {
        g.rotate(Math.toRadians(deg.toDouble()))
        return this
    }

    override fun rotate(deg: Float, x: Float, y: Float): Canvas {
        throw UnsupportedOperationException()
    }

    override fun skew(sx: Float, sy: Float): Canvas {
        throw UnsupportedOperationException()
    }

    override fun concat(matrix: Matrix33): Canvas {
        throw UnsupportedOperationException()
    }

    override fun concat(matrix: Matrix44): Canvas {
        throw UnsupportedOperationException()
    }

    override fun readPixels(bitmap: Bitmap, srcX: Int, srcY: Int): Boolean {
        throw UnsupportedOperationException()
    }

    override fun writePixels(bitmap: Bitmap, x: Int, y: Int): Boolean {
        throw UnsupportedOperationException()
    }

    override fun save(): Int {
        throw UnsupportedOperationException()
    }

    override fun saveLayer(
        left: Float,
        top: Float,
        right: Float,
        bottom: Float,
        paint: Paint?
    ): Int {
        throw UnsupportedOperationException()
    }

    override fun saveLayer(bounds: Rect?, paint: Paint?): Int {
        throw UnsupportedOperationException()
    }

    override fun saveLayer(layerRec: SaveLayerRec): Int {
        throw UnsupportedOperationException()
    }

//    override val saveCount: Int

    override fun restore(): Canvas {
        throw UnsupportedOperationException()
    }

    override fun restoreToCount(saveCount: Int): Canvas {
        throw UnsupportedOperationException()
    }
}