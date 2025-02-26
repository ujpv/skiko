package org.jetbrains.skiko.swing

class Profiler(val name: String = "Unknown") {
    var firstFrameTimeNs: Long = 0L
    var lastFrameTimeNs: Long = 0L
    var countFrames: Long = 0

    data class TimeData(var totalTimeNs: Long, var currentFrameBeginTimeNs: Long)
    val timeData = mutableMapOf<String, TimeData>()

    var lastTimeReportNs: Long = 0L

    fun onFrameBegin() {
        val nowNs = System.nanoTime()
        if (firstFrameTimeNs == 0L) {
            firstFrameTimeNs = nowNs
        }
        onBegin("Frame total")
    }

    fun onFrameEnd() {
        val nanoTime = System.nanoTime()
        onEnd("Frame total")
        lastFrameTimeNs = nanoTime
        countFrames++
    }

    fun onBegin(name: String) {
        timeData.getOrPut(name) {
            TimeData(0L, 0L)
        }.currentFrameBeginTimeNs = System.nanoTime()
    }

    fun onEnd(name: String) {
        timeData[name]?.let {
            it.totalTimeNs += System.nanoTime() - it.currentFrameBeginTimeNs
        }
    }


    fun getFSP(): Long {
        val timeSec = (lastFrameTimeNs - firstFrameTimeNs) / 1_000_000_000
        return countFrames / timeSec
    }

    fun printEvery10Sec() {
        if (System.nanoTime() - lastTimeReportNs > 10_000_000_000 && countFrames > 0 && lastFrameTimeNs - firstFrameTimeNs > 1_000_000_000) {
            lastTimeReportNs = System.nanoTime()
            println("Profiler: $name")
            println("FPS: ${getFSP()}")
            for ((key, value) in timeData) {
                println("Average $key time: ${value.totalTimeNs.toDouble() / 1_000_000 / countFrames.toDouble()} ms")
            }
        }
    }
}