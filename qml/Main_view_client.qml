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
        Layout.minimumHeight: 50
        Layout.fillHeight:  true
        Layout.minimumWidth: 400
        Layout.maximumWidth: 600
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignCenter

        CButton
        {
            anchors.left:head.left
            anchors.verticalCenter: head.verticalCenter
            text: qsTr("Book the box")
            height:50
            enabled: day_model.total_selected&&Book_Client.conected
            onClicked:
            {

                var component = Qt.createComponent("Enter_Pin_client.qml");
                if (component.status === Component.Ready) {
                    var next = component.createObject(mv, {stack:mv.stack,day_model:day_model});
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

    }
    Init_popup_client
    {
        anchors.centerIn: Overlay.overlay
        visible:!Book_Client.conected
        width: 400
        height: 600
        closePolicy: Popup.NoAutoClose

    }


    Rectangle
    {
        id:bottom_
        color:"#0f171e"

        Layout.fillHeight:  true
        Layout.minimumHeight: 400
        Layout.minimumWidth: 400
        Layout.maximumWidth: 600
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignCenter


	Day_model{
        	id:day_model
            book_client:Book_Client
        }
        Day_swipe_view{
            clip:true
            anchors.fill: bottom_
            can_book:true
            day_model: day_model
        }


    }




}
