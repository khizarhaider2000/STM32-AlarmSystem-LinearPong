// --------------------------------------------------------
// Main program for Lab 2
// --------------------------------------------------------

#include <stdio.h>
#include "systick.h"
#include "i2c.h"
#include "gpio.h"
#include "alarm.h"
#include "game.h"
#include "display.h"   // ✅ Added as per Lab 2 instructions

int main(void)
{
    // ----------------------------------------------------
    // Initialization
    // ----------------------------------------------------
    StartSysTick();         // Enable system tick timer
    I2C_Enable(LeafyI2C);   // Enable I2C peripheral

    // --------------------------------------------------------
    // Function prototypes (must appear BEFORE Init_Game)
    // --------------------------------------------------------
    void CallbackSelectPress(void);
    void CallbackSelectRelease(void);

//    Init_Alarm();           // Initialize alarm system
    Init_Game();            // Initialize game system


    // ----------------------------------------------------
    // Main loop
    // ----------------------------------------------------
    while (1) {
//        Task_Alarm();           // Handle alarm state machine
        Task_Game();            // Handle game logic

        // ------------------------------------------------
        // Housekeeping
        // ------------------------------------------------
        UpdateIOExpanders();    // Update LED/button I/O expanders
        UpdateDisplay();        // ✅ Added right after UpdateIOExpanders()
        ServiceI2CRequests();   // Handle queued I2C transactions
        WaitForSysTick();       // 1 ms tick delay
    }
}
