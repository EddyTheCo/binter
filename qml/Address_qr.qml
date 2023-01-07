import QtQuick.Controls
import QtQuick


Rectangle {
    id:rect_
    required property string addr_
    required property string url_

    Image {
        id:img
        anchors.centerIn:parent
        sourceSize.width: rect_.width-10
        source: "image://qrcodeblack/"+rect_.addr_


    }
    ToolTip
    {
        id:tooltip
        visible: false
        text:qsTr("Copy address")
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
            textEdit.text = rect_.addr_
            textEdit.selectAll()
            textEdit.copy()

            if(rect_.url_)
            {
                Qt.openUrlExternally(rect_.url_)
            }

        }
    }
}
