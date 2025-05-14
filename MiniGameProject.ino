#include <Arduino.h>
#include <U8g2lib.h> // U8g2 Library
#include <Wire.h>    // For I2C

// --- U8g2 Constructor for Paged Mode (Saves RAM) ---
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Screen dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Pin Definitions
#define JOY_VERT A0
#define JOY_HORZ A1
#define BTN_A_PIN 5
#define BTN_B_PIN 4
#define BTN_C_PIN 3
#define BTN_D_PIN 2

// Joystick thresholds
#define JOY_SENSITIVITY 200

// Game States
enum GameState { MENU, SNAKE_GAME, FLAPPY_BIRD_GAME, TETRIS_GAME };
GameState currentGameState = MENU;

// High Scores
int snakeMaxScore = 0;
int flappyBirdMaxScore = 0;
int tetrisMaxScore = 0;

// Menu variables
const char* gameNames[] = {"Snake Game", "Flappy Bird", "Tetris Game"};
const int numGames = 3;
int selectedGame = 0;
unsigned long lastMenuInputTime = 0;
const int menuInputDelay = 150;
const int FONT_LINE_HEIGHT = 10;
const int SMALL_FONT_LINE_HEIGHT = 8;

// --- Snake Game Variables ---
#define SNAKE_GRID_SIZE 4
#define MAX_SNAKE_LENGTH 128
struct Point {
    byte x;
    byte y;
};
Point snake[MAX_SNAKE_LENGTH];
int snakeLength;
Point food;
enum Direction { UP, DOWN, LEFT, RIGHT, NONE };
Direction snakeDir;
Direction lastSnakeDir;
int snakeCurrentScore;
bool snakeGameOver = false;
unsigned long lastSnakeMoveTime = 0;
const int normalSnakeMoveInterval = 200;
const int fastSnakeMoveInterval = 70;
int currentSnakeMoveInterval = normalSnakeMoveInterval;
const int SNAKE_GRID_PIXEL_WIDTH = (SCREEN_WIDTH / SNAKE_GRID_SIZE) * SNAKE_GRID_SIZE;
const int SNAKE_GRID_PIXEL_HEIGHT = (SCREEN_HEIGHT / SNAKE_GRID_SIZE) * SNAKE_GRID_SIZE;
const int SNAKE_PLAY_AREA_X_OFFSET = 0;
const int SNAKE_PLAY_AREA_Y_OFFSET = 0;


// --- Flappy Bird Game Variables ---
bool flappyBirdGameOver = false;
int flappyBirdCurrentScore = 0;
#define FLAPPY_SPRITE_HEIGHT   16
#define FLAPPY_SPRITE_WIDTH    16
#define FLAPPY_UPDATE_INTERVAL 65
unsigned long flappy_lastUpdateTime = 0;
int flappy_bird_x;
int flappy_bird_y;
int flappy_momentum = 0;
int flappy_wall_x[2];
int flappy_wall_y[2];
int flappy_wall_gap = 35;
int flappy_wall_width = 10;
#define FLAPPY_FONT_SMALL u8g2_font_6x10_tf
static const unsigned char PROGMEM flappy_wing_down_bmp[] = {
    0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B11000000, 0B00000011, 0B11111000, 0B00001111,
    0B11111100, 0B00011100, 0B11111110, 0B01111110, 0B11111111, 0B10000011, 0B11111011, 0B01111110,
    0B11111011, 0B00011110, 0B11111011, 0B00011110, 0B01110011, 0B00011110, 0B10001111, 0B00001111,
    0B11111110, 0B00000111, 0B11111100, 0B00000011, 0B11100000, 0B00000000, 0B00000000, 0B00000000
};
static const unsigned char PROGMEM flappy_wing_up_bmp[] = {
    0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B11000000, 0B00000011, 0B11111000, 0B00001111,
    0B11111100, 0B00011100, 0B10001110, 0B01111110, 0B01110111, 0B10000011, 0B11111011, 0B01111110,
    0B11111011, 0B00011110, 0B11111111, 0B00011111, 0B11111111, 0B00011111, 0B11111111, 0B00001111,
    0B11111110, 0B00000111, 0B11111100, 0B00000011, 0B11100000, 0B00000000, 0B00000000, 0B00000000
};

