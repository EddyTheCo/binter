import QtQuick 2.0
import QtQuick.Controls
import QtQuick.Layouts
import binter
ColumnLayout
{

    id:dia_delegate
    spacing:hours_container.delegate_height
    required property  date day
    required property  bool can_book
    required property  Hour_model hour_model

    Rectangle
    {

        id:day_label
        Layout.preferredHeight: 100
        Layout.minimumHeight:  50
        Layout.fillWidth: true

        Layout.alignment: Qt.AlignTop
        color: '#0f79af'
        ColumnLayout
        {

            anchors.fill: day_label
            Text
            {

                text: dia_delegate.day.toLocaleString(Qt.locale(),"ddd")
                color:"white"
                font.pointSize:35
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignCenter
                horizontalAlignment: Text.AlignHCenter
            }
            Text
            {
                text: dia_delegate.day.toLocaleString(Qt.locale(),"dd/MM/yy")
                font.pointSize:25
                color:"white"
                Layout.minimumHeight:  25
                Layout.minimumWidth:  25
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignCenter
                horizontalAlignment: Text.AlignHCenter
            }

        }






    }
    Horario_list_view
    {
        id:hours_container

        Layout.fillHeight: true
        Layout.preferredHeight: 500
        Layout.minimumHeight:  400
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignBottom

        delegate_height:100
        can_book:dia_delegate.can_book
        horario_model: dia_delegate.hour_model

    }






}


