// Alarm system app
#include <stdio.h>
#include <stdbool.h>
#include "alarm.h"
#include "systick.h"
#include "gpio.h"
#include "display.h"   //  Added display support

// --------------------------------------------------------
// GPIO pins
// --------------------------------------------------------
static const Pin_t BlueLED     = {GPIOB, 7};   // Pin PB7  -> User LD2
static const Pin_t RedLED      = {GPIOA, 9};   // Pin PA9  -> User LD1
static const Pin_t GreenLED    = {GPIOC, 7};   // Pin PC7  -> User LD3
static const Pin_t MotionSensor= {GPIOB, 9};   // Pin PB9  -> Motion Sensor
static const Pin_t Button      = {GPIOB, 2};   // Pin PB2  -> E-Stop Button
static const Pin_t Buzzer      = {GPIOA, 0};   // Pin PA0  -> Buzzer

// --------------------------------------------------------
// State machine
// --------------------------------------------------------
static enum {DISARMED, ARMED, TRIGGERED} state;

// --------------------------------------------------------
// Constants
// --------------------------------------------------------
#define BLINK_PERIOD 1000    // 1 second for Green and Blue LED
#define DISARM_TIME 3000     // 3 seconds or more to disarm
#define ARM_TIME 2000        // Less than 2 seconds to arm

// --------------------------------------------------------
// Variables
// --------------------------------------------------------
static Time_t pressTime = 0;      // Time button was pressed
static Time_t pressDur  = 0;      // Duration button was held
static bool pressed      = false; // Button short press flag
static Time_t lastToggle = 0;     // Time for LED blinking
static bool greenOn      = true;  // Toggle state for LEDs
static bool motion       = false; // Motion detection flag
static bool buttonHeld   = false; // Long-press flag

// --------------------------------------------------------
// Callback function prototypes
// --------------------------------------------------------
static void CallbackMotionDetect(void);
static void CallbackButtonPress(void);
static void CallbackButtonRelease(void);

// --------------------------------------------------------
// Initialization
// --------------------------------------------------------
void Init_Alarm(void) {
    // Configure LEDs
    GPIO_Enable(BlueLED);
    GPIO_Mode(BlueLED, OUTPUT);
    GPIO_Output(BlueLED, LOW);

    GPIO_Enable(RedLED);
    GPIO_Mode(RedLED, OUTPUT);
    GPIO_Output(RedLED, LOW);

    GPIO_Enable(GreenLED);
    GPIO_Mode(GreenLED, OUTPUT);
    GPIO_Output(GreenLED, LOW);

    // Configure button and motion sensor
    GPIO_Enable(Button);
    GPIO_Mode(Button, INPUT);

    GPIO_Enable(MotionSensor);
    GPIO_Mode(MotionSensor, INPUT);

    // Configure buzzer
    GPIO_Enable(Buzzer);
    GPIO_Mode(Buzzer, OUTPUT);
    GPIO_Output(Buzzer, LOW);

    // Set up interrupts
    GPIO_Callback(MotionSensor, CallbackMotionDetect, RISE);
    GPIO_Callback(Button, CallbackButtonPress, RISE);
    GPIO_Callback(Button, CallbackButtonRelease, FALL);

    // Enable system tick timer
    StartSysTick();
    lastToggle = TimeNow();

    // Set initial state
    state = DISARMED;

    //  Initialize display
    DisplayEnable();
    DisplayColor(WHITE);
    DisplayPrint(0, "DISARMED");
}

// --------------------------------------------------------
// Task (state machine)
// --------------------------------------------------------
void Task_Alarm(void) {
    switch (state) {

    // ----------------------------------------------------
    // DISARMED
    // ----------------------------------------------------
    case DISARMED:
        GPIO_Output(RedLED, LOW);
        GPIO_Output(BlueLED, LOW);
        GPIO_Output(GreenLED, LOW);
        GPIO_Output(Buzzer, LOW);

        if (pressed && pressDur <= ARM_TIME) {
            state = ARMED;
            DisplayColor(YELLOW);             // Update display
            DisplayPrint(0, "ARMED");
            printf("ARMED at time %u\n", TimeNow());
            pressed = false;
            pressDur = 0;
        }

        pressed = false;
        pressDur = 0;
        break;

    // ----------------------------------------------------
    // ARMED
    // ----------------------------------------------------
    case ARMED:
        GPIO_Output(Buzzer, LOW);

        // Blink blue/green LEDs alternately
        if (TimePassed(lastToggle) >= BLINK_PERIOD) {
            GPIO_Output(RedLED, LOW);
            if (greenOn) {
                GPIO_Output(GreenLED, LOW);
                GPIO_Output(BlueLED, HIGH);
            } else {
                GPIO_Output(BlueLED, LOW);
                GPIO_Output(GreenLED, HIGH);
            }
            greenOn = !greenOn;
            lastToggle = TimeNow();
        }

        // Long press → disarm
        if (buttonHeld && TimePassed(pressTime) >= DISARM_TIME) {
            state = DISARMED;
            DisplayColor(WHITE);              //  Update display
            DisplayPrint(0, "DISARMED");
            printf("DISARMED at time %u\n", TimeNow());
            pressed = false;
            buttonHeld = false;
            pressDur = 0;
        }

        // Motion → trigger alarm
        if (motion) {
            state = TRIGGERED;
            DisplayColor(RED);                //  Update display
            DisplayPrint(0, "TRIGGERED");
            motion = false;
        }

        pressed = false;
        pressDur = 0;
        break;

    // ----------------------------------------------------
    // TRIGGERED
    // ----------------------------------------------------
    case TRIGGERED:
        GPIO_Output(RedLED, HIGH);
        GPIO_Output(BlueLED, LOW);
        GPIO_Output(GreenLED, LOW);
        //GPIO_Output(Buzzer, HIGH);

        // Short press → re-arm
        if (pressed && pressDur <= ARM_TIME) {
            state = ARMED;
            DisplayColor(YELLOW);             //  Update display
            DisplayPrint(0, "ARMED");
            printf("ARMED at time %u\n", TimeNow());
            pressed = false;
            pressDur = 0;
            buttonHeld = false;
        }

        // Long press → disarm
        if (buttonHeld && TimePassed(pressTime) >= DISARM_TIME) {
            state = DISARMED;
            DisplayColor(WHITE);              //  Update display
            DisplayPrint(0, "DISARMED");
            printf("DISARMED at time %u\n", TimeNow());
            pressed = false;
            pressDur = 0;
            buttonHeld = false;
        }

        pressed = false;
        pressDur = 0;
        break;
    }
}

// --------------------------------------------------------
// Interrupt callbacks
// --------------------------------------------------------
void CallbackMotionDetect(void) {
    motion = true;  // Motion detected
}

void CallbackButtonPress(void) {
    pressTime = TimeNow();
    buttonHeld = true;
}

void CallbackButtonRelease(void) {
    pressDur = TimePassed(pressTime);
    if (pressDur >= 50) {
        pressed = true;
        buttonHeld = false;
    }
}
