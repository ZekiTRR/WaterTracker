import QtQuick

Item {
    id: root
    clip: true

    property color deepBlue: "#0077B6"
    property color brightBlue: "#00B4D8"
    property color foam: "#EAFBFF"

    Rectangle {
        anchors.fill: parent
        radius: 0
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#EAFBFF" }
            GradientStop { position: 0.52; color: "#D7F4FF" }
            GradientStop { position: 1.0; color: "#BDEBFF" }
        }
    }

    Rectangle {
        id: glow
        width: parent.width * 0.72
        height: parent.height * 0.72
        radius: height / 2
        x: parent.width * 0.15
        y: parent.height * 0.08
        color: "#66FFFFFF"
        opacity: 0.42

        SequentialAnimation on opacity {
            loops: Animation.Infinite
            running: root.visible
            NumberAnimation { to: 0.72; duration: 1700; easing.type: Easing.InOutSine }
            NumberAnimation { to: 0.42; duration: 1700; easing.type: Easing.InOutSine }
        }
    }

    Repeater {
        model: 8

        Rectangle {
            required property int index

            property real baseX: (root.width * ((index + 1) / 9.5)) - width / 2
            property real drift: index % 2 === 0 ? 12 : -12

            width: 8 + (index % 3) * 5
            height: width
            radius: width / 2
            x: baseX
            y: root.height + index * 14
            color: index % 2 === 0 ? "#88FFFFFF" : "#6600B4D8"
            opacity: 0.42

            SequentialAnimation on y {
                loops: Animation.Infinite
                running: root.visible
                PauseAnimation { duration: index * 190 }
                NumberAnimation {
                    from: root.height + 18
                    to: -32
                    duration: 3600 + index * 240
                    easing.type: Easing.InOutSine
                }
            }

            SequentialAnimation on x {
                loops: Animation.Infinite
                running: root.visible
                NumberAnimation {
                    from: baseX
                    to: baseX + drift
                    duration: 1300
                    easing.type: Easing.InOutSine
                }
                NumberAnimation {
                    from: baseX + drift
                    to: baseX
                    duration: 1300
                    easing.type: Easing.InOutSine
                }
            }
        }
    }

    Rectangle {
        id: waveBack
        width: parent.width * 1.28
        height: 58
        radius: 28
        x: -parent.width * 0.12
        y: parent.height - 52
        color: "#5533C4EA"

        SequentialAnimation on x {
            loops: Animation.Infinite
            running: root.visible
            NumberAnimation { to: -root.width * 0.20; duration: 1800; easing.type: Easing.InOutSine }
            NumberAnimation { to: -root.width * 0.08; duration: 1800; easing.type: Easing.InOutSine }
        }
    }

    Rectangle {
        id: waveFront
        width: parent.width * 1.36
        height: 64
        radius: 31
        x: -parent.width * 0.24
        y: parent.height - 40
        color: "#7700AEEF"

        SequentialAnimation on x {
            loops: Animation.Infinite
            running: root.visible
            NumberAnimation { to: -root.width * 0.12; duration: 2200; easing.type: Easing.InOutSine }
            NumberAnimation { to: -root.width * 0.28; duration: 2200; easing.type: Easing.InOutSine }
        }
    }
}
