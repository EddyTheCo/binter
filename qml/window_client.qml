import QtQuick.Controls
import binter

ApplicationWindow {
    visible: true


    StackView {
        id: stack
        initialItem: root
        anchors.fill: parent
    }
    Main_view_client
    {
        id:root
        anchors.fill: parent
        stack:stack
    }

}

