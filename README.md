# Arduino Retro Mini-Game Console

A multi-game console built on an Arduino-compatible board, featuring Snake, Flappy Bird, and Tetris, displayed on an SH1106 OLED screen and controlled via an analog joystick and push buttons.

## Features

*   Interactive menu for game selection.
*   Three classic games:
    *   **Snake:** Guide the snake to eat food and grow, avoiding self-collision and borders. Features a "Rush" button.
    *   **Flappy Bird:** Navigate the bird through pipe gaps.
    *   **Tetris:** Place falling tetrominoes to clear lines. Tracks score and pieces placed.
*   Joystick and two-button controls (Action/Select and Menu/Back).
*   High score tracking for each game.
*   Visual borders for Snake and Flappy Bird play areas.

## Hardware (Typical Setup)

*   Arduino (Uno, Nano, or compatible)
*   SH1106 I2C OLED Display (128x64)
*   Analog Joystick Module (2-axis with switch, switch not primarily used)
*   2x Push Buttons

## Software & Libraries

*   Arduino IDE
*   **U8g2 Library by Olik Kraus:** For OLED display control.
*   **Wire Library:** For I2C communication.

## Game Logic Adaptation & Credits

This project integrates and adapts core gameplay logic from several excellent open-source Arduino projects. Full credit goes to the original creators for their foundational work. My contribution focused on merging these into a unified multi-game system, adapting them for the specific hardware and U8g2 library, implementing a common menu and input scheme, and adding custom features.

*   **Snake Game:**
    *   Core gameplay adapted from: [Snake Game by joeljojop](https://projecthub.arduino.cc/joeljojop/snake-game-cb6241)

*   **Flappy Bird Game:**
    *   Core gameplay adapted from "Nano Bird" concepts, similar to: [Nano Bird by richardathome](https://gitlab.com/richardathome/nano-bird) (Note: The exact "Nano Bird" by datacute is harder to find a single canonical link for, this is a similar well-documented version).

*   **Tetris Game:**
    *   Core gameplay, piece logic, and rotation adapted from: [Tetris Clone by BADFEED](https://projecthub.arduino.cc/BADFEED/tetris-clone-with-oled-ssd1306-i2c-for-arduino-nano-uno-ef8a8a)
    *   (The "Tiny Tetris" by Anthony Russell was also a reference, especially for piece PROGMEM storage concepts if applicable to your final version).

*   **AI Assistance (Gemini):** Provided significant support in code merging, feature implementation, debugging, and documentation structuring.

## My Contribution

*   Design and assembly of the physical multi-game console hardware.
*   Development of the core software framework:
    *   Central state machine for managing game states (Menu, Snake, Flappy, Tetris).
    *   Menu system for game selection.
    *   Unified input handling for joystick and buttons.
    *   Main program loop with paged rendering for the OLED display.
*   **Extensive integration and adaptation** of the three core game logics into this unified framework. This involved:
    *   Refactoring display calls to use the shared U8g2 library instance.
    *   Re-mapping input controls to the console's joystick and 2-button scheme.
    *   Modifying game loops to be non-blocking and fit within the state machine.
    *   Resolving variable conflicts and managing game-specific data.
*   Implementation of custom features:
    *   Snake "Rush" mechanic.
    *   Tetris "Piece Count" display.
    *   Standardized "Game Over" and "Retry/Menu" prompts.
    *   Visual borders in Snake and Flappy Bird.
*   Systematic testing, debugging, and code organization.

## How to Use

1.  Assemble the circuit as described (refer to comments in the `.ino` file for pin connections if not using a schematic).
2.  Upload the `.ino` sketch to your Arduino board using the Arduino IDE.
3.  Navigate the menu with the joystick and select a game with Button A.
4.  Enjoy the games! Button B typically returns to the menu.

---

This README provides a good overview, acknowledges the original sources properly, and clearly states your contributions. You can adjust the details as needed to perfectly match your final project.
