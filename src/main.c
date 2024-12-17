/**
  ******************************************************************************
  * @file    main.c
  * @author  Andrew Dile, Abhi Annigeri, Daniel Wang, William Lee
  * @date    November 4 2024
  * @brief   ECE 362 Snake Game
  ******************************************************************************
*/

#include "superSnake.h"
#include "helper.h"

int main() {
  // set ups
  setupLCDDisplay();
  // readHighScoresFromSD();

  internal_clock();
  init_usart5();
  setup_adc();
  init_tim2();
  initializeSnake();
  setupMovementTimer();

  setup_tim1(); 
  setup_tim7();
  test_sounds();
 

  // playSound(WON);
  // readHighScoresFromSD();

  setupOLED();


  

  // loop part of game
  while (true) {
    updateJoystick();

    updateOLED();

    gameStateHandler();
  }
}


