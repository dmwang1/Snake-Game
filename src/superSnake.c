/**
 ******************************************************************************
 * @file    superSnake.c
 * @author  Andrew Dile, Abhi Annigeri, Daniel Wang, William Lee
 * @date    November 4 2024
 * @brief   ECE 362 Snake Game
 ******************************************************************************
 */

#include "superSnake.h"
#include "fifo.h"
#include "tty.h"

// variables for game state handling
int8_t gameState = IDLE;
int8_t lastGameState = IDLE;

// gameboard is 2D array of gametile structures, value of tile updated on snake movement
int8_t gameboard[NUM_X_CELLS][NUM_Y_CELLS] = {0}; // value determines what is displayed in that cell (0 = blank, 1 = snack, 2 = head facing left, etc.)

// array to store snake
segment snake[NUM_X_CELLS * NUM_Y_CELLS]; // not dynamically stored to avoid fragmentation and leaks, and to ensure program doesn't run out of space during gameplay

// snake movement variables
int8_t snakeLength = 3;
uint32_t snakeSpeed = INITIAL_SNAKE_SPEED;

// variable for joystick direction
int8_t joystickDirection = NEUTRAL;
uint32_t joystickXraw = -1;
int8_t joystickYraw = -1;

// variable for ability
int8_t ability = NONE;

// color values
uint16_t white = 65535;
uint16_t black = 0;
uint16_t red = 63488;
uint16_t green = 2016;
uint16_t blue = 31;
uint16_t yellow = 65504;
uint16_t purple = 30735;
uint16_t orange = 64512;
uint16_t uhh = 64300;

// variables for high scores
int8_t highscore1 = 30;
int8_t highscore2 = 25;
int8_t highscore3 = 5;
FATFS fs_storage;

int8_t newJoystickReading = 0;
int8_t button = 0;

int8_t lastHeadDirection = UP;

// set up LCD display to be communicated with
void setupLCDDisplay() {
  LCD_Setup();
  LCD_Clear(white);
  LCD_DrawFillRectangle(0, Y_BORDER, X_PIXELS, Y_PIXELS - Y_BORDER, black);
}

