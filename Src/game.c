#include <stdint.h>        // For integer types
#include <stdbool.h>
#include "game.h"
#include "gpio.h"
#include "systick.h"
#include "display.h"
#include <stdio.h>

// --------------------------------------------------------
// Constants
// --------------------------------------------------------
#define SHIFT_TIME 200U     // Time (ms) between LED shifts
#define SPEED_SLOW 150U
#define SPEED_MED 110U
#define SPEED_FAST 70U
#define NUM_SPEEDS     3
#define QUIT_HOLD_TIME 3000U  // 3 seconds to quit

static bool P1serve = true;

// --------------------------------------------------------

// GPIO pins
// --------------------------------------------------------
//static const Pin_t SelButton      = {GPIOB, 2};   // Pin PB2  -> Select Button

// State Machine
static enum {TITLE, SERVE, PLAY, WIN} state;


// Module-scope variables
// --------------------------------------------------------
static Time_t timeShift;   // Time LED position was last shifted
static int position = 1;   // Current position of illuminated LED
static int direction = 0;  // 0 = right, 1 = left
static int reversed = 0;   // Direction-reversal flag
static int speedIndex = 0; //default slow
static int firstServe = true;
static int P1score = 0;
static int P2score = 0;

static uint32_t speedTable[NUM_SPEEDS] = { SPEED_SLOW, SPEED_MED, SPEED_FAST };

static bool selectPressed = false; // select button press flag

static bool showingScore = false;


// --------------------------------------------------------
// Initialization
// --------------------------------------------------------
void Init_Game(void) {
    GPIO_PortEnable(GPIOX);   // Enable the I/O expander (LEDs & buttons)
    timeShift = TimeNow();    // Record current time
    position = 0;
    direction = 0;
    reversed = 0;
    GPIO_PortOutput(GPIOX, 1 << position);  // Initialize LED at position 0

    // Set initial state
      state = TITLE;

      DisplayEnable();
	  DisplayColor(WHITE);
	  DisplayPrint(0, "PONG");
	  DisplayPrint(1, "Speed: SLOW");

}


