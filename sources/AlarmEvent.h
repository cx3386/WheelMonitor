#pragma once
/// Alarm color, generally used in client-side
enum class AlarmEvent {
};

enum class HardwareAlarmEvent {
    Outer_Improc = 0, // ����
    Outer_Sensor, // ����
    Innner_Improc,
    Innner_Sensor,
    Outer_Warn = 0,
    Outer_Alarm,
    Inner_Warn,
    Inner_Alarm,
    Reset
};

//! �����ȼ�
enum class AlarmColor {
    Unkown = 0,
    Green,
    Red,
    Yellow = 4,
    Gray = 8,
    Off = Gray,
};