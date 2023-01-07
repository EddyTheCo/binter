
import QtQuick


Rectangle
{
    id:logoroot
    required property string role_
    color:"transparent"
    Rectangle
    {
        anchors.centerIn: parent
        FontLoader { id: webFont; source: "https://github.com/google/fonts/raw/a4f3deeca2d7547351ff746f7bf3b51f5528dbcf/ofl/indieflower/IndieFlower-Regular.ttf" }
        Text
        {
            id:text_
            anchors.centerIn: parent
            font.family: webFont.font
            font.pixelSize: 60
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text : "DLockers"
            color:"white"
        }
        Text
        {
            id:serv
            anchors.top:text_.bottom
            anchors.horizontalCenter: text_.horizontalCenter
            font.pixelSize: 30
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text : logoroot.role_
            color:"white"
        }

    }



}

