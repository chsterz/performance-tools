import QtQuick 2.0
import QtGraphicalEffects 1.0

Rectangle{
    id: root
    color: "white"

    anchors.left: parent.left
    anchors.leftMargin: 100
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 9


    width: 100
    height: 40
    property string numberText

    Text{
        id: display
        horizontalAlignment: Text.AlignHCenter
        anchors.centerIn: parent
        text: numberText + "<br>cache misses"
    }
    Timer{id: timer; interval: 2000; onTriggered: {fadeAnimation.start()}}
    NumberAnimation on opacity{id: fadeAnimation; running:false; duration: 500; to: 0.0; onRunningChanged: {if(!running) root.destroy()}}
    Component.onCompleted: {timer.start()}

    layer.enabled: true
    layer.effect: DropShadow {
        anchors.fill: root
        horizontalOffset: 0
        verticalOffset: 0
        radius: 8.0
        samples: 17
        color: "#80000000"
    }

}