// --- Tetris Game Variables ---
#define TETRIS_GRID_WIDTH 10
#define TETRIS_GRID_HEIGHT 20
#define TETRIS_BLOCK_SIZE 3
#define TETRIS_PLAYFIELD_X_OFFSET ((SCREEN_WIDTH - (TETRIS_GRID_WIDTH * TETRIS_BLOCK_SIZE) - (4 * TETRIS_BLOCK_SIZE) - 10) / 2)
#define TETRIS_PLAYFIELD_Y_OFFSET 1
byte tetrisPlayfield[TETRIS_GRID_HEIGHT][TETRIS_GRID_WIDTH];
struct TetrisPiece {
    byte shape[4][4];
    int gridX;
    int gridY;
    byte type;
};
TetrisPiece tetrisCurrentBlock;
byte tetrisNextBlockType;
unsigned long tetris_lastAutoDropTime = 0;
int tetris_autoDropInterval = 800;
int tetris_currentLevel = 0;
int tetris_linesClearedThisLevel = 0;
const int tetris_linesPerLevel = 10;
bool tetrisGameOver = false;
int tetrisCurrentScore = 0;
int tetrisPiecesPlaced = 0;
const byte TetrisBlocksPROGMEM[7][2] PROGMEM = {
    {0B01000100, 0B01000100}, {0B11000000, 0B01000100}, {0B01100000, 0B01000100}, {0B01100000, 0B00000110},
    {0B11000000, 0B00000110}, {0B01000000, 0B00001110}, {0B01100000, 0B00001100}
};

struct JoystickState {
    bool up, down, left, right, center;
    int rawX, rawY;
};

int freeMemory() {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

JoystickState readJoystick() {
    JoystickState joy;
    joy.rawX = analogRead(JOY_HORZ);
    joy.rawY = analogRead(JOY_VERT);
    joy.up = (joy.rawY < (512 - JOY_SENSITIVITY));
    joy.down = (joy.rawY > (512 + JOY_SENSITIVITY));
    joy.left = (joy.rawX < (512 - JOY_SENSITIVITY));
    joy.right = (joy.rawX > (512 + JOY_SENSITIVITY));
    if (joy.up || joy.down) {
        joy.left = false;
        joy.right = false;
    }
    joy.center = !joy.up && !joy.down && !joy.left && !joy.right;
    return joy;
}

bool isButtonPressed(int pin) {
    return digitalRead(pin) == LOW;
}

bool checkAndDebounce(int pin) {
    if (isButtonPressed(pin)) {
        delay(50);
        if (isButtonPressed(pin)) {
            while (isButtonPressed(pin)) {
                delay(10);
            }
            return true;
        }
    }
    return false;
}

void game_textAt(int x, int y, const char* txt) {
    u8g2.drawStr(x, y, txt);
}

void game_textAtCenter(int y, const char* txt, const uint8_t* font) {
    u8g2.setFont(font);
    int tw = u8g2.getStrWidth(txt);
    int x = (SCREEN_WIDTH - tw) / 2;
    game_textAt(x, y, txt);
}


void tetris_getShape(byte type, byte sh[4][4]) {
    if (type < 1 || type > 7) {
        for (int r = 0; r < 4; r++) for (int c = 0; c < 4; c++) sh[r][c] = 0;
        return;
    }
    for (int r = 0; r < 4; r++) for (int c = 0; c < 4; c++) sh[r][c] = 0;
    byte br = 0, bc = 0;
    for (int i = 0; i < 2; i++) {
        byte cb = pgm_read_byte(&TetrisBlocksPROGMEM[type - 1][i]);
        for (int j = 0; j < 8; j++) {
            if ((cb >> j) & 1) sh[br][bc] = 1;
            bc++;
            if (bc >= 4) {
                bc = 0;
                br++;
            }
            if (br >= 4) break;
        }
        if (br >= 4) break;
    }
}

bool tetris_checkCollision(const TetrisPiece& p) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (p.shape[r][c]) {
                int pr = p.gridY + r;
                int pc = p.gridX + c;
                if (pc < 0 || pc >= TETRIS_GRID_WIDTH || pr < 0 || pr >= TETRIS_GRID_HEIGHT) return true;
                if (tetrisPlayfield[pr][pc]) return true;
            }
        }
    }
    return false;
}

void tetris_spawnNewPiece() {
    tetrisPiecesPlaced++;
    tetrisCurrentBlock.type = tetrisNextBlockType;
    tetris_getShape(tetrisCurrentBlock.type, tetrisCurrentBlock.shape);
    tetrisCurrentBlock.gridX = TETRIS_GRID_WIDTH / 2 - 2;
    tetrisCurrentBlock.gridY = 0;
    tetrisNextBlockType = random(1, 8);
    if (tetris_checkCollision(tetrisCurrentBlock)) {
        tetrisGameOver = true;
        tetrisPiecesPlaced--;
    }
}

void tetris_addPieceToPlayfield() {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (tetrisCurrentBlock.shape[r][c]) {
                int pr = tetrisCurrentBlock.gridY + r;
                int pc = tetrisCurrentBlock.gridX + c;
                if (pr >= 0 && pr < TETRIS_GRID_HEIGHT && pc >= 0 && pc < TETRIS_GRID_WIDTH) {
                    tetrisPlayfield[pr][pc] = 1;
                }
            }
        }
    }
}

