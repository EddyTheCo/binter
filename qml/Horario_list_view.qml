import QtQuick 2.0
import QtQuick.Controls
import binter

ListView {
    id:horario
    required property int delegate_height
    required property bool can_book
    required property Hour_model horario_model

    Book_popup
    {
        id:book_pup_
        anchors.centerIn: Overlay.overlay
        visible:false
        width: 350
        height: 600
        closePolicy: Popup.CloseOnPressOutside
        start:new Date()
        finish: new Date()
        code_str:""
    }

    model: horario_model
    delegate: Horario_delegate {
        height:horario.delegate_height
        width:horario.width
        can_book:horario.can_book
        book_pup:book_pup_
    }


}
