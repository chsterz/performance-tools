import QtQuick 2.3
import QtQuick.Controls 1.2


Row{
    id: stopButton
    visible: false;
    spacing: 4
    Rectangle {
        id: redRect;
        width: 15;
        height: 15;
        color: "red";
        radius:2;
        anchors.verticalCenter: parent.verticalCenter;
		MouseArea {	anchors.fill:redRect; onClicked: {chart.stopMeasurement(); stopButton.visible = false;}}
    }
    Label {
        id: stopLabel;
        text: "Stop";
    }
}
