
import QtQuick 2.0
import QtQuick.Controls
import QtQuick.Layouts


Popup {
    id: popup
    required property date start
    required property date finish
    required property string code_str

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
                id:from_
                anchors.top: rect_.top
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("<b>From: </b>")  + popup.start.toLocaleTimeString("h:mm AP")
                color:"white"
                font.pointSize: 15
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            Text
            {
                id:to_
                anchors.top: from_.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                text: "<b>To: </b>" + popup.finish.toLocaleTimeString("h:mm AP")
                color:"white"
                font.pointSize: 15
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            Text
            {
                id:code_
                anchors.top: to_.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                text: "<b>Code: </b>" + popup.code_str
                color:"white"
                width:rect_.width*0.9
                elide: Text.ElideRight
                font.pointSize: 15
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            Address_qr
            {
                color:'lightyellow'
                anchors.top: code_.bottom
                anchors.horizontalCenter: code_.horizontalCenter
                width:rect_.width*0.9
                height:width
                addr_:popup.code_str
                url_:""
            }


        }
    }



}