void tetris_clearLines() {
    int lcc = 0;
    for (int r = TETRIS_GRID_HEIGHT - 1; r >= 0; r--) {
        bool lf = true;
        for (int c = 0; c < TETRIS_GRID_WIDTH; c++) {
            if (tetrisPlayfield[r][c] == 0) {
                lf = false;
                break;
            }
        }
        if (lf) {
            lcc++;
            tetris_linesClearedThisLevel++;
            for (int mr = r; mr > 0; mr--) {
                for (int mc = 0; mc < TETRIS_GRID_WIDTH; mc++) {
                    tetrisPlayfield[mr][mc] = tetrisPlayfield[mr - 1][mc];
                }
            }
            for (int mc = 0; mc < TETRIS_GRID_WIDTH; mc++) {
                tetrisPlayfield[0][mc] = 0;
            }
            r++;
        }
    }
    if (lcc > 0) {
        if (lcc == 1) tetrisCurrentScore += 40 * (tetris_currentLevel + 1);
        else if (lcc == 2) tetrisCurrentScore += 100 * (tetris_currentLevel + 1);
        else if (lcc == 3) tetrisCurrentScore += 300 * (tetris_currentLevel + 1);
        else if (lcc >= 4) tetrisCurrentScore += 1200 * (tetris_currentLevel + 1);

        if (tetrisCurrentScore > tetrisMaxScore) tetrisMaxScore = tetrisCurrentScore;

        if (tetris_linesClearedThisLevel >= tetris_linesPerLevel) {
            tetris_currentLevel++;
            tetris_linesClearedThisLevel = 0;
            if (tetris_autoDropInterval > 100) tetris_autoDropInterval -= 50;
        }
    }
}

void tetris_rotatePiece() {
    if (tetrisGameOver) return;
    TetrisPiece rp = tetrisCurrentBlock;
    byte ts[4][4];
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            ts[c][3 - r] = rp.shape[r][c];
        }
    }
    memcpy(rp.shape, ts, sizeof(ts));
    if (!tetris_checkCollision(rp)) {
        memcpy(tetrisCurrentBlock.shape, rp.shape, sizeof(rp.shape));
    } else {
        TetrisPiece kp = rp;
        kp.gridX--;
        if (!tetris_checkCollision(kp)) {
            memcpy(tetrisCurrentBlock.shape, kp.shape, sizeof(kp.shape));
            tetrisCurrentBlock.gridX = kp.gridX;
            return;
        }
        kp.gridX += 2;
        if (!tetris_checkCollision(kp)) {
            memcpy(tetrisCurrentBlock.shape, kp.shape, sizeof(kp.shape));
            tetrisCurrentBlock.gridX = kp.gridX;
            return;
        }
    }
}

bool tetris_movePiece(int dx, int dy) {
    if (tetrisGameOver) return false;
    TetrisPiece mp = tetrisCurrentBlock;
    mp.gridX += dx;
    mp.gridY += dy;
    if (!tetris_checkCollision(mp)) {
        tetrisCurrentBlock = mp;
        return true;
    }
    return false;
}

void setup() {
    Serial.begin(9600);
    Serial.print(F("Setup. RAM free: "));
    Serial.println(freeMemory());
    if (!u8g2.begin()) {
        Serial.println(F("u8g2 fail!"));
        for (;;);
    }
    Serial.println(F("u8g2 ok."));
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setDrawColor(1);
    u8g2.setFontPosBaseline();
    u8g2.enableUTF8Print();
    pinMode(JOY_VERT, INPUT);
    pinMode(JOY_HORZ, INPUT);
    pinMode(BTN_A_PIN, INPUT_PULLUP);
    pinMode(BTN_B_PIN, INPUT_PULLUP);
    pinMode(BTN_C_PIN, INPUT_PULLUP);
    pinMode(BTN_D_PIN, INPUT_PULLUP);
    randomSeed(analogRead(A2));
    Serial.println(F("Setup complete."));
}

void loop() {
    u8g2.firstPage();
    do {
        switch (currentGameState) {
            case MENU:
                drawMenuScreen_Paged();
                break;
            case SNAKE_GAME:
                drawSnakeGame_Paged();
                break;
            case FLAPPY_BIRD_GAME:
                drawFlappyBirdGame_Paged();
                break;
            case TETRIS_GAME:
                drawTetrisGame_Paged();
                break;
        }
    } while (u8g2.nextPage());

    switch (currentGameState) {
        case MENU:
            handleMenuInput();
            break;
        case SNAKE_GAME:
            handleSnakeGameInputAndLogic();
            break;
        case FLAPPY_BIRD_GAME:
            handleFlappyBirdGameInputAndLogic();
            break;
        case TETRIS_GAME:
            handleTetrisGameInputAndLogic();
            break;
    }

    if (currentGameState == SNAKE_GAME && !snakeGameOver) {
        if (millis() - lastSnakeMoveTime > currentSnakeMoveInterval) {
            updateSnake();
            lastSnakeMoveTime = millis();
        }
    }
    if (currentGameState == TETRIS_GAME && !tetrisGameOver) {
        if (millis() - tetris_lastAutoDropTime > tetris_autoDropInterval) {
            if (!tetris_movePiece(0, 1)) {
                tetris_addPieceToPlayfield();
                tetris_clearLines();
                tetris_spawnNewPiece();
            }
            tetris_lastAutoDropTime = millis();
        }
    }
}

