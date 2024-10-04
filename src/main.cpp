#include <Arduino.h>
#include "Arduino_LED_Matrix.h"
#include "ble.h"

// define game speed variables
unsigned long gameLogicInterval = 200;
unsigned long initialGameLogicInterval = 200;
unsigned long gameLogicIntervalMin = 100;
unsigned long gameLogicSpeedup = 10;
// direction variable for joystick input
uint8_t direction = 0;

// define snake and prey as collection of coordinates
struct Coordinate
{
  int x = 0;
  int y = 0;
};
Coordinate snake[100]; 
Coordinate prey;
int snakeLength = 1; 

// initialize matrix object
ArduinoLEDMatrix matrix;
const int cols = 12;
const int rows = 8;

// define bitmap to set matrix
byte bitmap[rows][cols] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

// forward declarations
void initializeGame();
unsigned long gameLogic(uint8_t directionBLE);
void generatePrey();
void moveSnake();
void checkCollision();
void clearBitmap();
void updateMatrix();


void setup() {
  Serial.begin(115200); 
  startBLEStack(); 
  matrix.begin();
  initializeGame();
}

void loop() {
  wrapperBLE(gameLogic);
  Serial.println("No connection! Try again in 3 seconds ...");
  delay(3000);
}

/* *******************************************************************************************************************
   ****************************************************** SNAKE ******************************************************
   ******************************************************************************************************************* */

void initializeGame()
{
  clearBitmap();
  direction = 0;
  gameLogicInterval = initialGameLogicInterval; 
  // initialize snake
  memset(snake, 0, sizeof(snake));
  snakeLength = 1;
  // start in center of matrix
  snake[0].x = cols/2; 
  snake[0].y = rows/2;
  bitmap[snake[0].y][snake[0].x] = 1;
  matrix.renderBitmap(bitmap, rows, cols);
  // generate prey
  generatePrey();
}

unsigned long gameLogic(uint8_t directionBLE)
{
  direction = directionBLE;
  moveSnake();
  updateMatrix();
  checkCollision();
  return gameLogicInterval;
}

void generatePrey()
{
  prey.x = random(cols);
  prey.y = random(rows);
  // regenerate prey when it spawns on the snake
  for (int i = 0; i < snakeLength; i++)
  {
    if (prey.x == snake[i].x && prey.y == snake[i].y)
    {
      generatePrey();
      return;
    }
  }
  bitmap[prey.y][prey.x] = 1;
  matrix.renderBitmap(bitmap, rows, cols);
}

void moveSnake()
{
  // move body
  for (int i = snakeLength - 1; i > 0; i--)
  {
    snake[i].x = snake[i - 1].x;
    snake[i].y = snake[i - 1].y;
  }
  // move head of the snake - employ periodic boundary conditions
  switch (direction)
  {
  case 1:
    // move left
    snake[0].x = (snake[0].x - 1 + cols) % cols;
    break;
  case 3:
    // move right
    snake[0].x = (snake[0].x + 1) % cols;
    break;
  case 2:
    // move up
    snake[0].y = (snake[0].y - 1 + rows) % rows;
    break;
  case 4:
    // move down
    snake[0].y = (snake[0].y + 1) % rows;
    break;
  }
}

void checkCollision()
{
  // check if snake hits itself
  for (int i = 1; i < snakeLength; i++)
  {
    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
      initializeGame();
  } 
  // check if snake eats prey
  if ((snake[0].x == prey.x) && (snake[0].y == prey.y))
  {
    snakeLength++;
    generatePrey();
    if (gameLogicInterval > gameLogicIntervalMin)
      gameLogicInterval -= gameLogicSpeedup;
  }
}

void clearBitmap()
{
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      bitmap[i][j] = 0;
    }
  }
}

void updateMatrix()
{
  clearBitmap();
  for (int i = 0; i < snakeLength; i++)
  {
    bitmap[snake[i].y][snake[i].x] = 1;
  }
  bitmap[prey.y][prey.x] = 1;
  matrix.renderBitmap(bitmap, rows, cols);
}
