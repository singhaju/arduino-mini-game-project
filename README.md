# Arduino Retro Mini-Game Console

This project is a handheld-style gaming device built using an Arduino (e.g., Uno/Nano), an SH1106 OLED display, an analog joystick, and two push buttons. It features a main menu to select and play classic-style mini-games.

## Games Included

*   **Snake:** Guide the snake to eat food and grow longer, avoiding collisions with itself and the borders. Includes a "Rush" feature.
*   **Flappy Bird:** Navigate a bird through a series of pipes by flapping.
*   **Tetris:** Arrange falling tetrominoes to complete lines and score points. Displays score and pieces placed.

## Controls

*   **Joystick:** Directional movement in games and menu navigation.
*   **Button A (D5):** Primary action (Select in menu, Snake Rush/Retry, Flappy Bird Flap/Retry, Tetris Rotate/Retry).
*   **Button B (D4):** Secondary/Menu action (Return to game menu).

## Simulation & Circuit

A version of this project, demonstrating the core circuit and game interactions, can be simulated on Wokwi:
[Wokwi Simulation Link](https://wokwi.com/projects/430307229810717697)

The physical circuit involves connecting the OLED display (via I2C), the analog joystick, and the two push buttons to the appropriate pins on the Arduino.

## Code Overview

The software is built around a state machine that manages transitions between the menu and the three games. Each game has its own setup, drawing, and input/logic handling functions. The U8g2 library is used for efficient OLED display rendering. Game logic for Snake, Flappy Bird, and Tetris has been adapted and integrated into this common framework.

## Credits & Sources of Inspiration

This project adapts and integrates logic and assets from several excellent open-source Arduino game projects:

*   **Snake Game:** Core gameplay adapted from Joel Jojo P's "Snake Game" on Arduino Project Hub:
    *   [https://projecthub.arduino.cc/joeljojop/snake-game-cb6241](https://projecthub.arduino.cc/joeljojop/snake-game-cb6241)

*   **Flappy Bird Game:** Adapted from "Nano Bird" by richardathome:
    *   [https://gitlab.com/richardathome/nano-bird](https://gitlab.com/richardathome/nano-bird)

*   **Tetris Game:** Core logic and piece concepts adapted from "Tetris Clone with OLED SSD1306 I2C" by BADFEED on Arduino Project Hub:
    *   [https://projecthub.arduino.cc/BADFEED/tetris-clone-with-oled-ssd1306-i2c-for-arduino-nano-uno-ef8a8a](https://projecthub.arduino.cc/BADFEED/tetris-clone-with-oled-ssd1306-i2c-for-arduino-nano-uno-ef8a8a)

*   **U8g2 Library:** By Olik Kraus – essential for display control.

*   **AI Assistance (Gemini):** Provided significant support in merging disparate game codes, refactoring for integration, implementing new features (e.g., Snake Rush, Tetris Piece Count, UI standardization, borders), debugging, and structuring documentation.

## My Own Contribution

*   Conception and overall design of the multi-game console.
*   Hardware assembly and circuit design.
*   Development of the core software framework: state machine, menu system, unified input handling, and main rendering loop.
*   **Significant integration and adaptation work:** Merging the adapted Snake, Flappy Bird, and Tetris game codes into a cohesive system. This involved substantial refactoring of display logic, input mapping, game loop timing for non-blocking operation, and resolving variable conflicts.
*   Implementation of custom features and UI enhancements (e.g., Snake Rush, Tetris Piece Count, standardized game screens, game borders, two-button control scheme).
*   Systematic testing, debugging, and code organization.
