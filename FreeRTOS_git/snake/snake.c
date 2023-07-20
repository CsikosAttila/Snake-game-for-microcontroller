#include "snake.h"
#include "display.h"

#define INITIAL_SNAKE_LENGTH 1

/// Stores the segments representing the body of the snake
Segment snake[MAX_SNAKE_LENGTH];

Segment* getSnake()
{
  return snake;
}

/// The current length of the snake
int snakeLength = INITIAL_SNAKE_LENGTH;

int getSnakeLength() {
    return snakeLength;
}
void setSnakeLength(int newSnakeLength) {
    snakeLength = newSnakeLength;
}

/// The current orientation of the head of the snake
Orientation snakeOrientation;

Orientation getSnakeOrientation() {
    return snakeOrientation;
}
void setSnakeOrientation(Orientation newSnakeOrientation) {
    snakeOrientation = newSnakeOrientation;
}

/// The last, disposed segment of the snake
Segment snakeTail;

void resetSnake() {
    snakeLength = INITIAL_SNAKE_LENGTH;
    snake[0].row = 2;
    snake[0].column = 0;

    snakeOrientation = Orientation_Right;
}

void moveSnake(Segment newHead) {
    // save last tail
    snakeTail = snake[snakeLength - 1];

    // move all segments forward
    for (int i = snakeLength - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }

    // set new head
    snake[0] = newHead;
}

Segment* getHead() {
    return snake;
}

void displaySnake() {
    displaySegmentList(snake, snakeLength, /* reset = */ true);
}

void appendSnakeTail() {
    snake[snakeLength] = snakeTail;
    snakeLength++;
}

bool isSegmentInSnake(Segment segment, bool ignoreHead) {
    for (int i = ignoreHead ? 1 : 0; i < snakeLength; i++)
        if (areSegmentsEqual(snake[i], segment))
            return true;

    return false;
}
