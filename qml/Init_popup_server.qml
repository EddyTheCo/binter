
import QtQuick 2.0
import QtQuick.Controls
import QtQuick.Layouts
import binter

Popup {
    id: popup

    modal: true
    focus: true
    background:Rectangle
    {
        color:"#0f171e"
        border.width: 2
        border.color: "white"
        radius:8
    }
    ColumnLayout
    {
        anchors.fill: parent

        Logo
        {
            Layout.preferredHeight: 150
            Layout.maximumHeight: 200
            Layout.minimumHeight: 50 
            Layout.preferredWidth: 360 
            Layout.minimumWidth: 300
            Layout.maximumWidth: 400 
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            role_:"Server"
        }

        Rectangle
        {
            color:"transparent"
            id:rect_
            Layout.minimumHeight: 100
            Layout.preferredWidth: 360 
            Layout.minimumWidth: 300
            Layout.maximumWidth: 400
            Layout.fillHeight:  true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom

            Text
            {
                id:notice_
                anchors.top: rect_.top
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Transfer at least "+ Book_Server.transfer_funds +" to:"
                color:"white"
                font.pointSize: 15 
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            Address_qr
            {
                color:'lightyellow'
                anchors.top: notice_.bottom
                anchors.horizontalCenter: notice_.horizontalCenter
                width:rect_.width*0.7
                height:width
                addr_:Book_Server.publish_addr
                url_:"https://firefly.iota.org/"
            }


        }
    }



}
