# Arduino Pong

A classic **PONG game** running on an Arduino Nano with an SSD1306 OLED display, two potentiometers, a push button and a passive buzzer.

The project recreates the classic Pong gameplay on a small embedded system, including paddle controls, ball physics, score tracking, sound effects, pause functionality and a spin effect based on paddle movement.

## 🎥 Demo

A demonstration of the Arduino Pong game can be seen in the following video:

[![Project demo](https://img.youtube.com/vi/wbXttuUYay4/hqdefault.jpg)](https://www.youtube.com/watch?v=wbXttuUYay4)

*Click the image to watch the full video on YouTube.*

## Features

* Classic two-player Pong gameplay
* SSD1306 128×64 OLED display
* Independent paddle control using two potentiometers
* Start and pause functionality using a push button
* Button debounce handled using an interrupt
* Ball bouncing against the screen borders and paddles
* Paddle movement affects the ball trajectory through a spin effect
* Score tracking for both players
* Sound effects for:

  * Game start
  * Ball collisions
  * Scoring
* Splash screen with project credits
* Pause indicator displayed on screen

## Hardware

* Arduino Nano (ATmega328P)
* SSD1306 OLED 128×64 I2C display
* 2 × potentiometers
* Push button
* Passive buzzer

## Controls

| Component            | Function        |
| -------------------- | --------------- |
| Potentiometer A → A0 | Player A paddle |
| Potentiometer B → A1 | Player B paddle |
| Push button → D2     | Start / Pause   |
| Passive buzzer → D3  | Sound effects   |

### Button behavior

* On startup, the game is paused on the splash screen.
* Pressing the button starts the game.
* Pressing the button again pauses or resumes the game.
* The button input uses an interrupt with software debounce to prevent accidental multiple triggers.

## Display

The game uses an **SSD1306 128×64 OLED display** connected through I2C.

The display shows:

* Player A score
* Player B score
* Both paddles
* Ball
* Center line
* `PAUSED` indicator when the game is paused

The OLED uses the standard I2C address:

```text
0x3C
```

## Game Mechanics

### Paddle movement

Each player controls their paddle using a potentiometer connected to an analog input.

The analog value is mapped to the vertical position of the paddle:

```text
Potentiometer → Analog input → Paddle position
```

### Ball physics

The ball moves continuously across the screen and bounces when it hits:

* The top or bottom screen borders
* Player A's paddle
* Player B's paddle

When the ball reaches the left or right side without being blocked by a paddle, the corresponding player scores a point.

### Spin effect

The game includes a simple spin mechanic.

The vertical movement of each paddle is measured between frames. When the ball hits a paddle, its vertical speed is modified according to the paddle's movement.

This allows the player to influence the ball's trajectory by moving the paddle while making contact with the ball.

## Sound Effects

The passive buzzer provides simple audio feedback using the Arduino `tone()` function.

Different sounds are used for:

| Event          | Sound                      |
| -------------- | -------------------------- |
| Game start     | Rising three-tone sequence |
| Ball collision | Short tone                 |
| Point scored   | Lower, longer tone         |

## Wiring

See the complete wiring diagram in:

**[`docs/wiring.md`](docs/wiring.md)**

### Pin assignment

| Component       | Arduino Pin |
| --------------- | ----------- |
| OLED SDA        | A4          |
| OLED SCL        | A5          |
| Potentiometer A | A0          |
| Potentiometer B | A1          |
| Push button     | D2          |
| Passive buzzer  | D3          |

### OLED power

```text
OLED VCC → 5V
OLED GND → GND
OLED SDA → A4
OLED SCL → A5
```

### Button

```text
Button → D2
Button → GND
```

The button uses the Arduino's internal pull-up resistor.

### Buzzer

```text
Buzzer + → D3
Buzzer - → GND
```

## Libraries

The project uses the following Arduino libraries:

* **Adafruit GFX Library**
* **Adafruit SSD1306**
* **Wire** (included with the Arduino framework)

## Project Structure

```text
arduino-pong/
├── README.md
├── LICENSE
├── .gitignore
├── src/
│   └── Arduino_Pong.cpp
└── docs/
    ├── wiring.md
    └── pong_screenshot.png
```

## Improvements

Some possible future improvements include:

* **Double-button press to restart the game** without requiring an Arduino reset.
* Add a dedicated **game reset function** to return the score, ball position and paddle states to their initial values.
* Add a **win condition** and game-over screen.
* Add configurable difficulty levels.
* Improve the ball physics and collision detection.
* Add additional sound effects.
* Add a more advanced scoring or match system.
* Improve the pause and start interface.

### Planned reset behavior

Currently, once a game has started, the only way to completely restart the match and reset the score is to reset the Arduino.

A future version will implement a **double press of the pause button** as a game reset command:

```text
Single press       → Pause / Resume
Double press       → Reset game
                      ↓
                  Score = 0
                  Ball reset
                  Game restarted
```

This will allow a new match to be started without physically resetting the Arduino.

## Author

**Rafael Ramírez Salas**