// function that updates LCD display
void updateLCDDisplay() {
  for (int x = 0; x < NUM_X_CELLS; x++) {
    for (int y = 0; y < NUM_Y_CELLS; y++) {
      switch (gameboard[x][y]) {
      case EMPTY:
        LCD_DrawFillRectangle((CELL_PIXEL_WIDTH * x), Y_BORDER + (CELL_PIXEL_WIDTH * y), ((CELL_PIXEL_WIDTH * x) + CELL_PIXEL_WIDTH), (Y_BORDER + (CELL_PIXEL_WIDTH * y) + CELL_PIXEL_WIDTH), black);
        break;
      case SNACK:
        LCD_DrawFillRectangle((CELL_PIXEL_WIDTH * x) + 2, Y_BORDER + (CELL_PIXEL_WIDTH * y) + 2, ((CELL_PIXEL_WIDTH * x) + CELL_PIXEL_WIDTH) - 4, (Y_BORDER + (CELL_PIXEL_WIDTH * y) + CELL_PIXEL_WIDTH) - 4, green);
        break;
      case HEAD_LEFT:
        LCD_DrawFillRectangle((CELL_PIXEL_WIDTH * x), Y_BORDER + (CELL_PIXEL_WIDTH * y), ((CELL_PIXEL_WIDTH * x) + CELL_PIXEL_WIDTH), (Y_BORDER + (CELL_PIXEL_WIDTH * y) + CELL_PIXEL_WIDTH), purple);
        break;
      case HEAD_RIGHT:
        LCD_DrawFillRectangle((CELL_PIXEL_WIDTH * x), Y_BORDER + (CELL_PIXEL_WIDTH * y), ((CELL_PIXEL_WIDTH * x) + CELL_PIXEL_WIDTH), (Y_BORDER + (CELL_PIXEL_WIDTH * y) + CELL_PIXEL_WIDTH), purple);
        break;
      case HEAD_UP:
        LCD_DrawFillRectangle((CELL_PIXEL_WIDTH * x), Y_BORDER + (CELL_PIXEL_WIDTH * y), ((CELL_PIXEL_WIDTH * x) + CELL_PIXEL_WIDTH), (Y_BORDER + (CELL_PIXEL_WIDTH * y) + CELL_PIXEL_WIDTH), purple);
        break;
      case HEAD_DOWN:
        LCD_DrawFillRectangle((CELL_PIXEL_WIDTH * x), Y_BORDER + (CELL_PIXEL_WIDTH * y), ((CELL_PIXEL_WIDTH * x) + CELL_PIXEL_WIDTH), (Y_BORDER + (CELL_PIXEL_WIDTH * y) + CELL_PIXEL_WIDTH), purple);
        break;
      case SEGMENT_VER:
        LCD_DrawFillRectangle((CELL_PIXEL_WIDTH * x), Y_BORDER + (CELL_PIXEL_WIDTH * y), ((CELL_PIXEL_WIDTH * x) + CELL_PIXEL_WIDTH), (Y_BORDER + (CELL_PIXEL_WIDTH * y) + CELL_PIXEL_WIDTH), uhh);
        break;
      case SEGMENT_HOR:
        LCD_DrawFillRectangle((CELL_PIXEL_WIDTH * x), Y_BORDER + (CELL_PIXEL_WIDTH * y), ((CELL_PIXEL_WIDTH * x) + CELL_PIXEL_WIDTH), (Y_BORDER + (CELL_PIXEL_WIDTH * y) + CELL_PIXEL_WIDTH), uhh);
        break;
      case BEND_UP_RIGHT:
        LCD_DrawFillRectangle((CELL_PIXEL_WIDTH * x), Y_BORDER + (CELL_PIXEL_WIDTH * y), ((CELL_PIXEL_WIDTH * x) + CELL_PIXEL_WIDTH), (Y_BORDER + (CELL_PIXEL_WIDTH * y) + CELL_PIXEL_WIDTH), uhh);
        break;
      case BEND_UP_LEFT:
        LCD_DrawFillRectangle((CELL_PIXEL_WIDTH * x), Y_BORDER + (CELL_PIXEL_WIDTH * y), ((CELL_PIXEL_WIDTH * x) + CELL_PIXEL_WIDTH), (Y_BORDER + (CELL_PIXEL_WIDTH * y) + CELL_PIXEL_WIDTH), uhh);
        break;
      case BEND_DOWN_RIGHT:
        LCD_DrawFillRectangle((CELL_PIXEL_WIDTH * x), Y_BORDER + (CELL_PIXEL_WIDTH * y), ((CELL_PIXEL_WIDTH * x) + CELL_PIXEL_WIDTH), (Y_BORDER + (CELL_PIXEL_WIDTH * y) + CELL_PIXEL_WIDTH), uhh);
        break;
      case BEND_DOWN_LEFT:
        LCD_DrawFillRectangle((CELL_PIXEL_WIDTH * x), Y_BORDER + (CELL_PIXEL_WIDTH * y), ((CELL_PIXEL_WIDTH * x) + CELL_PIXEL_WIDTH), (Y_BORDER + (CELL_PIXEL_WIDTH * y) + CELL_PIXEL_WIDTH), uhh);
        break;
      case TAIL_UP:
        LCD_DrawFillRectangle((CELL_PIXEL_WIDTH * x), Y_BORDER + (CELL_PIXEL_WIDTH * y), ((CELL_PIXEL_WIDTH * x) + CELL_PIXEL_WIDTH), (Y_BORDER + (CELL_PIXEL_WIDTH * y) + CELL_PIXEL_WIDTH), uhh);
        break;
      case TAIL_DOWN:
        LCD_DrawFillRectangle((CELL_PIXEL_WIDTH * x), Y_BORDER + (CELL_PIXEL_WIDTH * y), ((CELL_PIXEL_WIDTH * x) + CELL_PIXEL_WIDTH), (Y_BORDER + (CELL_PIXEL_WIDTH * y) + CELL_PIXEL_WIDTH), uhh);
        break;
      case TAIL_RIGHT:
        LCD_DrawFillRectangle((CELL_PIXEL_WIDTH * x), Y_BORDER + (CELL_PIXEL_WIDTH * y), ((CELL_PIXEL_WIDTH * x) + CELL_PIXEL_WIDTH), (Y_BORDER + (CELL_PIXEL_WIDTH * y) + CELL_PIXEL_WIDTH), uhh);
        break;
      case TAIL_LEFT:
        LCD_DrawFillRectangle((CELL_PIXEL_WIDTH * x), Y_BORDER + (CELL_PIXEL_WIDTH * y), ((CELL_PIXEL_WIDTH * x) + CELL_PIXEL_WIDTH), (Y_BORDER + (CELL_PIXEL_WIDTH * y) + CELL_PIXEL_WIDTH), uhh);
        break;
      default:
        break;
      }
    }
  }
}

