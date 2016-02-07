import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import Atomify 1.0
import QtCharts 2.0
import "qrc:/mobile/style"
import "qrc:/mobile/dashboard"

QtObject {
    id: chartScroller
    
    property real timeRange: 3
    
    property real value
    property real time
    
    property real lowPassValue: 0.0

    property real yMin: 0
    property real yMax: 0

    property ValueAxis xAxis
    property ValueAxis yAxis
    property LineSeries lineSeries
    
    onValueChanged: {
        if(isNaN(value)) {
            return;
        }
        var lowPassFactor = 0.99
        // lowPassFilter smooths over a few tenth's of the time range
        if(lineSeries.count > 1) {
            lowPassFactor = 1.0 - 10 * (lineSeries.at(1).x - lineSeries.at(0).x) / timeRange
        }
        lowPassValue = lowPassFactor * lowPassValue + (1 - lowPassFactor) * value
        lineSeries.append(time, lowPassValue)
        
        var firstPoint = lineSeries.at(0)
        var lastPoint = lineSeries.at(lineSeries.count - 1)
        if(lastPoint.x - firstPoint.x > timeRange) {
            lineSeries.remove(firstPoint.x, firstPoint.y)
        }
        var minMaxFactor = 0.0
        // influence the vertical min/max over a few time ranges
        if(lineSeries.count > 1) {
            minMaxFactor = 0.1 * (lineSeries.at(1).x - lineSeries.at(0).x) / timeRange
        }
        yMax = minMaxFactor * lowPassValue + (1 - minMaxFactor) * Math.max(yMax, lowPassValue)
        yMin = minMaxFactor * lowPassValue + (1 - minMaxFactor) * Math.min(yMin, lowPassValue)
        xAxis.min = time - timeRange
        xAxis.max = time
        yAxis.min = yMin*0.9
        yAxis.max = yMax*1.1
    }
}
