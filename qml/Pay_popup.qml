
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
            Layout.preferredHeight: 200
            Layout.maximumHeight: 400
            Layout.minimumHeight: 180
            Layout.preferredWidth: 400
            Layout.minimumWidth: 300
            Layout.maximumWidth: 500
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            role_:""
        }

        Rectangle
        {
            color:"transparent"
            id:rect_
            Layout.minimumHeight: 200
            Layout.preferredWidth: 400
            Layout.minimumWidth: 300
            Layout.maximumWidth: 500
            Layout.fillHeight:  true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom

            Text
            {
                id:notice_
                anchors.top: rect_.top
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Pay " + Book_Client.topay + " to:"
                color:"white"
                font.pointSize: 20
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            Address_qr
            {
                color:'lightyellow'
                anchors.top: notice_.bottom
                anchors.horizontalCenter: notice_.horizontalCenter
                width:rect_.width*0.9
                height:width
                addr_:Book_Client.payment_addr
                url_:""
            }


        }
    }



}