// void init_gpio() {
//     RCC->AHBENR |= (RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN);
//     GPIOA->MODER |= (GPIO_MODER_MODER0 | GPIO_MODER_MODER1); // Joystick PA0, PA1 as analog
//     GPIOB->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1); // LED PB0, PB1 clear mode
//     GPIOB->MODER |= (GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0); // Set PB0, PB1 as output
//     GPIOB->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR0 | GPIO_OSPEEDER_OSPEEDR1); // High speed for LEDs
//     GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR0 | GPIO_PUPDR_PUPDR1); // No pull-up or pull-down for LEDs
// }

// function to initialize ADC for joystick readings

volatile uint16_t x_axis_value = 0;

void setup_adc(void) {
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN; // Enable GPIOA clock
  GPIOA->MODER |= GPIO_MODER_MODER1; // Set PA1 to analog mode
  RCC->APB2ENR |= RCC_APB2ENR_ADCEN; // Enable ADC clock
  RCC->CR2 |= RCC_CR2_HSI14ON;       // Enable HSI14 clock
  while (!(RCC->CR2 & RCC_CR2_HSI14RDY)); // Wait for HSI14 to be ready
  ADC1->CR |= ADC_CR_ADEN;           // Enable ADC
  while (!(ADC1->ISR & ADC_ISR_ADRDY)); // Wait for ADC ready
  //while ((ADC1->CR & ADC_CR_ADSTART)); // Ensure no ongoing conversion
  ADC1->CHSELR = ADC_CHSELR_CHSEL1;  // Select channel 1 (PA1)
  // while (ADC1->ISR & ADC_ISR_EOC);   // Clear any pending EOC
  while (!(ADC1->ISR & ADC_ISR_ADRDY)); // Wait until ready
}

void TIM2_IRQHandler(void) {
  TIM2->SR &= ~TIM_SR_UIF; // Clear interrupt flag
  ADC1->CR |= ADC_CR_ADSTART; // Start ADC conversion
  while (!(ADC1->ISR & ADC_ISR_EOC)); // Wait for conversion complete
  uint32_t ADCReading = ADC1->DR;
  joystickXraw = ADCReading; // Read ADC value
  newJoystickReading = 1;
}

void init_tim2(void) {
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Enable TIM2 clock
  TIM2->PSC = 48000 - 1;             // Prescaler for 1ms tick
  TIM2->ARR = 100 - 1;               // Update period = 100ms
  TIM2->DIER |= TIM_DIER_UIE;        // Enable update interrupt
  NVIC_EnableIRQ(TIM2_IRQn);         // Enable TIM2 interrupt in NVIC
  NVIC_SetPriority(TIM2_IRQn, 38);
  TIM2->CR1 |= TIM_CR1_CEN;          // Start timer
}

