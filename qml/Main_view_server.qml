import QtQuick 2.0
import QtQuick.Layouts
import QtQuick.Controls
import binter

ColumnLayout
{

    id:mv
    required property StackView stack
    spacing:5
    Rectangle
    {
        id:head
        color:"#0f171e"

        Layout.preferredHeight: 100
        Layout.maximumHeight: 200
        Layout.minimumHeight: 75
        Layout.fillHeight:  true
        Layout.minimumWidth: 360
        Layout.maximumWidth: 600
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignCenter

        CButton
        {
            anchors.left:head.left
            anchors.verticalCenter: head.verticalCenter
            text: "Open box"
            height:50
            onClicked:
            {

                var component = Qt.createComponent("Enter_Pin_server.qml");
                if (component.status === Component.Ready) {
                    var next = component.createObject(mv, {stack:mv.stack});
                    if (next === null) {
                        // Error Handling
                        console.log("Error creating object");
                    }
                } else if (component.status === Component.Error) {
                    // Error Handling
                    console.log("Error loading component:", component.errorString());
                }

                mv.stack.push(next)
            }
        }
        Text
        {
            id:labelidserver

            anchors.bottom:  head.bottom
            anchors.horizontalCenter: head.horizontalCenter
            text: "<b>Server id:</b>"+ Book_Server.publish_addr
            width: parent.width-30
            color:"white"
            elide: Text.ElideRight
            ToolTip
            {
                id:tooltip
                visible: false
                text:qsTr("Copy")
            }
            TextEdit{
                id: textEdit
                visible: false
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled :true
                onEntered: tooltip.visible=!tooltip.visible
                onExited: tooltip.visible=!tooltip.visible
                onClicked:
                {
                    textEdit.text = Book_Server.publish_addr
                    textEdit.selectAll()
                    textEdit.copy()
                }
            }
            font.pointSize: 20
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

        }



    }
    Init_popup_server
    {
        anchors.centerIn: Overlay.overlay
        visible:!Book_Server.publishing
        width: 360
        height: 460
        closePolicy: Popup.NoAutoClose

    }

    Rectangle
    {
        id:bottom_
        color:"#0f171e"

        Layout.fillHeight:  true
        Layout.minimumHeight: 400
        Layout.minimumWidth: 360
        Layout.maximumWidth: 600
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignCenter

	Day_model{
        	id:day_model
            book_server:Book_Server
        }
        Day_swipe_view{
            clip:true
            anchors.fill: bottom_
            can_book:false
            day_model: day_model
        }


    }




}
