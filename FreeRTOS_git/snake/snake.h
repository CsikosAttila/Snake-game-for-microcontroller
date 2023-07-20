#ifndef SNAKE
#define SNAKE

#include "display.h"

#include <stdbool.h>

/// The maximum possible length of the snake
#define MAX_SNAKE_LENGTH 37

/// Gets the current length of the snake
int getSnakeLength();
/// Sets the current length of the snake
void setSnakeLength(int newSnakeLength);

/// Gets the segments representing the body of the snake
Segment* getSnake(void);

/// Represents orientations of the head of the snake
typedef enum {
    Orientation_Up,
    Orientation_Left,
    Orientation_Down,
    Orientation_Right
} Orientation;

/// Gets the current orientation of the head of the snake
Orientation getSnakeOrientation();
/// Sets the current orientation of the head of the snake
void setSnakeOrientation(Orientation newSnakeOrientation);

/// Resets the snake to its initial state
void resetSnake();

/// Moves the snake one step forward, towards the specified position
void moveSnake(Segment newHead);

/// Gets a pointer to the segment representing the head of the snake
Segment *getHead();

/// Displays the snake on the display
void displaySnake();

/// Reappends the last disposed segment of the snake to the snake tail
void appendSnakeTail();

/// Gets whether the specified segment is part of the snake body
/// @param ignoreHead Whether to ignore the head of the snake
bool isSegmentInSnake(Segment segment, bool ignoreHead);

#endif
