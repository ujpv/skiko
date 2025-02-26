package org.jetbrains.skiko.swing

class Profiler {
    var firstFrameTimeNs: Long = 0L
    var lastFrameTimeNs: Long = 0L
    var countFrames: Int = 0

    var currentFrameBeginTimeNs: Long = 0L
    var currentRenderBeginTimeNs: Long = 0L
    var currentDrawBeginTimeNs: Long = 0L

    var totalFrameTimeNs: Long = 0L
    var totalRenderTimeNs: Long = 0L
    var totalDrawTimeNs: Long = 0L

    var lastTimeReportNs: Long = 0L

    fun onFrameBegin() {
        val nowNs = System.nanoTime()
        if (firstFrameTimeNs == 0L) {
            firstFrameTimeNs = nowNs
        }

        currentFrameBeginTimeNs = nowNs
    }

    fun onFrameEnd() {
        val nanoTime = System.nanoTime()
        totalFrameTimeNs += nanoTime - currentFrameBeginTimeNs
        lastFrameTimeNs = nanoTime
        countFrames++
    }

    fun onRenderBegin() {
        currentRenderBeginTimeNs = System.nanoTime()
    }

    fun onRenderEnd() {
        totalRenderTimeNs += System.nanoTime() - currentRenderBeginTimeNs
    }

    fun onDrawBegin() {
        currentDrawBeginTimeNs = System.nanoTime()
    }

    fun onDrawEnd() {
        totalDrawTimeNs += System.nanoTime() - currentDrawBeginTimeNs
    }

    fun getFSP(): Long {
        val timeSec = (lastFrameTimeNs - firstFrameTimeNs) / 1_000_000_000
        return countFrames / timeSec
    }

    fun printEvery10Sec() {
        if (System.nanoTime() - lastTimeReportNs > 10_000_000_000 && countFrames > 0 && lastFrameTimeNs - firstFrameTimeNs > 1_000_000_000) {
            lastTimeReportNs = System.nanoTime()
            println("FPS: ${getFSP()}")
            println("Average render time: ${totalRenderTimeNs.toDouble() / 1_000_000 / countFrames.toDouble()} ms")
            println("Average draw time: ${totalDrawTimeNs.toDouble() / 1_000_000 / countFrames.toDouble()}")
            println("Average frame time: ${totalFrameTimeNs / 1_000_000 / countFrames.toDouble()} ms")
        }
    }
}