void updateJoystick() {
  if (newJoystickReading == 1) {
    if (joystickXraw > 3000) {
      joystickDirection = RIGHT;
    } else if (joystickXraw > 1000) {
      joystickDirection = NEUTRAL;
    } else {
      joystickDirection = LEFT;
    }

    if (gameState == RUNNING) {
      switch (lastHeadDirection) {
        case RIGHT:
          if (joystickDirection == RIGHT) {
            snake[0].direction = DOWN;
          }
          if (joystickDirection == LEFT) {
            snake[0].direction = UP;
          }
          break;
        case LEFT:
          if (joystickDirection == RIGHT) {
            snake[0].direction = UP;
          }
          if (joystickDirection == LEFT) {
            snake[0].direction = DOWN;
          }
          break;
        case UP:
          if (joystickDirection == RIGHT) {
            snake[0].direction = RIGHT;
          }
          if (joystickDirection == LEFT) {
            snake[0].direction = LEFT;
          }
          break;
        case DOWN:
          if (joystickDirection == RIGHT) {
            snake[0].direction = LEFT;
          }
          if (joystickDirection == LEFT) {
            snake[0].direction = RIGHT;
          }
          break;
      } 
    }

    newJoystickReading = 0;
  }

  button = GPIOA->IDR & (1 << 3);
}

void initializeSnake() {
  // set position and direction of snake head and first two segments
  snake[0].x = NUM_X_CELLS / 2;
  snake[0].y = NUM_Y_CELLS / 2;
  snake[0].direction = UP;
  snake[1].x = snake[0].x;
  snake[1].y = snake[0].y + 1;
  snake[1].direction = snake[0].direction;
  snake[2].x = snake[0].x;
  snake[2].y = snake[1].y + 1;
  snake[2].direction = snake[0].direction;

  gameboard[snake[0].x][snake[0].y] = HEAD_UP;
  gameboard[snake[1].x][snake[1].y] = SEGMENT_VER;
  gameboard[snake[2].x][snake[2].y] = TAIL_UP;
  
  // Generate first snack
  srand(joystickXraw);
  generateSnack();
}

void setupMovementTimer() {
  // Enable TIM3 clock
  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
  
  // Set prescaler for 1ms resolution
  TIM3->PSC = 47999;  // 48MHz/48000 = 1kHz
  
  // Set initial period to snakeSpeed (in ms)
  TIM3->ARR = snakeSpeed;
  
  // Enable update interrupt
  TIM3->DIER |= TIM_DIER_UIE;
  
  // Enable timer
  TIM3->CR1 |= TIM_CR1_CEN;
  
  // Enable TIM3 interrupt in NVIC
  NVIC_EnableIRQ(TIM3_IRQn);
}

void generateSnack() {
  // tile clears[NUM_X_CELLS] = {0};
  // int8_t clearNum = 0;

  // for (int x = 0; x < NUM_X_CELLS; x++) {
  //   for (int y = 0; y < NUM_Y_CELLS; y++) {
  //     if (gameboard[x][y] == EMPTY) {
  //       clears[clearNum].x = x;
  //       clears[clearNum].y = y;
  //       clearNum++;
  //     }
  //   }
  // }

  // uint8_t randomIndex = rand() % clearNum;
  // gameboard[clears[randomIndex].x][clears[randomIndex].y] = SNACK;

  // Generate new snack at random position
  // Create arrays to store empty positions
  uint8_t emptyX[NUM_X_CELLS * NUM_Y_CELLS];
  uint8_t emptyY[NUM_X_CELLS * NUM_Y_CELLS];
  uint16_t emptyCount = 0;
  
  // Find all empty positions on the gameboard
  for(uint8_t x = 0; x < NUM_X_CELLS; x++) {
    for(uint8_t y = 0; y < NUM_Y_CELLS; y++) {
      if(gameboard[x][y] == EMPTY) {
        emptyX[emptyCount] = x;
        emptyY[emptyCount] = y;
        emptyCount++;
      }
    }
  }
    
  // Check if there are any empty spaces
  if(emptyCount == 0) {
    // Game is won - no empty spaces left
    gameState = GAMEWON;
    return;
  }
  
  // Generate random index from available empty positions
  uint16_t randomIndex = rand() % emptyCount;
  
  // Get the randomly selected empty position
  uint8_t newSnackX = emptyX[randomIndex];
  uint8_t newSnackY = emptyY[randomIndex];
  
  // Place snack on gameboard
  gameboard[newSnackX][newSnackY] = SNACK;
}

