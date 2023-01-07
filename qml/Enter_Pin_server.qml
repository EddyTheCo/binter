import binter
import QtQuick 2.0
import QtQuick.Layouts
import QtQuick.Controls



ColumnLayout
{
    id: ep
    required property  StackView stack

    spacing:2
    Rectangle
    {
        id:head
        color:"#0f171e"

        Layout.preferredHeight: 100
        Layout.maximumHeight: 200
        Layout.minimumHeight: 75
        Layout.fillHeight:  true
        Layout.minimumWidth: 360
        Layout.maximumWidth: 600
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignCenter

        CButton
        {
            anchors.left:head.left
            anchors.verticalCenter: head.verticalCenter

            height:50
            text: "Back"
            onClicked: ep.stack.pop()
        }


    }

    Rectangle
    {
        id:center_
        color:"#0f171e"
        Layout.fillHeight:  true
        Layout.minimumHeight: 400
        Layout.minimumWidth: 360
        Layout.maximumWidth: 600
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignCenter

        Pin_box
        {
            width:center_.width*0.7
            height:width*0.6
            id:pin_box_
            anchors.centerIn: center_
            description:"Enter pin"
        }

    }

    Rectangle
    {
        id:tail
        color:"#0f171e"
        Layout.minimumWidth: 360
        Layout.maximumWidth: 600
        Layout.preferredHeight: 100
        Layout.maximumHeight: 200
        Layout.minimumHeight: 50
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignCenter
        CButton
        {
            anchors.centerIn: parent
            text: "Check"
            onClicked:
            {
                var component;
                var next
                if(Book_Server.try_to_open(pin_box_.pin))
                {
                    component = Qt.createComponent("Check_Open_Window.qml");
                    if (component.status === Component.Ready) {
                        next = component.createObject(ep.stack, {
                                                          message:"The box is opened",
                                                          stack:ep.stack
                                                      });
                        if (next === null) {
                            // Error Handling
                            console.log("Error creating object");
                        }
                    } else if (component.status === Component.Error) {
                        // Error Handling
                        console.log("Error loading component:", component.errorString());
                    }

                    ep.stack.push(next)
                }
                else
                {
                    component = Qt.createComponent("Check_Open_Window.qml");
                    if (component.status === Component.Ready) {
                        next = component.createObject(ep.stack, {
                                                              message:"Ooops you can not open the box",
                                                              stack:ep.stack
                                                          });

                        if (next === null) {
                            // Error Handling
                            console.log("Error creating object");
                        }
                    } else if (component.status === Component.Error) {
                        // Error Handling
                        console.log("Error loading component:", component.errorString());
                    }

                    ep.stack.push(next)
                }
            }
        }
    }

}
