# Arduino Retro Mini-Game Console

This project is a simple handheld-style gaming console built using an Arduino (or compatible board), an SH1106 OLED display, an analog joystick, and two push buttons. It features a main menu to select and play three classic-style mini-games: Snake, Flappy Bird, and Tetris.

## Features

*   Menu system for game selection.
*   **Snake Game:** Classic snake gameplay with increasing speed (rush button) and high score tracking.
*   **Flappy Bird Game:** Side-scrolling bird game with pipe avoidance and high score tracking.
*   **Tetris Game:** Falling block puzzle game with rotation, line clearing, scoring, piece count, and next piece preview.
*   Input via analog joystick and two primary action buttons.
*   Visual borders for Snake and Flappy Bird play areas.
*   Paged rendering for efficient display updates on the OLED.

## Simulation & Circuit

A simulation of this project, demonstrating its functionality and circuit connections, can be found on Wokwi:
[Arduino Multi-Game Console on Wokwi](https://wokwi.com/projects/430307229810717697)

The circuit generally consists of:
*   Arduino (e.g., Uno/Nano)
*   SH1106 128x64 I2C OLED Display
*   Analog Joystick Module (for X/Y movement)
*   Two Push Buttons (for actions and menu navigation)

## Code Structure

The code is organized with a main state machine managing transitions between the menu and individual games. Each game has its own setup, drawing, and input/logic handling functions for modularity. The U8g2 library is used for display control.

## Credits & Sources of Inspiration

This project builds upon and adapts code and concepts from several excellent open-source projects and resources. Full credit and thanks go to the original creators:

*   **Snake Game Logic:** Adapted from the "Snake Game" project by Joel Jojo P.
    *   [Arduino Project Hub - Snake Game](https://projecthub.arduino.cc/joeljojop/snake-game-cb6241)

*   **Flappy Bird Game Mechanics:** Adapted from "Nano Bird" concepts, with display and input logic modified for this project. A common source for this style of game is often attributed to projects like the one by RichardAtHome.
    *   [GitLab - Nano Bird by RichardAtHome](https://gitlab.com/richardathome/nano-bird) 

*   **Tetris Game Logic & Piece Definitions:** Adapted from the "Tetris Clone with OLED SSD1306" project by BADFEED.
    *   [Arduino Project Hub - Tetris Clone](https://projecthub.arduino.cc/BADFEED/tetris-clone-with-oled-ssd1306-i2c-for-arduino-nano-uno-ef8a8a)

*   **U8g2 Library:** An essential library for OLED display control, created by Olik Kraus.
    *   [U8g2 GitHub Repository](https://github.com/olikraus/u8g2)

*   **AI Assistance (Gemini):** Provided support for code merging of the various game sources, feature implementation (like snake rush, Tetris piece count, UI standardization), debugging, and documentation structuring.

## My Own Contribution

*   Overall project conception and system design for the multi-game console.
*   Hardware assembly and circuit integration.
*   Development of the core software framework: state machine, menu system, unified input handling, and main paged-rendering loop.
*   **Significant integration and adaptation** of the sourced Snake, Flappy Bird, and Tetris game codes to function cohesively within the single console environment. This involved refactoring display logic, input mapping, game loop timing, and variable management.
*   Implementation of custom features and enhancements (e.g., Snake rush, Tetris piece count, standardized UI, game borders, control scheme consolidation).
*   Systematic testing, debugging, and code organization.

---

**How to Use this README:**

1.  **Review the Flappy Bird Source:** The link you provided for Flappy Bird (`https://gitlab.com/richardathome/nano-bird`) is a good example. If this is the primary one you drew from, great! If your adaptation was more generally based on the common "Nano Bird" code found in many Arduino examples (often originally by `datacute` or similar), you might just mention "Adapted from common 'Nano Bird' Arduino examples." For a specific credit, always try to link to the most direct source if possible.
2.  **Add Your Name:** Replace `[Your Name Here]` if you add an author section.
3.  **Save as `README.md`:** Place this file in the root directory of your project. Markdown files (`.md`) are commonly used for project descriptions on platforms like GitHub, GitLab, etc.