void movementLogic() {
  // Return if game isn't running
  if(gameState != RUNNING) return;

  // variable to keep track of last directions
  int8_t lastDirection;
  int8_t lastX;
  int8_t lastY;

  for (int i = 0; i <= snakeLength && snakeLength < NUM_X_CELLS * NUM_Y_CELLS; i++) {
    // snake head logic
    if (i == 0) {
      // save last head direction
      lastHeadDirection = snake[i].direction;

      // save last position before updating it
      lastX = snake[i].x;
      lastY = snake[i].y;
      
      // update position, detect border collision, detect eating, detect winning, update gameboard
      switch (snake[0].direction) {
        case UP:
          snake[0].y--;
          if (snake[0].y < 0 || snake[0].y >= NUM_Y_CELLS) {  // border collision
            gameState = GAMELOST;
            playSound(LOST);
            return;
          }
          if (gameboard[snake[0].x][snake[0].y] == SNACK) {  // eating detection
            snakeLength++;
            if (snakeLength >= NUM_X_CELLS * NUM_Y_CELLS) {  // winning detection
              gameState = GAMEWON;
              playSound(WON);
              return;
            }
            goFaster();
            ateSnack();  // possibly obtain an ability
            playSound(EAT);
            generateSnack();  // generate new snack
          }
          gameboard[snake[0].x][snake[0].y] = HEAD_UP; // update gameboard
          break;
        case DOWN:
          snake[0].y++;
          if (snake[0].y < 0 || snake[0].y >= NUM_Y_CELLS) {  // border collision
            gameState = GAMELOST;
            playSound(LOST);
            return;
          }
          if (gameboard[snake[0].x][snake[0].y] == SNACK) {  // eating detection
            snakeLength++;
            if (snakeLength >= NUM_X_CELLS * NUM_Y_CELLS) {  // winning detection
              gameState = GAMEWON;
              playSound(WON);
              return;
            }
            goFaster();
            ateSnack();  // possibly obtain an ability
            playSound(EAT);
            generateSnack();  // generate new snack
          }
          gameboard[snake[0].x][snake[0].y] = HEAD_DOWN; // update gameboard
          break;
        case LEFT:
          snake[0].x--;
          if (snake[0].x < 0 || snake[0].x >= NUM_X_CELLS) {  // border collision
            gameState = GAMELOST;
            playSound(LOST);
            return;
          }
          if (gameboard[snake[0].x][snake[0].y] == SNACK) {  // eating detection
            snakeLength++;
            if (snakeLength >= NUM_X_CELLS * NUM_Y_CELLS) {  // winning detection
              gameState = GAMEWON;
              playSound(WON);
              return;
            }
            goFaster();
            ateSnack();  // possibly obtain an ability
            playSound(EAT);
            generateSnack();  // generate new snack
          }
          gameboard[snake[0].x][snake[0].y] = HEAD_LEFT; // update gameboard
          break;
        case RIGHT:
          snake[0].x++;
          if (snake[0].x < 0 || snake[0].x >= NUM_X_CELLS) {  // border collision
            gameState = GAMELOST;
            playSound(LOST);
            return;
          }
          if (gameboard[snake[0].x][snake[0].y] == SNACK) {  // eating detection
            snakeLength++;
            if (snakeLength >= NUM_X_CELLS * NUM_Y_CELLS) {  // winning detection
              gameState = GAMEWON;
              playSound(WON);
              return;
            }
            goFaster();
            ateSnack();  // possibly obtain an ability
            playSound(EAT);
            generateSnack();  // generate new snack
          }
          gameboard[snake[0].x][snake[0].y] = HEAD_RIGHT; // update gameboard
          break;
      }
    }

    // snake segment logic
    else if (i > 0 && i < snakeLength - 1) {
      // if new segment, initialize position and direction based on the last values of next segment
      if (snake[i].direction == NEUTRAL) {
        snake[i].x = lastX;
        snake[i].y = lastY;
        snake[i].direction = lastDirection;
      }
      
      // save last position before updating it
      int8_t tempX = lastX;
      int8_t tempY = lastY;
      lastX = snake[i].x;
      lastY = snake[i].y;

      // update position
      snake[i].x = tempX;
      snake[i].y = tempY;

      // save last direction before updating it
      lastDirection = snake[i].direction;

      // update direction
      int diffx = snake[1].x - snake[i - 1].x;
      int diffy = snake[1].y - snake[i - 1].y;

      if (diffx < 0) {
        snake[i].direction = RIGHT;
      } else if (diffx > 0) {
        snake[i].direction = LEFT;
      } else if (diffy < 0) {
        snake[i].direction = DOWN;
      } else if (diffy > 0) {
        snake[i].direction = UP;
      }

      // update gameboard
      switch (lastDirection) {
        case UP:
          switch (snake[i].direction) {
            case UP:
              gameboard[snake[i].x][snake[i].y] = SEGMENT_VER;
              break;
            case RIGHT:
              gameboard[snake[i].x][snake[i].y] = BEND_RIGHT_DOWN;
              break;
            case LEFT:
              gameboard[snake[i].x][snake[i].y] = BEND_LEFT_DOWN;
              break;
            default:
              break;
          }
          break;
        case DOWN:
          switch (snake[i].direction) {
            case DOWN:
              gameboard[snake[i].x][snake[i].y] = SEGMENT_VER;
              break;
            case RIGHT:
              gameboard[snake[i].x][snake[i].y] = BEND_RIGHT_UP;
              break;
            case LEFT:
              gameboard[snake[i].x][snake[i].y] = BEND_LEFT_UP;
              break;
            default:
              break;
          }
          break;
        case RIGHT:
          switch (snake[i].direction) {
            case UP:
              gameboard[snake[i].x][snake[i].y] = BEND_UP_LEFT;
              break;
            case DOWN:
              gameboard[snake[i].x][snake[i].y] = BEND_DOWN_LEFT;
              break;
            case RIGHT:
              gameboard[snake[i].x][snake[i].y] = SEGMENT_HOR;
              break;
            default:
              break;
          }
          break;
        case LEFT:
          switch (snake[i].direction) {
            case UP:
              gameboard[snake[i].x][snake[i].y] = BEND_UP_RIGHT;
              break;
            case DOWN:
              gameboard[snake[i].x][snake[i].y] = BEND_DOWN_RIGHT;
              break;
            case LEFT:
              gameboard[snake[i].x][snake[i].y] = SEGMENT_HOR;
              break;
            default:
              break;
          }
          break;
        default:
          break;
      }
    }

    // snake tail logic
    else if (i == snakeLength - 1) {
      // save last position before updating it
      int8_t tempX = lastX;
      int8_t tempY = lastY;
      lastX = snake[i].x;
      lastY = snake[i].y;

      // update position
      snake[i].x = tempX;
      snake[i].y = tempY;

      // save last direction before updating it
      lastDirection = snake[i].direction;

      // update direction
      int diffx = snake[1].x - snake[i - 1].x;
      int diffy = snake[1].y - snake[i - 1].y;

      if (diffx < 0) {
        snake[i].direction = RIGHT;
      } else if (diffx > 0) {
        snake[i].direction = LEFT;
      } else if (diffy < 0) {
        snake[i].direction = DOWN;
      } else if (diffy > 0) {
        snake[i].direction = UP;
      }

      // update gameboard
      switch (snake[i].direction) {
        case UP:
          gameboard[snake[i].x][snake[i].y] = TAIL_UP;
          break;
        case DOWN:
          gameboard[snake[i].x][snake[i].y] = TAIL_DOWN;
          break;
        case RIGHT:
          gameboard[snake[i].x][snake[i].y] = TAIL_RIGHT;
          break;
        case LEFT:
          gameboard[snake[i].x][snake[i].y] = TAIL_LEFT;
          break;
        default:
          break;
      }
    }

    // last cycle clears where tail was on gameboard
    else {
      gameboard[lastX][lastY] = EMPTY;
    }
  }

  // now that full snake is updated, check for collision with self
  for (int i = 1; i < snakeLength; i++) {
    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
      gameState = GAMELOST;
      playSound(LOST);
      return;
    }
  }
  
  // Update the game display
  updateLCDDisplay();
}

