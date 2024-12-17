/**
  ******************************************************************************
  * @file    superSnake.h
  * @author  Andrew Dile, Abhi Annigeri, Daniel Wang, William Lee
  * @date    November 4 2024
  * @brief   ECE 362 Snake Game
  ******************************************************************************
*/

#include "stm32f0xx.h"
#include <stdint.h>
#include <stdio.h>
#include "fifo.h"
#include "tty.h"
#include "commands.h"
#include "helper.h"

// display is 240x320 pixels

//////////// PIN MAP ////////////
// 
//  PA1  as ADC1 -- JoystickX
//  PA2  as ADC2 -- JoystickY
//  PA3  as GPIO -- JoystickSw
//  PA5 OLED pin 12 SCL->SPI
//  PA7 pin 14 SDI->SPI
//  PA15 pin 16 nCS ->SPI
//  
//  PB12  as SPI2 -- NSS
//  PB13  as SPI2 -- SCK
//  PB15  as SPI2 -- MOSI
//
//  Using Lab7 Pin Schematic For
//    TFT Display and SD Card
//  UART also connected like past
//    labs
//
/////////////////////////////////

// global includes
#include "stm32f0xx.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// number of cells on the gameboard, can be changed but will drastically affect memory game takes to play
#define NUM_X_CELLS 20
#define NUM_Y_CELLS 20

// display constants
#define X_PIXELS 240  // 0 is left
#define Y_PIXELS 320  // 0 is top
#define CELL_PIXEL_WIDTH (X_PIXELS / NUM_X_CELLS)
#define X_BORDER 0
#define Y_BORDER ((Y_PIXELS - X_PIXELS) / 2)

// color hex code constants
extern uint16_t white;
extern uint16_t black;
extern uint16_t red;
extern uint16_t green;
extern uint16_t blue ;
extern uint16_t yellow;
extern uint16_t purple;
extern uint16_t orange;

// direction constants for code readability and standardization
#define NEUTRAL 0
#define RIGHT 1
#define LEFT 2
#define UP 3
#define DOWN 4

// game state constants for code readability and standardization
#define IDLE 0
#define RUNNING 1
#define GAMELOST 2
#define GAMEWON 3

// constants for tile pieces
#define EMPTY 0
#define SNACK 1
#define HEAD_LEFT 2
#define HEAD_RIGHT 3
#define HEAD_UP 4
#define HEAD_DOWN 5
#define SEGMENT_VER 6
#define SEGMENT_HOR 7
#define BEND_UP_RIGHT 8  // directions signify where the cell is connected to the snake segments (BEND_UP_RIGHT means the segment connects to a segments above and to the right)
#define BEND_UP_LEFT 9
#define BEND_DOWN_RIGHT 10
#define BEND_DOWN_LEFT 11
#define BEND_RIGHT_UP BEND_UP_RIGHT
#define BEND_RIGHT_DOWN BEND_DOWN_RIGHT
#define BEND_LEFT_UP BEND_UP_LEFT
#define BEND_LEFT_DOWN BEND_DOWN_LEFT
#define TAIL_LEFT 12
#define TAIL_RIGHT 13
#define TAIL_UP 14
#define TAIL_DOWN 15

// constants for audio
#define EAT 0
#define LOST 1
#define WON 2
#define ABILITY_USED 3

// constants for adc
#define THRESHOLD_LOW 1000  // ADC RANGE ADJUSTMENTS NEEDED
#define THRESHOLD_HIGH 3000
#define BCSIZE 16
extern char direction;

// constants for snake and movement
#define INITIAL_SNAKE_SPEED 500  // milliseconds between speed
#define INITIAL_SNAKE_LENGTH 3  // segments including head
#define MAX_SNAKE_LENGTH (NUM_X_CELLS * NUM_Y_CELLS)
#define MIN_SNAKE_SPEED 100      // maximum speed (minimum delay)
#define SPEED_INCREASE 25        // ms faster per snack eaten

// constants for ability (more abilities probably need to be added)
#define NONE 0
#define INCREASE_SPEED 1
#define DECREASE_SPEED 2
#define PAUSE 3

#define PWM_MAX 2400

// constants for OLED messages
#define WELCOME 0
#define SCORE 1
#define HIGHS 2
#define TEST 3
extern FATFS fs_storage;
extern int8_t button;

// each segment of the snake needs a position and a direction value
struct segment {
  int8_t x; // horizontal gametile position -- left = 0
  int8_t y; // vertical gametile position -- bottom = 0
  uint8_t direction; // direction current segment (or head) is facing
};

struct tile {
  int8_t x;
  int8_t y;
};

// defines the structure as a data type, for easier later use
typedef struct segment segment;
typedef struct tile tile;

// exteral variables declared so other files to access
extern int8_t gameState;  // game over can be derived from this
extern int8_t lastGameState;
extern int8_t gameboard[NUM_X_CELLS][NUM_Y_CELLS];
extern segment snake[NUM_X_CELLS * NUM_Y_CELLS];
extern int8_t snakeLength; // current score can be derived from this
extern uint32_t snakeSpeed;
extern int8_t joystickDirection;
extern uint32_t joystickXraw;
extern int8_t joystickYraw;
extern int8_t highscore1;
extern int8_t highscore2;
extern int8_t highscore3;

// function declarations
void setupLCDDisplay();
void updateLCDDisplay();
void setupJoystick();
void updateJoystick();
void setupDMA();
void enableDMA();
void initializeSnake();
void setupMovementTimer();
void generateSnack();
void movementLogic();
void gameStateHandler();
void playSound(uint8_t);
void setup_tim1();
void ateSnack();
void goFaster();

// oled functions

void init_spi1(void);
void spi1_init_oled(void);
void spi_cmd(unsigned int data);
void spi_data(unsigned int data);
void oled_write_string(const char* str, int line);
void update_oled_display(void);
void spi_cmd();
void spi1_dma_display1(const char*);
void spi1_dma_display2(const char*);
void spi1_setup_dma();
void spi1_enable_dma();
void setupOLED();
void updateOLED();
void setup_adc();
void init_tim2();

// sd card functions
void writeHighScoresToSD();
void readHighScoresFromSD();
void mountSD();



void setup_tim7(void);
void TIM7_IRQHandler(void);
void play_sound_effect(uint8_t effect);
void playSound(uint8_t song);


#define SOUND_DURATION 200 // ms
#define EAT_FREQ 999    // Hz
#define WIN_FREQ 799    
#define LOSE_FREQ 1199

void test_sounds();

void setup_tim7(void);
void play_sound_effect(uint8_t effect);