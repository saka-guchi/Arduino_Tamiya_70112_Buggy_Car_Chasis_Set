// Seeeduino XIAO.
// Silk: Func(A:Analog,D:Digital,P:PWM,I:I2C,U:UART,S:SPI)
// <Pin Assign>
//  0: A0,D0,DAC
//  1: A1,D1,P              PIN_MOTOR_IN1
//  2: A2,D2,P              PIN_MOTOR_IN2
//  3: A3,D3,P              PIN_MOTOR_VREF
//  4: A4,D4,I(SDA),P       PIN_I2C_SDA
//  5: A5,D5,I(SCL),P       PIN_I2C_SCL
//  6: A6,D6,U(TX),P
//  7: A7,D7,U(RX),P
//  8: A8,D8,S(SCK),P
//  9: A9,D9,S(MISO),P
// 10: A10,D10,S(MOSI),P    PIN_SERVO_PWM
// 11: 3V3: -
// 12: GND: -
// 13: 5V: BUILTIN_LED

//--------------------------------------------------------
// [DEVICE DRIVER]
//--------------------------------------------------------
// Built in LED
// PIN_LED
// PIN_LED2
// PIN_LED3

// I2C
#include <Wire.h>
//#define PIN_I2C_SDA 4
//#define PIN_I2C_SCL 5
#define ADDRESS_SLAVE 0x30

// Servo
#define PIN_SERVO_PWM 10
#include <Servo.h>
Servo sg90;

// Motor driver
#define PIN_MOTOR_IN1  1  // digital pin.
#define PIN_MOTOR_IN2  2  // digital pin.
#define PIN_MOTOR_VREF 3  // PWM pin.
int m_speed = 127;    // -255 to 255.

//--------------------------------------------------------
// [FUNCTION]
//--------------------------------------------------------
void setup(){
    // I2C
    Wire.begin(ADDRESS_SLAVE);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
    Serial.begin(9600);

    // Servo
    sg90.attach(PIN_SERVO_PWM);

    // Motor driver
    pinMode(PIN_MOTOR_IN1, OUTPUT);
    pinMode(PIN_MOTOR_IN2, OUTPUT);
}

void receiveEvent(int howMany)
{
}

void requestEvent()
{
}

void loop(){
    // Servo
    // Turn right
    sg90.write(90);
    delay(1000);

    // Turn left
    sg90.write(150);
    delay(1000);

    // Motor driver
    if(m_speed > 0)
    {
        if( m_speed > 255 ) { m_speed = 255; }

        // Move forward
        digitalWrite(PIN_MOTOR_IN1, HIGH);
        digitalWrite(PIN_MOTOR_IN2, LOW);
        analogWrite(PIN_MOTOR_VREF, m_speed);
    }
    else if(m_speed < 0)
    {
        if( m_speed < -255 ) { m_speed = -255; }

        // Go backward
        digitalWrite(PIN_MOTOR_IN1, LOW);
        digitalWrite(PIN_MOTOR_IN2, HIGH);
        analogWrite(PIN_MOTOR_VREF, m_speed);
    }
    else
    {
        // Stop
        digitalWrite(PIN_MOTOR_IN1, LOW);
        digitalWrite(PIN_MOTOR_IN2, LOW);
    }

}
