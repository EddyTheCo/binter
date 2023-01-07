
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
            role_:"Client"
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
                text: "Set the server id to connect"
                color:"white"
                font.pointSize: 15 
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            TextEdit{
                id: textEdit_
                visible: true
                width:notice_.width
                anchors.top: notice_.bottom
                anchors.horizontalCenter: notice_.horizontalCenter
                color:"white"
                text:"rms1..."
                horizontalAlignment: Text.AlignHCenter
                clip:true
                onTextChanged: {
                    Book_Client.server_id=textEdit_.text
                }
                 focus: true
                 font.family: "Helvetica"
                 font.pointSize: 20
            }


        }
    }



}