void drawMenuScreen_Paged() {
    u8g2.setFont(u8g2_font_7x13_tr);
    u8g2.drawStr(u8g2.getDisplayWidth() / 2 - u8g2.getStrWidth("GAME MENU") / 2, FONT_LINE_HEIGHT, "GAME MENU");
    u8g2.drawLine(0, FONT_LINE_HEIGHT + 3, SCREEN_WIDTH - 1, FONT_LINE_HEIGHT + 3);
    u8g2.setFont(u8g2_font_6x10_tr);
    for (int i = 0; i < numGames; i++) {
        int yP = FONT_LINE_HEIGHT * 2 + 5 + i * (FONT_LINE_HEIGHT + 2);
        if (i == selectedGame) {
            u8g2.drawStr(10, yP, "> ");
            u8g2.drawStr(10 + u8g2.getStrWidth("> "), yP, gameNames[i]);
        } else {
            u8g2.drawStr(10 + u8g2.getStrWidth("> "), yP, gameNames[i]);
        }
    }
}

void handleMenuInput() {
    JoystickState joy = readJoystick();
    if (millis() - lastMenuInputTime > menuInputDelay) {
        if (joy.down) {
            selectedGame = (selectedGame + 1) % numGames;
            lastMenuInputTime = millis();
        } else if (joy.up) {
            selectedGame = (selectedGame - 1 + numGames) % numGames;
            lastMenuInputTime = millis();
        }
    }
    if (checkAndDebounce(BTN_A_PIN)) {
        switch (selectedGame) {
            case 0:
                setupSnakeGame();
                currentGameState = SNAKE_GAME;
                break;
            case 1:
                setupFlappyBirdGame();
                currentGameState = FLAPPY_BIRD_GAME;
                break;
            case 2:
                setupTetrisGame();
                currentGameState = TETRIS_GAME;
                break;
        }
    }
}

void setupSnakeGame() {
    currentSnakeMoveInterval = normalSnakeMoveInterval;
    snakeLength = 3;
    snake[0] = {(byte)((SCREEN_WIDTH / SNAKE_GRID_SIZE) / 2), (byte)((SCREEN_HEIGHT / SNAKE_GRID_SIZE) / 2)};
    snake[1] = {(byte)(snake[0].x - 1), snake[0].y};
    snake[2] = {(byte)(snake[0].x - 2), snake[0].y};
    snakeDir = RIGHT;
    lastSnakeDir = RIGHT;
    snakeCurrentScore = 0;
    snakeGameOver = false;
    placeFood();
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_7x13_tr);
        u8g2.drawStr(u8g2.getDisplayWidth() / 2 - u8g2.getStrWidth("Get Ready!") / 2, u8g2.getDisplayHeight() / 2, "Get Ready!");
    } while (u8g2.nextPage());
    delay(1500);
    u8g2.setFont(u8g2_font_6x10_tr);
}

void placeFood() {
    bool coll;
    do {
        coll = false;
        food.x = random(0, SCREEN_WIDTH / SNAKE_GRID_SIZE);
        food.y = random(0, SCREEN_HEIGHT / SNAKE_GRID_SIZE);
        for (int i = 0; i < snakeLength; i++) {
            if (food.x == snake[i].x && food.y == snake[i].y) {
                coll = true;
                break;
            }
        }
    } while (coll);
}