void TIM3_IRQHandler() {
  if(TIM3->SR & TIM_SR_UIF) {  // If update interrupt flag is set
    TIM3->SR &= ~TIM_SR_UIF;  // Clear interrupt flag
    movementLogic();          // Call movement logic
  }
}

void gameStateHandler() {
  if (button) gameState = RUNNING;
  if (lastGameState != gameState) {
    switch (gameState) {
      case IDLE:

        break;
      case RUNNING:

        break;
      case GAMELOST:
      case GAMEWON:
        readHighScoresFromSD();
        writeHighScoresToSD();
        break;
    
      default:
        break; 
    }
    lastGameState = gameState;
  }
}

void setup_tim1(void) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    RCC->AHBENR  |= RCC_AHBENR_GPIOAEN;

    // Configure only PA11 for TIM1_CH4
    GPIOA->MODER &= ~(3 << 22);  // Clear mode bits for PA11
    GPIOA->MODER |= (2 << 22);   // Set PA11 to alternate function mode
    GPIOA->AFR[1] &= ~(0xF << 12); // Clear AFR bits for PA11
    GPIOA->AFR[1] |= (0x2 << 12);  // Set AF2 for PA11

    TIM1->BDTR |= TIM_BDTR_MOE;
    TIM1->PSC = 0;
    TIM1->ARR = 2399;

    // Configure PWM mode for TIM1_CH4
    TIM1->CCMR2 &= ~(7 << 12);  // Clear OC4M
    TIM1->CCMR2 |= (6 << 12);   // Set OC4M = 110 (PWM1)
    TIM1->CCMR2 |= TIM_CCMR2_OC4PE;  // Enable output compare preload for CH4

    TIM1->CCER |= TIM_CCER_CC4E;  // Enable output for TIM1_CH4
    TIM1->CR1 |= TIM_CR1_CEN;
}