// --------------------------------------------------------
// Periodic task
// --------------------------------------------------------
void Task_Game(void) {

	uint16_t inputs;

	static Time_t startHoldTime = 0;
	static bool startHeld = false;

	// Global quit detection (hold Start for 3 seconds)
	bool currStartState = (GPIO_PortInput(GPIOX) & (1 << 11));  // Active-low

	if (currStartState && !startHeld) {
	    // Start button just pressed
	    startHoldTime = TimeNow();
	    startHeld = true;
	    GPIO_PortOutput(GPIOX, 0x00);  // Turn off all LEDs immediately
	} else if (!currStartState) {
	    // Start button released
	    startHeld = false;
	}

	// Check if Start held for 3+ seconds (except in TITLE state)
	if (state != TITLE && startHeld && TimePassed(startHoldTime) >= QUIT_HOLD_TIME) {
	    // LEDs already off from above

	    // Wait for Start button release
	    while (!(GPIO_PortInput(GPIOX) & (1 << 11))) {
	        // Busy wait
	    }

	    // Reset game and return to title
	    P1score = 0;
	    P2score = 0;
	    firstServe = true;
	    speedIndex = 0;
	    position = 0;
	    direction = 0;

	    DisplayColor(WHITE);
	    DisplayPrint(0, "PONG");
	    DisplayPrint(1, "Speed: SLOW");
	    GPIO_PortOutput(GPIOX, 1 << position);
	    timeShift = TimeNow();

	    state = TITLE;
	    startHeld = false;
	    return;
	}

	// If Start is held but not in quit mode, keep LEDs off (except in TITLE)
	if (state != TITLE && startHeld) {
	    GPIO_PortOutput(GPIOX, 0x00);
	}

	switch (state) {
		case TITLE:
		 // Shift LED position every SHIFT_TIME ms
			if (TimePassed(timeShift) >=  speedTable[speedIndex]) {

				if (position == 7)
					direction = 0;

				else if (position == 0)
					direction = 1;


				position += direction ? +1 : -1;

				GPIO_PortOutput(GPIOX, (uint16_t)(1 << position));  // Update LEDs
				timeShift = TimeNow();                              // Reset shift timer
			}



			bool currSelectState = (GPIO_PortInput(GPIOX) & (1 << 12));  // Active-low button
			static bool prevSelectState = false;
			if (currSelectState && !prevSelectState) {
			    // Rising edge (press)
			    selectPressed = true;
			}
			prevSelectState = currSelectState;
			// Handle select button press (speed change)
			if (selectPressed) {
				selectPressed = false;


				// Cycle speed index 0 → 1 → 2 → 0
				speedIndex = (speedIndex + 1) % NUM_SPEEDS;

				// Update display text for feedback
				switch (speedIndex) {
					case 0: DisplayPrint(1, "Speed: SLOW   "); break;
					case 1: DisplayPrint(1, "Speed: MEDIUM "); break;
					case 2: DisplayPrint(1, "Speed: FAST   "); break;
				}
			}



			bool currStartStateTitle = (GPIO_PortInput(GPIOX) & (1 << 11));  // Active-low button
			static bool prevStartStateTitle = false;

			// Only transition on RISING EDGE (button press), not while held
			if (currStartStateTitle && !prevStartStateTitle) {
			    state = SERVE;
			}
			prevStartStateTitle = currStartStateTitle;

		break;

		case SERVE:
			static bool serveReady = false;
			static Time_t serveStart = 0;

			Time_t seed = TimeNow();

			if (firstServe){
				if (seed % 2 == 1) {
					P1serve = true;

				} else {
					P1serve = false;
				}

				firstServe = false;
				serveStart = TimeNow();
				serveReady = true;


			}else if (!serveReady) {

			 int totalPoints = P1score + P2score;

				// Before 10 → alternate every 2 points
				// After 10  → alternate every 1 point
				if ((P1score < 10 && P2score < 10)) {
					if (totalPoints % 2 == 0) {
						P1serve = !P1serve;
					}
				} else {
					// After 10 points reached by anyone, alternate every serve
					P1serve = !P1serve;
				}

	            serveStart = TimeNow();
				serveReady = true;
	        }

			inputs = GPIO_PortInput(GPIOX);


			bool selectHeld = (inputs & (1 << 12));


			// --- Detect serve button (active-low) ---
			bool P2press = !(inputs & ((1 << 8) | (1 << 9) | (1 << 10)));
			bool P1press = !(inputs & ((1 << 13) | (1 << 14) | (1 << 15)));

			if (serveReady) {


				 if (P1serve) {
					DisplayColor(CYAN);
					DisplayPrint(0, "1P SERVES");
					DisplayPrint(1, "");
					position = 6;
					GPIO_PortOutput(GPIOX, (uint16_t)(1 << position));
				} else {
					DisplayColor(YELLOW);
					DisplayPrint(0, "2P SERVES");
					DisplayPrint(1, "");
					position = 1;
					GPIO_PortOutput(GPIOX, (uint16_t)(1 << position));
				}

				 if (selectHeld) {
					 DisplayColor(RED);
					 char scoreText[17];
					 sprintf(scoreText, "SCORE  %d - %d", P1score, P2score);
					 DisplayPrint(1, scoreText);

					 // Show score in binary on LEDs
					 uint8_t scoreBinary = ((P1score & 0x0F) << 4) | (P2score & 0x0F);
					 GPIO_PortOutput(GPIOX, scoreBinary);
				}

				if ((P1serve && !P1press) || (!P1serve && !P2press)) {
					// Start the volley

					direction = P1serve ? 0 : 1;
					DisplayColor(WHITE);
					DisplayPrint(0, "PLAY!");
					DisplayPrint(1, "");

					serveReady = false;

					timeShift = TimeNow();
					state = PLAY;
					msDelay(400);

				}

			}
		break;

		case PLAY:
			inputs = GPIO_PortInput(GPIOX);

			if (TimePassed(timeShift) >= speedTable[speedIndex]) {
			        // Move ball one step in current direction
				position += direction ? +1 : -1;
				GPIO_PortOutput(GPIOX, (uint16_t)(1 << position));
				timeShift = TimeNow();
			}

			static bool prevP1State = true;
			static bool prevP2State = true;


			 // read active-low inputs
			 bool currP2State = (inputs & ((1 << 8) | (1 << 9) | (1 << 10)));
			 bool currP1State = (inputs & ((1 << 13) | (1 << 14) | (1 << 15)));


			 // detect *edge* — button was just pressed
			 bool P1return = (currP1State && prevP1State); // went high→low
			 bool P2return = (currP2State && prevP2State);

			 prevP1State = currP1State;
			 prevP2State = currP2State;


			 if (position == 0){
				 if (P2return) {
					direction = 1;  // send ball right
					DisplayColor(YELLOW);
				}
			 }

			 if (position == 7){
				 if (P1return) {
					direction = 0;  // send ball left
					DisplayColor(CYAN);
				}
			 }


			if (position == -1) {  // left edge → P1’s turn

				if (P2return) {
					direction = 1;  // send ball right
					DisplayColor(YELLOW);
				} else {
					P1score++;
					DisplayColor(CYAN);
					DisplayPrint(0, "1P SCORES!");

					 // Check win condition
					if (P1score >= 11 && (P1score - P2score) >= 2) {
						state = WIN;
					} else {
						state = SERVE;
					}

					msDelay(100);
				}
			}

			else if (position == 8) {  // right edge → P2’s turn

				if (P1return) {
					direction = 0;  // send ball left
					DisplayColor(CYAN);
				} else {
					P2score++;
					DisplayColor(YELLOW);
					DisplayPrint(0, "2P SCORES!");

					// Check win condition
					if (P2score >= 11 && (P2score - P1score) >= 2) {
						state = WIN;
					} else {
						state = SERVE;
					}

					msDelay(100);
				}
			}

		break;

		case WIN:
			// Display winner
			if (P1score > P2score) {
				DisplayColor(CYAN);
				DisplayPrint(0, "PLAYER 1 WINS!");
			} else {
				DisplayColor(YELLOW);
				DisplayPrint(0, "PLAYER 2 WINS!");
			}

			char finalScore[17];
			sprintf(finalScore, "Score: %d - %d", P1score, P2score);
			DisplayPrint(1, finalScore);

			// Flash all LEDs
			static Time_t flashTime = 0;
			static bool ledsOn = false;

			if (TimePassed(flashTime) >= 500) {
				ledsOn = !ledsOn;
				GPIO_PortOutput(GPIOX, ledsOn ? 0xFF : 0x00);
				flashTime = TimeNow();
			}

			// Wait for Start button press to return to title
			currStartState = (GPIO_PortInput(GPIOX) & (1 << 11));
			static bool prevStartStateWin = false;
			if (currStartState && !prevStartStateWin) {
				// Reset game
				P1score = 0;
				P2score = 0;
				firstServe = true;
				position = 0;
				direction = 0;

				DisplayColor(WHITE);
				DisplayPrint(0, "PONG");
				switch (speedIndex) {
				case 0: DisplayPrint(1, "Speed: SLOW   "); break;
				case 1: DisplayPrint(1, "Speed: MEDIUM "); break;
				case 2: DisplayPrint(1, "Speed: FAST   "); break;
				}
				GPIO_PortOutput(GPIOX, 1 << position);
				timeShift = TimeNow();

				state = TITLE;
			}
			prevStartStateWin = currStartState;
		break;
	}


}


