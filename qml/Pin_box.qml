import QtQuick 2.0

Rectangle
{
    id:pinrect
    property  string pin:numbers_.text
    required property string description
    color:"transparent"
    border.width: 2
    border.color: "white"
    radius:8
    Text
    {
        id:pintext
        font.pointSize:20
        color:"white"
        text:qsTr(pinrect.description)
        anchors.bottom:numbers_.top
        anchors.horizontalCenter: numbers_.horizontalCenter
    }
    TextInput {
        id:numbers_
        anchors.centerIn: parent
        horizontalAlignment: TextEdit.AlignHCenter
        font.letterSpacing :20
        font.pointSize: 25
        color:"white"
        inputMask: "99999"
        text: "12345"

    }

}