// void playSound(uint8_t song) {
//     switch (song) {
//         case 0:
//             TIM1->ARR = 999; // ~1 kHz
 
//             break;
//         case 1:
//             TIM1->ARR = 799; // ~1.25 kHz
//             break;
//         case 2:
//             TIM1->ARR = 599; // ~1.67 kHz
//             break;
//         default:
//             TIM1->CCR4 = 0; // Muted
//             return;
//     }
//     TIM1->CCR4 = TIM1->ARR / 2; // Maintains 50% cycle
// }



void setup_tim7(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    TIM7->PSC = 48000 - 1;    // 1ms ticks
    TIM7->ARR = SOUND_DURATION - 1;
    TIM7->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM7_IRQn);
    NVIC_SetPriority(TIM7_IRQn, 1);
}

void TIM7_IRQHandler(void) {
    if (TIM7->SR & TIM_SR_UIF) {
        TIM7->SR &= ~TIM_SR_UIF;
        TIM1->CCR4 = 0; // Stop sound
        TIM7->CR1 &= ~TIM_CR1_CEN; // Stop timer
    }
}

void play_sound_effect(uint8_t effect) {
    uint16_t frequency;
    switch (effect) {
        case EAT:
            frequency = EAT_FREQ;
            // TIM1->ARR = 999;
            break;
        case WON:
            frequency = WIN_FREQ;
            // TIM1->ARR = 799;
            break;
        case LOST:
            frequency = LOSE_FREQ;
            // TIM1->ARR = 599;
            break;
        default:
            return;
    }
    
    TIM1->ARR = 48000000 / frequency - 1;
    TIM1->CCR4 = TIM1->ARR / 2;
    
    TIM7->CR1 &= ~TIM_CR1_CEN;
    TIM7->CNT = 0;
    TIM7->CR1 |= TIM_CR1_CEN;
}


