import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4
import QtCharts 2.0
import Phase 1.0

ApplicationWindow {
	id: mainWindow
	visible: true
	width: 640
	height: 480
	title: "Phasenprüfer"

///////////////////////////////////////////////////////////////////////////////////////////////////

	menuBar: MenuBar {
		Menu {
			title: "\&Process"

			MenuItem{
				text: "Measure \&New Process"
				shortcut: "Ctrl+N"
				onTriggered: {newDialog.visible = true;}
			}
			MenuItem{
				text: "Attach to Running Process"
			}
		}
		Menu {
			title: "\&Help"
			MenuItem{
				text: "\&About"
				onTriggered:{aboutDialog.visible = true;}
			}
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Step 0 ASK User to Choose new
	Label{
		anchors.centerIn: parent
		visible: !chart.visible
		text: "Start new Process (Ctrl+N)";
		font.family: "Source Sans Pro"
		font.pointSize: 28
		font.bold: true
		color: "#ff555753"
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Step 2 SHOW Data

	ChartView {
		id: chart
		title: "Memory usage"
		titleFont.family: "Source Sans Pro"
		titleFont.bold: true
		titleFont.pointSize: 13
		anchors.fill: parent
		antialiasing: true
		visible: false
		property var magnitudes: [1, 2, 5, 8, 10, 20, 50, 80, 100, 200, 500, 800, 1000, 2000, 5000, 8000, 10000, 20000, 50000, 80000, 100000, 200000]

		ValueAxis {
			id: axisX
			property var magnitudeIndex: 0
			titleText: "Time"
			min: 0
			max: 20
		}

		ValueAxis {
			id: axisY
			property var magnitudeIndex: 0
			titleText: "Memory"
			labelFormat: "%'d MB"
			min: 0
			max: 1
		}

		AreaSeries {
			id: series1withArea
			name: "Resident Set Size"
			color: "#ff3465a4"
			axisX: axisX
			axisY: axisY
			onHovered:{color = (state ?  "#ff5382ba" : "#ff3465a4")}
			upperSeries: LineSeries{
				id: series1;	
			}
		}
		AreaSeries {
			id: split1withArea;
			name: "Ramp up"
			color: "#ff73d216"
			axisX: axisX
			axisY: axisY
            onClicked:{
                var tooltips = Qt.createComponent("Tooltip.qml");
                tooltips.createObject(chart, {numberText: Shell.cacheMissesSplitAt(split1.count*0.01).x, color: split1withArea.color});
                console.log(point.x, point.y);
            }

			onHovered:{color = (state ?  "#ff8ae234" : "#ff73d216")}
			upperSeries: LineSeries{
				id: split1;
			}
		}
		AreaSeries{
			id: split2withArea;
			name: "Calculation"
			color: "#fff57900"
			axisX: axisX
			axisY: axisY
            onClicked:{
                var tooltips = Qt.createComponent("Tooltip.qml");
                tooltips.createObject(chart, {numberText: Shell.cacheMissesSplitAt(split1.count*0.01).y, color: split2withArea.color});
            }
			onHovered:{color = (state ?  "#fff8941f" : "#fff57900")}
			upperSeries: LineSeries{
				id: split2;
			}
		}

		Timer{
			id: pollTimer
			running: false; interval: 10; repeat: true
			onTriggered:{
				var mem = Shell.memoryUsage/1000.0;
				if (series1.count/100.0 > axisX.max - 1 )
				{
					axisX.magnitudeIndex+=1;
					axisX.max =  chart.magnitudes[axisX.magnitudeIndex];
				}
				if (mem > axisY.max)
				{
					axisY.magnitudeIndex+=1;
					axisY.max = chart.magnitudes[axisY.magnitudeIndex];
				}
				series1.append(series1.count/100.0, mem);

				if (mem == 0)
				{
					pollTimer.stop();
					chart.autoadjust();
					stopButton.visible = false;
					splitButtons.visible = true;
					return;
				}
			}
		}

		function autoadjust()
		{
			axisX.max = series1.count/100.0 * 1.05;
		}

		function getData()
		{
			var result = [];
			for(var i=0; i<series1.count; i++)
				result.push(series1.at(i));
			return result;
		}

		function stopMeasurement()
		{
			Shell.stopMeasurement();
			pollTimer.stop();
			chart.autoadjust();
			splitButtons.visible = true;
		}

		function viewDataInRange(item, name)
		{
			var component = Qt.createComponent("DetailView.qml");
			var newWindow = component.createObject(mainWindow, {"title":name});
			newWindow.show();
		}

		function reset()
		{
			axisX.min = 0; axisX.max = 20;
			axisY.min = 0; axisY.max = 1; axisY.magnitudeIndex = 0;
			series1.clear();split1.clear();split2.clear();
			splitActive = false;
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Step 3 Split / Join
	property var splitActive: false;
	SplitButtons {
		id: splitButtons
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
	statusBar: StatusBar{
		id: statusBar
		style: StatusBarStyle{}
		StopButton {
			id: stopButton
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////

	MessageDialog {
		id: aboutDialog
		title: "About Phasenprüfer"
		icon: StandardIcon.Information
		text: "Phasenprüfer checks your program phases \n(based on Memory consumption) \n\n Christoph Sterz \n code@chsterz.de"
	}

///////////////////////////////////////////////////////////////////////////////////////////////////

	Dialog{
		id: newDialog
		standardButtons: StandardButton.Ok | StandardButton.Cancel
		width: 400
		height: 130

		ColumnLayout{
			anchors.fill: parent
			Text{id:label; text:" Command of Process to execute:"}
			TextField{
				id: commandfield
				focus: true;
                text: "qtcreator-bin"
				anchors.right: parent.right
				anchors.left: parent.left
				Component.onCompleted: {focus=true}
			}
		}
		onAccepted: {
			Shell.command = commandfield.text;
			if(! Shell.checkCommandExists()){
				processNotFoundDialog.visible = true;
			}else{
				Shell.startMeasurement();
				chart.visible = true;
				stopButton.visible = true;
				chart.reset();
				pollTimer.running = true;
			}
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////

	MessageDialog{
		id: processNotFoundDialog
		title: "Process not found";
		text: "The process '"+ Shell.command +"' could not be found. \n Please re-enter!"
		onAccepted: newDialog.visible = true;
	}
////////////////////////////////////////////////////////////////////////////////////////////////////


}
