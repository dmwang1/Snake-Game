#include "superSnake.h"
#include "fifo.h"
#include "tty.h"

void init_spi1() {
  RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
  RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

  GPIOB->MODER &= ~(GPIO_MODER_MODER12 | GPIO_MODER_MODER13 | GPIO_MODER_MODER15);

  GPIOB->MODER |= GPIO_MODER_MODER12_1 | GPIO_MODER_MODER13_1 | GPIO_MODER_MODER15_1;

  SPI2->CR1 &= ~SPI_CR1_SPE;
  SPI2->CR1 |= SPI_CR1_MSTR | SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0;
  // SPI2->CR2 = (SPI2->CR2 & (SPI_CR2_DS)) | SPI_CR2_DS_0 | SPI_CR2_DS_3 | SPI_CR2_SSOE | SPI_CR2_NSSP | SPI_CR2_TXDMAEN;
  
  SPI2->CR2 |=  0b100100000000;
  SPI2->CR2 &= ~0b011000000000;
  // SPI2->CR2 |= 0b1110;
  SPI2->CR2 |= SPI_CR2_SSOE | SPI_CR2_NSSP | SPI_CR2_TXDMAEN;

  SPI2->CR1 |= SPI_CR1_SPE;
}

uint16_t display[34] = {
  0x002, // Command to set the cursor at the first position line 1
  0x200+'E', 0x200+'C', 0x200+'E', 0x200+'3', 0x200+'6', + 0x200+'2', 0x200+' ', 0x200+'i',
  0x200+'s', 0x200+' ', 0x200+'t', 0x200+'h', + 0x200+'e', 0x200+' ', 0x200+' ', 0x200+' ',
  0x0c0, // Command to set the cursor at the first position line 2
  0x200+'c', 0x200+'l', 0x200+'a', 0x200+'s', 0x200+'s', + 0x200+' ', 0x200+'f', 0x200+'o',
  0x200+'r', 0x200+' ', 0x200+'y', 0x200+'o', + 0x200+'u', 0x200+'!', 0x200+' ', 0x200+' ',
};

void spi1_init_oled() {
  nano_wait(1000000);
  spi_cmd(0x38);
  spi_cmd(0x08);
  //check again
  spi_cmd(0x01);
  nano_wait(2000000);
  spi_cmd(0x06);
  spi_cmd(0x02);
  spi_cmd(0x0c);
}

void spi1_setup_dma(void) {
  RCC->AHBENR |= RCC_AHBENR_DMA1EN;
  DMA1_Channel5->CCR &= ~DMA_CCR_EN;
  DMA1_Channel5->CCR |= DMA_CCR_DIR | DMA_CCR_MINC | DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0 | DMA_CCR_CIRC;
  DMA1_Channel5->CPAR = (uint32_t)&(SPI2->DR);
  DMA1_Channel5->CMAR = (uint32_t)display;
  DMA1_Channel5->CNDTR = 34;
  
  SPI2->CR2 |= SPI_CR2_TXDMAEN;
}

void spi1_enable_dma(void) {
  DMA1_Channel5->CCR |= DMA_CCR_EN;  
}

void spi1_dma_display1(const char *str) {
  for(int i=0; i<16; i++) {
    if (str[i])
      display[i+1] = 0x200 + str[i];
    else {
      // End of string.  Pad with spaces.
      for(int j=i; j<16; j++)
        display[j+1] = 0x200 + ' ';
      break;
    }
  }
}

void spi1_dma_display2(const char *str)
{
  for(int i=0; i<16; i++) {
    if (str[i])
      display[i+18] = 0x200 + str[i];
    else {
      // End of string.  Pad with spaces.
      for(int j=i; j<16; j++)
        display[j+18] = 0x200 + ' ';
      break;
    }
  }
}

void spi_cmd(unsigned int data) {
  while(!(SPI2->SR & SPI_SR_TXE));
  SPI2->DR = data;                 // RS=0
}

void setupOLED() {
  init_spi1();
  spi1_init_oled();

  spi1_setup_dma();
  spi1_enable_dma();
}

void updateOLED() {
  char line1[17];
  char line2[17];

  switch (gameState) {
    case IDLE:
      spi1_dma_display1("Welcome! Press");
      spi1_dma_display2("button to Start");
      break;
    case GAMEWON:
    case GAMELOST:
      sprintf(line1, "Highs: 1. %d", highscore1);
      sprintf(line2, "2. %d 3. %d", highscore2, highscore3);
      spi1_dma_display1(line1);
      spi1_dma_display2(line2);
      break;
    case RUNNING:
      sprintf(line1, "Current Score:");
      sprintf(line2, "%d", snakeLength);
      spi1_dma_display1(line1);
      spi1_dma_display2(line2);
      break;
  }
}