void playSound(uint8_t song) {
    play_sound_effect(song);
}

void delay_ms(uint32_t ms) {
    for(uint32_t i = 0; i < ms; i++)
        for(volatile uint32_t j = 0; j < 48000/4; j++);
}


void test_sounds() {
    printf("Testing eat sound...\n");
    playSound(EAT);
    delay_ms(500);
    
    printf("Testing win sound...\n");  
    playSound(WON);
    delay_ms(500);
    
    printf("Testing lose sound...\n");
    playSound(LOST);
    
}



void ateSnack(){

}


void writeHighScoresToSD() {
  disable_sdcard();
  enable_sdcard();
  mountSD();

  FRESULT res;
  res = f_unlink("HIGHS.TXT");
  if (res != FR_OK) return;

  int8_t temp1 = highscore1;
  int8_t temp2 = highscore2;

  if (snakeLength > highscore1) {
    highscore1 = snakeLength;
    highscore2 = temp1;
    highscore3 = temp2;
  } else if (snakeLength > highscore2) {
    highscore2 = snakeLength;
    highscore3 = temp2;
  } else if (snakeLength > highscore3) {
    highscore3 = snakeLength;
  }
  
  FIL fil;
  FRESULT fr;
  fr = f_open(&fil, "HIGHS.TXT", FA_WRITE | FA_CREATE_ALWAYS);
  if (fr != FR_OK) {
    f_close(&fil);
    return;
  }
  UINT wlen;
  int8_t buff[3] = {highscore1, highscore2, highscore3};
  fr = f_write(&fil, (BYTE*)buff, 3, &wlen);
  if (fr != FR_OK) {
    f_close(&fil);
    return;
  }
  f_close(&fil);
}

void readHighScoresFromSD() {
  disable_sdcard();
  enable_sdcard();
  mountSD();

  highscore1 = 0;
  highscore2 = 0;
  highscore3 = 0;

  FIL fil;
  FRESULT fr;
  fr = f_open(&fil, "HIGHS.TXT", FA_READ | FA_OPEN_EXISTING);
  if (fr != FR_OK) {
    f_close(&fil);
    return;
  }
  UINT br;
  int8_t buff[3];
  fr = f_read(&fil, buff, 3, &br);
  if (fr != FR_OK) {
    f_close(&fil);
    return;
  }
  f_close(&fil);
  highscore1 = buff[0];
  highscore2 = buff[1];
  highscore3 = buff[2];
}

void mountSD() {
  FATFS *fs = &fs_storage;
  if (fs->id != 0) return;
  int res = f_mount(fs, "", 1);
  if (res != FR_OK) return;
}

void goFaster() {
  TIM3->CR1 &= ~TIM_CR1_CEN;
  snakeSpeed -= SPEED_INCREASE;
  TIM3->ARR = snakeSpeed;
  TIM3->CR1 |= TIM_CR1_CEN;
}