void updateSnake() {
    Point nH = snake[0];
    switch (snakeDir) {
        case UP:    nH.y--; lastSnakeDir = UP;    break;
        case DOWN:  nH.y++; lastSnakeDir = DOWN;  break;
        case LEFT:  nH.x--; lastSnakeDir = LEFT;  break;
        case RIGHT: nH.x++; lastSnakeDir = RIGHT; break;
        case NONE:  return;
    }
    if (nH.x >= (SCREEN_WIDTH / SNAKE_GRID_SIZE) || nH.x < 0 || nH.y >= (SCREEN_HEIGHT / SNAKE_GRID_SIZE) || nH.y < 0) {
        snakeGameOver = true;
        return;
    }
    for (int i = 0; i < snakeLength; i++) {
        if (nH.x == snake[i].x && nH.y == snake[i].y) {
            snakeGameOver = true;
            return;
        }
    }
    for (int i = snakeLength - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    snake[0] = nH;
    if (nH.x == food.x && nH.y == food.y) {
        snakeCurrentScore++;
        if (snakeLength < MAX_SNAKE_LENGTH) {
            snakeLength++;
        }
        placeFood();
    }
    if (snakeCurrentScore > snakeMaxScore) {
        snakeMaxScore = snakeCurrentScore;
    }
}

void drawSnakeGame_Paged() {
    if (snakeGameOver) {
        u8g2.setFont(u8g2_font_helvB10_tr);
        int goW = u8g2.getStrWidth("GAME OVER");
        u8g2.drawStr(SCREEN_WIDTH / 2 - goW / 2, SCREEN_HEIGHT / 2 - FONT_LINE_HEIGHT - 3, "GAME OVER");
        u8g2.setFont(u8g2_font_6x10_tr);
        char scrStr[20];
        sprintf(scrStr, "Score: %d", snakeCurrentScore);
        u8g2.drawStr(SCREEN_WIDTH / 2 - u8g2.getStrWidth(scrStr) / 2, SCREEN_HEIGHT / 2 - 3, scrStr);
        sprintf(scrStr, "Max: %d", snakeMaxScore);
        u8g2.drawStr(SCREEN_WIDTH / 2 - u8g2.getStrWidth(scrStr) / 2, SCREEN_HEIGHT / 2 + SMALL_FONT_LINE_HEIGHT - 1, scrStr);
        u8g2.drawStr(10, SCREEN_HEIGHT - SMALL_FONT_LINE_HEIGHT - 10, "A:Retry");
        const char* mp = "B:Menu";
        u8g2.drawStr(SCREEN_WIDTH - u8g2.getStrWidth(mp) - 10, SCREEN_HEIGHT - SMALL_FONT_LINE_HEIGHT - 10, mp);
    } else {
        u8g2.drawFrame(SNAKE_PLAY_AREA_X_OFFSET, SNAKE_PLAY_AREA_Y_OFFSET, SNAKE_GRID_PIXEL_WIDTH, SNAKE_GRID_PIXEL_HEIGHT);

        for (int i = 0; i < snakeLength; i++) {
            u8g2.drawBox(SNAKE_PLAY_AREA_X_OFFSET + snake[i].x * SNAKE_GRID_SIZE, SNAKE_PLAY_AREA_Y_OFFSET + snake[i].y * SNAKE_GRID_SIZE, SNAKE_GRID_SIZE, SNAKE_GRID_SIZE);
        }
        u8g2.drawBox(SNAKE_PLAY_AREA_X_OFFSET + food.x * SNAKE_GRID_SIZE, SNAKE_PLAY_AREA_Y_OFFSET + food.y * SNAKE_GRID_SIZE, SNAKE_GRID_SIZE, SNAKE_GRID_SIZE);
        
        u8g2.setFont(u8g2_font_6x10_tr);
        u8g2.setCursor(SNAKE_PLAY_AREA_X_OFFSET + 2, SNAKE_PLAY_AREA_Y_OFFSET + SMALL_FONT_LINE_HEIGHT + 1);
        u8g2.print(F("S:"));
        u8g2.print(snakeCurrentScore);
        char maxScrB[10];
        sprintf(maxScrB, "Max:%d", snakeMaxScore);
        u8g2.drawStr(SNAKE_PLAY_AREA_X_OFFSET + SNAKE_GRID_PIXEL_WIDTH - u8g2.getStrWidth(maxScrB) - 2, SNAKE_PLAY_AREA_Y_OFFSET + SMALL_FONT_LINE_HEIGHT + 1, maxScrB);
    }
}

void handleSnakeGameInputAndLogic() {
    if (snakeGameOver) {
        bool btnP = false;
        currentSnakeMoveInterval = normalSnakeMoveInterval;
        while (!btnP) {
            if (checkAndDebounce(BTN_A_PIN)) {
                setupSnakeGame();
                btnP = true;
            } else if (checkAndDebounce(BTN_B_PIN)) {
                currentGameState = MENU;
                selectedGame = 0;
                btnP = true;
            }
            u8g2.firstPage();
            do {
                drawSnakeGame_Paged();
            } while (u8g2.nextPage());
            if (btnP) break;
            delay(10);
        }
        return;
    }
    JoystickState joy = readJoystick();
    if (joy.up && lastSnakeDir != DOWN) snakeDir = UP;
    else if (joy.down && lastSnakeDir != UP) snakeDir = DOWN;
    else if (joy.left && lastSnakeDir != RIGHT) snakeDir = LEFT;
    else if (joy.right && lastSnakeDir != LEFT) snakeDir = RIGHT;

    if (isButtonPressed(BTN_A_PIN)) {
        currentSnakeMoveInterval = fastSnakeMoveInterval;
    } else {
        currentSnakeMoveInterval = normalSnakeMoveInterval;
    }
    if (checkAndDebounce(BTN_B_PIN)) {
        currentSnakeMoveInterval = normalSnakeMoveInterval;
        currentGameState = MENU;
        selectedGame = 0;
        return;
    }
}

void setupFlappyBirdGame() {
    flappyBirdCurrentScore = 0;
    flappyBirdGameOver = false;
    flappy_bird_x = SCREEN_WIDTH / 4;
    flappy_bird_y = SCREEN_HEIGHT / 2;
    flappy_momentum = -4; // Initial flap
    flappy_wall_x[0] = SCREEN_WIDTH;
    flappy_wall_y[0] = random(flappy_wall_gap / 2 + 5, SCREEN_HEIGHT - flappy_wall_gap - (flappy_wall_gap / 2) - 5);
    flappy_wall_x[1] = SCREEN_WIDTH + SCREEN_WIDTH / 2 + flappy_wall_width / 2;
    flappy_wall_y[1] = random(flappy_wall_gap / 2 + 5, SCREEN_HEIGHT - flappy_wall_gap - (flappy_wall_gap / 2) - 5);
    flappy_lastUpdateTime = millis();
}

void drawFlappyBirdGame_Paged() {
    u8g2.setFontPosTop();
    if (flappyBirdGameOver) {
        u8g2.setFont(u8g2_font_helvB10_tr);
        int goW = u8g2.getStrWidth("GAME OVER");
        u8g2.drawStr(SCREEN_WIDTH / 2 - goW / 2, SCREEN_HEIGHT / 2 - FONT_LINE_HEIGHT - 3 - SMALL_FONT_LINE_HEIGHT, "GAME OVER");

        u8g2.setFont(u8g2_font_6x10_tr);
        char sStr[20];
        sprintf(sStr, "Score: %d", flappyBirdCurrentScore);
        u8g2.drawStr(SCREEN_WIDTH / 2 - u8g2.getStrWidth(sStr) / 2, SCREEN_HEIGHT / 2 - SMALL_FONT_LINE_HEIGHT, sStr);
        sprintf(sStr, "Max: %d", flappyBirdMaxScore);
        u8g2.drawStr(SCREEN_WIDTH / 2 - u8g2.getStrWidth(sStr) / 2, SCREEN_HEIGHT / 2 + 2, sStr);

        u8g2.drawStr(10, SCREEN_HEIGHT - SMALL_FONT_LINE_HEIGHT - 10, "A:Retry");
        const char* mp = "B:Menu";
        u8g2.drawStr(SCREEN_WIDTH - u8g2.getStrWidth(mp) - 10, SCREEN_HEIGHT - SMALL_FONT_LINE_HEIGHT - 10, mp);
    } else {
        u8g2.drawFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        u8g2.setDrawColor(1);
        const unsigned char* bSprite = (flappy_momentum < -1) ? (((millis() / 100) % 2 == 0) ? flappy_wing_down_bmp : flappy_wing_up_bmp) : flappy_wing_up_bmp;
        u8g2.drawXBMP(flappy_bird_x, flappy_bird_y, FLAPPY_SPRITE_WIDTH, FLAPPY_SPRITE_HEIGHT, bSprite);
        for (int i = 0; i < 2; i++) {
            u8g2.drawBox(flappy_wall_x[i], 0, flappy_wall_width, flappy_wall_y[i]);
            u8g2.drawBox(flappy_wall_x[i], flappy_wall_y[i] + flappy_wall_gap, flappy_wall_width, SCREEN_HEIGHT - (flappy_wall_y[i] + flappy_wall_gap));
        }

        char sBuf[10];
        sprintf(sBuf, "%d", flappyBirdCurrentScore);
        u8g2.setFont(FLAPPY_FONT_SMALL);
        game_textAt(2, 2, sBuf); // Score near top-left
    }
    u8g2.setFontPosBaseline();
}

void handleFlappyBirdGameInputAndLogic() {
    if (flappyBirdGameOver) {
        bool waitingForInput = true;
        while (waitingForInput && currentGameState == FLAPPY_BIRD_GAME && flappyBirdGameOver) {
            if (checkAndDebounce(BTN_A_PIN)) {
                setupFlappyBirdGame(); // This sets flappyBirdGameOver to false
                waitingForInput = false;
            } else if (checkAndDebounce(BTN_B_PIN)) {
                currentGameState = MENU;
                selectedGame = 1;
                waitingForInput = false;
            }
            if (waitingForInput) {
                u8g2.firstPage();
                do {
                    drawFlappyBirdGame_Paged();
                } while (u8g2.nextPage());
                delay(10);
            }
        }
        if (currentGameState != FLAPPY_BIRD_GAME || !flappyBirdGameOver) { // If retrying or switched state
            return;
        }
    } else { // Game is active
        if (checkAndDebounce(BTN_A_PIN)) {
            flappy_momentum = -4;
        }
        if (checkAndDebounce(BTN_B_PIN)) {
            currentGameState = MENU;
            selectedGame = 1;
            flappyBirdGameOver = true; // Set to game over for a clean state if re-entered
            return;
        }

        if (millis() - flappy_lastUpdateTime > FLAPPY_UPDATE_INTERVAL) {
            flappy_momentum += 1;
            if (flappy_momentum > 6) flappy_momentum = 6;
            flappy_bird_y += flappy_momentum;

            if (flappy_bird_y < 0) { // Hit top
                flappy_bird_y = 0;
                flappy_momentum = 0;
            }
            if (flappy_bird_y >= SCREEN_HEIGHT - FLAPPY_SPRITE_HEIGHT) { // Hit bottom
                flappy_bird_y = SCREEN_HEIGHT - FLAPPY_SPRITE_HEIGHT;
                flappyBirdGameOver = true;
                if (flappyBirdCurrentScore > flappyBirdMaxScore) flappyBirdMaxScore = flappyBirdCurrentScore;
                return;
            }

            for (int i = 0; i < 2; i++) {
                flappy_wall_x[i] -= 2;
                if (flappy_wall_x[i] + flappy_wall_width < 0) {
                    flappy_wall_y[i] = random(flappy_wall_gap / 2 + 5, SCREEN_HEIGHT - flappy_wall_gap - (flappy_wall_gap / 2) - 5);
                    flappy_wall_x[i] = SCREEN_WIDTH;
                }
                // Score
                if (flappy_bird_x > flappy_wall_x[i] + flappy_wall_width && flappy_bird_x - 2 <= flappy_wall_x[i] + flappy_wall_width) {
                    if (flappy_bird_x - 2 < flappy_wall_x[i] + flappy_wall_width + 2) {
                        flappyBirdCurrentScore++;
                        if (flappyBirdCurrentScore > flappyBirdMaxScore) flappyBirdMaxScore = flappyBirdCurrentScore;
                    }
                }
                // Collision
                if (flappy_bird_x + FLAPPY_SPRITE_WIDTH - 2 > flappy_wall_x[i] && flappy_bird_x < flappy_wall_x[i] + flappy_wall_width) {
                    if (flappy_bird_y < flappy_wall_y[i] || flappy_bird_y + FLAPPY_SPRITE_HEIGHT - 2 > flappy_wall_y[i] + flappy_wall_gap) {
                        flappyBirdGameOver = true;
                        if (flappyBirdCurrentScore > flappyBirdMaxScore) flappyBirdMaxScore = flappyBirdCurrentScore;
                        break; 
                    }
                }
            }
            flappy_lastUpdateTime = millis();
        }
    }
}

void setupTetrisGame() {
    tetrisCurrentScore = 0;
    tetrisPiecesPlaced = 0;
    tetrisGameOver = false;
    tetris_currentLevel = 0;
    tetris_linesClearedThisLevel = 0;
    tetris_autoDropInterval = 800;
    for (int r = 0; r < TETRIS_GRID_HEIGHT; r++) for (int c = 0; c < TETRIS_GRID_WIDTH; c++) tetrisPlayfield[r][c] = 0;
    tetrisNextBlockType = random(1, 8);
    tetris_spawnNewPiece();
    if (tetrisGameOver) tetrisPiecesPlaced = 0;
    tetris_lastAutoDropTime = millis();
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_7x13_tr);
        u8g2.drawStr(u8g2.getDisplayWidth() / 2 - u8g2.getStrWidth("Get Ready!") / 2, u8g2.getDisplayHeight() / 2, "Get Ready!");
    } while (u8g2.nextPage());
    delay(1500);
    u8g2.setFont(u8g2_font_6x10_tr);
}

