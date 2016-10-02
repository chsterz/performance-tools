import QtQuick 2.0
import QtQuick.Window 2.0
import Memory 1.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Window {
    visible: true

    height: 720
    width: 1280

    property int numberOfBuckets: 50
    property int bucketSize: 10
    property var measurements: []

    Component.onCompleted: {
        for (var i = 0; i < numberOfBuckets; i++)
            measurements.push(0);
    }

    Rectangle{
        id: background

        anchors.fill: parent
        color: "lightgray"
    }

    RowLayout{
        id: controls

        anchors.horizontalCenter: background.horizontalCenter
        anchors.top: background.top
        anchors.topMargin: 5
        spacing: 5

        Text{
            text: "pid:"
        }

        TextField{
            id: pidInput

            text: "1609"
        }

        Button{
            id: goButton

            text: "Measure"
            onClicked: {
                Shell.setPid(Number(pidInput.text))
                poll.stop();
                poll.currentLatency = 10;
                poll.repeat = true;
                poll.start();
            }
        }

        Timer {
            id: poll

            interval: 1
            repeat: true

            property int currentLatency: 1
            property int bucket: currentLatency/bucketSize
            property int lastMeasurement: 0
            property int currentMeasurement: 0

            onTriggered: {
                measurements[bucket] = Shell.lookupDataForBucket(currentLatency)
                bars.updateValues(bucket);
                currentLatency = currentLatency + bucketSize;
                if (currentLatency > (numberOfBuckets-1)*bucketSize ){
                    currentLatency = 10;
                    lastMeasurement = 0;
                }
            }
        }

    }

    Rectangle{
        id: chart

        width: background.width * 0.9
        height: background.height * 0.9
        anchors.horizontalCenter: background.horizontalCenter
        anchors.bottom: background.bottom

        Rectangle{
            id: bottomline

            width: 0.9 * parent.width
            height: 1
            color: "gray"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 50
        }
        Rectangle{
            id: chartArea

            width: 0.9 * parent.width
            color: "#dddddd"

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: bottomline.top
            height: 0.8 * parent.height

            ColumnLayout{
                ExclusiveGroup{id: costOptionGroup}
                RadioButton {
                    id: optionEventNumbers

                    text: "#Events"
                    checked: true
                    exclusiveGroup: costOptionGroup
                }
                RadioButton {
                    id: optionCycleNumbers

                    text: "#Cycles spent"
                    exclusiveGroup: costOptionGroup
                }
            }

            Rectangle{
                width: parent.width
                y: parent.height*0.2
                height: 2
                color: "gray"
            }

            Repeater {
                id: bars
                model: numberOfBuckets

                property real maxValue: 100000
                property real scaleFactor: parent.height*0.8/maxValue

                delegate: Rectangle{

                    x: index * (chartArea.width/numberOfBuckets)
                    width: (chartArea.width/numberOfBuckets)
                    height: 1
                    anchors.bottom: parent.bottom
                    color: "#f57900"
                    Text {
                        id: label
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: index*bucketSize + "-" + (index+1) * bucketSize
                        font.pixelSize: 12
                        rotation: 90
                    }

                }

                function updateValues(i) {
                        var value;
                        if (i === 0) return;
                        if (optionEventNumbers.checked) value = measurements[i];
                        if (optionCycleNumbers.checked) value = measurements[i] * (i*bucketSize) - bucketSize/2;
                        if (Math.abs(value) > maxValue) maxValue = Math.abs(value*1.2);
                        if (value > 0) {
                            itemAt(i).color = "#f57900";
                            itemAt(i).height = value * scaleFactor;
                        }
                        else
                        {
                            itemAt(i).color = "gray";
                            itemAt(i).height = -value * scaleFactor;
                        }
                }
            }
        }

    }


}

