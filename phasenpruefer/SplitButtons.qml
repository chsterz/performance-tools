import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4
import QtCharts 2.0
import Phase 1.0

Row{
    id: splitButtons
    visible: false
    anchors.right: parent.right; anchors.bottom: parent.bottom;
    anchors.rightMargin: 64; anchors.bottomMargin: 9;
    spacing: 4
    Rectangle{
        color: splitActive ? "lightgrey":"#ff3465a4";
        width: 40; height: 36;
        Label{anchors.centerIn: parent; text: "\uf1fe"; font.family: "Awesome"; font.pointSize: 16}
        TooltipArea{text: "Combined Data"}
        MouseArea{
            anchors.fill: parent;
            onClicked:{
                if(splitActive){
                    split1.clear(); split1withArea.visible=false;
                    split2.clear(); split2withArea.visible=false;
                    splitActive = false;
                    color = "lightgrey";
                }
            }
        }
    }
    Rectangle{
        color: splitActive ? "#fff8941f" : "lightgrey";
        width: 40; height: 36;
        Rectangle{ width: 2; height: 28; anchors.centerIn: parent; color:"#373d3e";}
        Label{ anchors.centerIn: parent; text: "\uf1fe"; font.family: "Awesome"; font.pointSize: 16}
        TooltipArea{text: "Split Data"}
        MouseArea{
            anchors.fill: parent;
            onClicked:{
                if(!splitActive){
                    var splitIndex = Shell.split(chart.getData());
                    split1.clear();split1withArea.visible=true;
                    split2.clear();split2withArea.visible=true;
                    for( var i = 0; i < series1.count; i++){
                        if(i < splitIndex)
                            split1.append(series1.at(i).x,series1.at(i).y);
                        else
                            split2.append(series1.at(i).x,series1.at(i).y);
                    }
                    splitActive = true;
                    color = "lightgrey";
                }
            }
        }
    }
}
