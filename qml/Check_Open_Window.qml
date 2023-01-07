import QtQuick 2.0
import QtQuick.Layouts
import QtQuick.Controls



ColumnLayout
{
    id:cow
    required property string message
    required property  StackView stack
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
            text: "Back"
            onClicked: cow.stack.pop()
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

        Text
        {
            text:cow.message
            width:parent.width*0.9
            wrapMode: TextEdit.Wrap
            font.pointSize:20
            anchors.centerIn: center_
            color:"white"
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
            text: "finish"
            onClicked: cow.stack.pop(null)
        }
    }

}