void drawTetrisGame_Paged() {
    u8g2.setFont(u8g2_font_6x10_tr);
    if (tetrisGameOver) {
        u8g2.setFont(u8g2_font_helvB10_tr);
        int goW = u8g2.getStrWidth("GAME OVER");
        u8g2.drawStr(SCREEN_WIDTH / 2 - goW / 2, SCREEN_HEIGHT / 2 - FONT_LINE_HEIGHT - 3 - SMALL_FONT_LINE_HEIGHT / 2, "GAME OVER");
        u8g2.setFont(u8g2_font_6x10_tr);
        char scrStr[20];
        sprintf(scrStr, "Score: %d", tetrisCurrentScore);
        u8g2.drawStr(SCREEN_WIDTH / 2 - u8g2.getStrWidth(scrStr) / 2, SCREEN_HEIGHT / 2 - SMALL_FONT_LINE_HEIGHT / 2, scrStr);
        sprintf(scrStr, "Max: %d", tetrisMaxScore);
        u8g2.drawStr(SCREEN_WIDTH / 2 - u8g2.getStrWidth(scrStr) / 2, SCREEN_HEIGHT / 2 + SMALL_FONT_LINE_HEIGHT - SMALL_FONT_LINE_HEIGHT / 2 + 2, scrStr);
        sprintf(scrStr, "Pieces: %d", tetrisPiecesPlaced);
        u8g2.drawStr(SCREEN_WIDTH / 2 - u8g2.getStrWidth(scrStr) / 2, SCREEN_HEIGHT / 2 + SMALL_FONT_LINE_HEIGHT * 2 - SMALL_FONT_LINE_HEIGHT / 2 + 4, scrStr);
        u8g2.drawStr(10, SCREEN_HEIGHT - SMALL_FONT_LINE_HEIGHT - 10, "A:Retry");
        const char* mp = "B:Menu";
        u8g2.drawStr(SCREEN_WIDTH - u8g2.getStrWidth(mp) - 10, SCREEN_HEIGHT - SMALL_FONT_LINE_HEIGHT - 10, mp);
    } else {
        u8g2.drawFrame(TETRIS_PLAYFIELD_X_OFFSET - 1, TETRIS_PLAYFIELD_Y_OFFSET - 1, TETRIS_GRID_WIDTH * TETRIS_BLOCK_SIZE + 2, TETRIS_GRID_HEIGHT * TETRIS_BLOCK_SIZE + 2);
        for (int r = 0; r < TETRIS_GRID_HEIGHT; r++) for (int c = 0; c < TETRIS_GRID_WIDTH; c++) if (tetrisPlayfield[r][c]) u8g2.drawBox(TETRIS_PLAYFIELD_X_OFFSET + c * TETRIS_BLOCK_SIZE, TETRIS_PLAYFIELD_Y_OFFSET + r * TETRIS_BLOCK_SIZE, TETRIS_BLOCK_SIZE, TETRIS_BLOCK_SIZE);
        for (int rs = 0; rs < 4; rs++) for (int cs = 0; cs < 4; cs++) if (tetrisCurrentBlock.shape[rs][cs]) u8g2.drawBox(TETRIS_PLAYFIELD_X_OFFSET + (tetrisCurrentBlock.gridX + cs) * TETRIS_BLOCK_SIZE, TETRIS_PLAYFIELD_Y_OFFSET + (tetrisCurrentBlock.gridY + rs) * TETRIS_BLOCK_SIZE, TETRIS_BLOCK_SIZE, TETRIS_BLOCK_SIZE);
        int stX = TETRIS_PLAYFIELD_X_OFFSET + TETRIS_GRID_WIDTH * TETRIS_BLOCK_SIZE + 5;
        u8g2.setCursor(stX, TETRIS_PLAYFIELD_Y_OFFSET + SMALL_FONT_LINE_HEIGHT);
        u8g2.print(F("S:"));
        u8g2.print(tetrisCurrentScore);
        u8g2.setCursor(stX, TETRIS_PLAYFIELD_Y_OFFSET + SMALL_FONT_LINE_HEIGHT * 2 + 2);
        u8g2.print(F("L:"));
        u8g2.print(tetris_currentLevel);
        u8g2.setCursor(stX, TETRIS_PLAYFIELD_Y_OFFSET + SMALL_FONT_LINE_HEIGHT * 3 + 4);
        u8g2.print(F("P:"));
        u8g2.print(tetrisPiecesPlaced);
        u8g2.setCursor(stX, TETRIS_PLAYFIELD_Y_OFFSET + SMALL_FONT_LINE_HEIGHT * 4 + 6);
        u8g2.print(F("Nxt:"));
        byte nS[4][4];
        tetris_getShape(tetrisNextBlockType, nS);
        int pX = stX;
        int pY = TETRIS_PLAYFIELD_Y_OFFSET + SMALL_FONT_LINE_HEIGHT * 5 + 8;
        for (int r = 0; r < 4; r++) for (int c = 0; c < 4; c++) if (nS[r][c]) u8g2.drawBox(pX + c * TETRIS_BLOCK_SIZE, pY + r * TETRIS_BLOCK_SIZE, TETRIS_BLOCK_SIZE, TETRIS_BLOCK_SIZE);
    }
}

