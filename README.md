# 🕹️ STM32 Embedded Systems – Alarm System & Linear Pong Game

This project was developed for **CEG3136 (Computer Architecture & Embedded Systems)** at the **University of Ottawa (Fall 2025)**.  
It demonstrates embedded programming concepts such as **real-time I/O control**, **I²C communication**, **state machines**, and **cooperative multitasking** on the **STM32L552ZET6Q** microcontroller.

---

## 🚀 Overview

This lab consists of **two main systems**:

### 🔔 Alarm System
A simulated security system controlled through LEDs, push buttons, and an LCD display.  
- Displays system states — **DISARMED**, **ARMED**, and **TRIGGERED** — on the 16×2 LCD via I²C.  
- Uses LED feedback to indicate system mode.  
- Buttons simulate arming, triggering, and disarming.  
- Demonstrates **I²C communication**, **GPIO I/O**, and **finite-state logic**.

### 🟢 Linear Pong Game
A two-player **LED-based Pong game** that runs on the STM32 board.  
- LEDs act as the “ball” moving across 8 positions.  
- Players use push-buttons to return the ball at the edges.  
- Features **adjustable speed**, **randomized serving**, and **score display** on the LCD.  
- Built using **non-blocking timing** and **state machines**, running at a 1 ms system tick for smooth, real-time gameplay.

---

## 🧠 Skills & Concepts Demonstrated
- Embedded **C programming** for real-time systems  
- **I²C driver implementation** and communication handling  
- **GPIO configuration** for LED and button control  
- **Finite-state machine** and **multitasking** design  
- **SysTick-based scheduling** for timing  
- Hardware abstraction via modular drivers  
- Debugging using **printf() via SWO**

---

## ⚙️ Hardware Setup
- **Microcontroller:** STM32L552ZET6Q (Leafy development board)  
- **Peripherals:**
  - 8 LEDs and 8 push buttons (via GPIO/I²C expanders)
  - 16×2 LCD with RGB backlight
  - Optional buzzer for alarm feedback  
- **Interfaces:** I²C, GPIO, SysTick timer interrupt

---

## 🧩 Key Source Files Overview

### 🔹 `main.c`
Initializes system peripherals (SysTick, I²C, GPIO, Display) and runs the main control loop.  
Selects whether to run the **Alarm System** or **Linear Pong Game**, updating LEDs, display, and I/O expanders in real time.

---

### 🔹 `game.c` / `game.h`
Implements the **Linear Pong** game logic.  
- Uses a **finite-state machine** (`TITLE`, `SERVE`, `PLAY`, `WIN`).  
- Manages LED movement, button inputs, and scoring.  
- Displays game information on the LCD.  
- Uses **non-blocking timing** and **random serving logic** for responsiveness.

---

### 🔹 `alarm.c` / `alarm.h`
Implements the **Alarm System**.  
- Displays **ARMED**, **TRIGGERED**, and **DISARMED** states on the LCD.  
- Uses button input to control alarm status.  
- Demonstrates **I²C display control**, **GPIO input/output**, and **state-based transitions**.

---

### 🔹 `display.c` / `display.h`
Handles the **16×2 LCD** and RGB backlight via I²C.  
- `DisplayPrint()` for formatted text.  
- `DisplayColor()` for backlight changes.  
- Queues I²C transfers to update text and color asynchronously.

---

### 🔹 `gpio.c` / `gpio.h`
Manages LED and button I/O, including the **I²C-based port expander**.  
- Reads button states and drives LED outputs.  
- Handles `UpdateIOExpanders()` for real-time synchronization.

---

### 🔹 `i2c.c` / `i2c.h`
Implements a **non-blocking I²C driver** using a queued transfer system.  
- Supports multiple devices (LCD, I/O expander, RGB backlight).  
- Called continuously in the main loop via `ServiceI2CRequests()`.

---

### 🔹 `systick.c` / `systick.h`
Provides the **1 ms SysTick timer** for real-time scheduling.  
- Tracks time via `TimeNow()` and `TimePassed()`.  
- Drives both Alarm and Pong timing logic.

---

## 📂 Project Structure
├── Src/
│ ├── main.c
│ ├── alarm.c
│ ├── game.c
│ ├── display.c
│ ├── gpio.c
│ ├── i2c.c
│ └── systick.c
│
├── Inc/
│ ├── alarm.h
│ ├── game.h
│ ├── display.h
│ ├── gpio.h
│ ├── i2c.h
│ └── systick.h
│
└── README.md



---

## 🧪 How to Run
1. Open the project in **STM32CubeIDE**.  
2. Connect the **Leafy STM32 board** via USB.  
3. Build and flash the program.  
4. Uncomment either `Init_Alarm()` or `Init_Game()` in `main.c`.  
5. Interact using physical buttons:
   - **Alarm System:** Arm/disarm; LCD and LEDs show mode.  
   - **Linear Pong:** Press player buttons to hit; hold *Start* 3s to reset.

---

## 👤 Author
**Khizar Haider**  
B.Sc. Computer Engineering — University of Ottawa  
[LinkedIn](https://www.linkedin.com/in/khizarhaider)

---

## 📄 License
For educational purposes under the University of Ottawa CEG3136 course.  
Free to reference and modify for learning.
