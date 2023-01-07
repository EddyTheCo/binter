import QtQuick 2.0
import QtQuick.Layouts
import QtQuick.Controls
import binter

ColumnLayout
{
    id: epc
    required property  StackView stack
    required property Day_model day_model

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

            height:50
            text: "Back"
            onClicked: epc.stack.pop()
        }


    }

    Rectangle
    {
        id:center_
        color:"#0f171e"
        Layout.minimumWidth: 400
        Layout.maximumWidth: 600
        Layout.preferredHeight: 300
        Layout.minimumHeight: 200
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.alignment: Qt.AlignCenter
        Pin_box
        {
            anchors.centerIn: center_
            width:center_.width*0.9
            height:200
            id:pin_box_
            description:"Set pin"
        }
        Pay_popup
        {
            id:pay_pup
            anchors.centerIn: Overlay.overlay
            visible:false
            width: 400
            height: 600
            closePolicy: Popup.CloseOnPressOutside
        }

    }

    Rectangle
    {
        id:tail
        color:"#0f171e"
        Layout.minimumWidth: 400
        Layout.maximumWidth: 600
        Layout.preferredHeight: 100
        Layout.maximumHeight: 200
        Layout.minimumHeight: 50
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignCenter
        CButton
        {

            anchors.centerIn: parent
            text: "Book"
            onClicked:
            {
                Book_Client.code_str=pin_box_.pin
                day_model.get_new_bookings()
            }
        }
        Connections {
            target: Book_Client
            function onStatusChanged() {
                if(Book_Client.status===Book_Client.Ready)
                {
                    day_model.get_new_bookings()
                    if(!Book_Client.topay)epc.stack.pop(null)
                }
                if(Book_Client.status===Book_Client.Null)
                {
                    pay_pup.visible=true;
                }

            }
        }
        Connections {
            target: Book_Client
            function onTopayChanged() {
                if(!Book_Client.topay)
                {
                    pay_pup.visible=false;
                    epc.stack.pop(null)
                }

            }
        }



    }

}