void handleTetrisGameInputAndLogic() {
    if (tetrisGameOver) {
        bool wait = true;
        while (wait) {
            if (checkAndDebounce(BTN_A_PIN)) {
                setupTetrisGame();
                wait = false;
            } else if (checkAndDebounce(BTN_B_PIN)) {
                currentGameState = MENU;
                selectedGame = 2;
                wait = false;
            }
            u8g2.firstPage();
            do {
                drawTetrisGame_Paged();
            } while (u8g2.nextPage());
            if (!wait) break;
            delay(10);
        }
        return;
    }
    JoystickState joy = readJoystick();
    static unsigned long lastJoyIT = 0;
    const int joyID = 100;
    if (millis() - lastJoyIT > joyID) {
        if (joy.left) {
            tetris_movePiece(-1, 0);
            lastJoyIT = millis();
        } else if (joy.right) {
            tetris_movePiece(1, 0);
            lastJoyIT = millis();
        } else if (joy.down) {
            if (tetris_movePiece(0, 1)) {
                tetris_lastAutoDropTime = millis();
            } else {
                tetris_addPieceToPlayfield();
                tetris_clearLines();
                tetris_spawnNewPiece();
            }
            lastJoyIT = millis();
        }
    }
    if (checkAndDebounce(BTN_A_PIN)) tetris_rotatePiece();
    if (checkAndDebounce(BTN_B_PIN)) {
        currentGameState = MENU;
        selectedGame = 2;
        return;
    }
}