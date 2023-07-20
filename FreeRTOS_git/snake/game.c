#include "game.h"

#include "snake.h"
#include "display.h"
#include "random.h"
#include "../segmentlcd.h"
#include "../uart_comm.h"

#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

/// Number of columns in the field the snake can step onto with a horizontal orientation
#define FIELD_WIDTH_HORIZONTAL 7
#define FIELD_WIDTH_HORIZONTAL_WITH_EDGE FIELD_WIDTH_HORIZONTAL + 1

/// The current state of the game
GameStates gameState = GameState_Ready;

TaskHandle_t taskHandle_GamePlaying;
TaskHandle_t taskHandle_GameOver;

/// Stores the turn direction in the next step.
QueueHandle_t queueNextTurn;

void flashDots(void);

static void taskGamePlaying(void *pvParam __attribute__((unused)))
{
    vTaskSuspend(NULL);

    while (1)
    {
        BaseType_t isGameOver = gameStep();
        if (isGameOver)
        {
            vTaskResume(taskHandle_GameOver);
            vTaskSuspend(NULL);
        }
        else
        {
            vTaskDelay(configTICK_RATE_HZ); // delay between two game steps
        }
    }
}

static void taskGameOver(void *pvParam __attribute__((unused)))
{
    while (1)
    {
        vTaskSuspend(NULL);

        gameState = GameState_GameOver;

        print("#");
        printNumber(getSnakeLength());
        print("|");

        flashDots();
    }
}

void initGame(void)
{
    /* Initialize the LCD
     * (For this, we use the standard driver, located in files "segmentlcd.(c|h)"
     */
    SegmentLCD_Init(false);

    initRandom();

    // Create tasks for the game
    xTaskCreate(
        taskGamePlaying, // pxTaskCode
        "taskGamePlaying", // pcName
        configMINIMAL_STACK_SIZE, // usStackDepth
        NULL, // pvParameters
        tskIDLE_PRIORITY + 1, // uxPriority
        &taskHandle_GamePlaying // pxCreatedTask
        );

    xTaskCreate(
        taskGameOver, // pxTaskCode
        "taskGameOver", // pcName
        configMINIMAL_STACK_SIZE, // usStackDepth
        NULL, // pvParameters
        tskIDLE_PRIORITY + 1, // uxPriority
        &taskHandle_GameOver // pxCreatedTask
        );

    queueNextTurn = xQueueCreate(1, sizeof(TurnDirection));

    resetGame();
}

GameStates getGameState()
{
    return gameState;
}

void setGameState(GameStates newGameState)
{
    gameState = newGameState;
}

TurnDirection receiveNextTurn()
{
    TurnDirection nextTurn;

    return
        xQueueReceive(queueNextTurn, &nextTurn, 0 /*return immediately*/) == pdPASS
        ? nextTurn
        : TurnDirection_Straight; // if the queue is empty, return straight
}

void setNextTurn(TurnDirection newNextTurn)
{
    xQueueSend(queueNextTurn, &newNextTurn, portMAX_DELAY);
}

void applyTurn(TurnDirection direction) {
    TurnDirection nextTurn = receiveNextTurn();

    if (direction == nextTurn) { // if the turn direction is already set, return
        setNextTurn(nextTurn); // receiveNextTurn() removed the item from the queue, so we put it back
        return;
    }

    // Apply the turn direction
    switch (direction)
    {
    case TurnDirection_Left:
        setNextTurn((nextTurn + TurnDirection_Left) % 4);
        break;

    case TurnDirection_Right:
        setNextTurn((nextTurn + TurnDirection_Right) % 4);
        break;

    default: // TurnDirection_Straight
        break;
    }
}

/// Food position
Segment food;

/// Places a food item on the field at a random position
/// and ensures that the food is not placed on the snake
void placeFood()
{
    do
    {
        food = getRandomSegment();
    } while (isSegmentInSnake(food, /* ignoreHead = */ false));
}
/// Displays the food on the display
/// @param flash Whether to flash the food item after having displayed it
void displayFood(bool flash)
{
    displaySegment(food, /* reset = */ false); // Display the food segment without resetting the display (to not to clear the snake)

    if (flash)
    {
        for (int i = 0; i < 2; i++)
        {
            vTaskDelay(configTICK_RATE_HZ / 100 * 8);
            displaySnake();

            vTaskDelay(configTICK_RATE_HZ / 100 * 8);
            displaySegment(food, /* reset = */ false);
        }
    }
}

/// Displays the current length of the snake
void displaySnakeLength()
{
    displayScore(getSnakeLength());
}

/// Flashes the decimal dots and the gecko symbol on the display
void flashDots()
{
    const int flashDelay = 100;
    const int flashCount = 8;

    for (int i = 0; i < flashCount; i++)
    {
        // turn on decimal dots
        setDots(1);
        vTaskDelay(configTICK_RATE_HZ / 1000 * flashDelay);

        // turn off decimal dots (but leave them on at the end)
        if (i < flashCount - 1)
        {
            setDots(0);
            vTaskDelay(configTICK_RATE_HZ / 1000 * flashDelay);
        }
    }
}

void resetGame()
{
    gameState = GameState_Ready;
    receiveNextTurn(); // empty the queue of the next turn, thus resetting it

    resetSnake();
    displaySnake();
    displaySnakeLength();

    setDots(0);
}

void startGame()
{
    gameState = GameState_Playing;

    print("+");

    placeFood();
    displayFood(true);

    vTaskResume(taskHandle_GamePlaying);
}

/// Get the new orientation after a step of the snake
Orientation getOrientationAfterStep(
    Orientation currentOrientation,
    TurnDirection turnDirection)
{
    return (Orientation)((currentOrientation + turnDirection + 4) % 4);
}

void stepSnake()
{
    Segment currentHead = *getHead(); // get the current head position of the snake
    Segment newHead;

    Orientation nextOrientation = getOrientationAfterStep(getSnakeOrientation(), receiveNextTurn());

    if (getSnakeOrientation() == Orientation_Left || getSnakeOrientation() == Orientation_Right)
    {
        if (nextOrientation == Orientation_Left || nextOrientation == Orientation_Right)
        {
            // current orientation: horizontal
            // next orientation:    horizontal (straight)

            newHead.row = currentHead.row; // straight horizontal step, so row index does not change

            int delta = (nextOrientation == Orientation_Left) ? -1 : 1;                                      // change of column index (without overflow)
            newHead.column = (currentHead.column + delta + FIELD_WIDTH_HORIZONTAL) % FIELD_WIDTH_HORIZONTAL; // handle overflow (with negative values included)
        }
        else
        {
            // current orientation: horizontal
            // next orientation:    vertical (turn)

            newHead.column = currentHead.column + (getSnakeOrientation() == Orientation_Right ? 1 : 0); // in case of left orientation, the snake stays in the same digit (same column); otherwise, steps into the rightmost one

            newHead.row = ((currentHead.row == 2) == (nextOrientation == Orientation_Down)) ? 3 : 1; // symmetrically check into which of the two possible rows the snake will step into
        }
    }
    else
    {
        if (nextOrientation == Orientation_Up || nextOrientation == Orientation_Down)
        {
            // current orientation: vertical
            // next orientation:    vertical (straight)

            newHead.column = currentHead.column;        // straight vertical step, so column index does not change
            newHead.row = currentHead.row == 1 ? 3 : 1; // step into the other row
        }
        else
        {
            // current orientation: vertical
            // next orientation:    horizontal (turn)

            int delta = nextOrientation == Orientation_Left ? -1 : 0;                                        // in case of righ orientation, the snake stays in the same digit (same column); otherwise, steps into the leftmost one
            newHead.column = (currentHead.column + delta + FIELD_WIDTH_HORIZONTAL) % FIELD_WIDTH_HORIZONTAL; // handle overflow (with negative values included)

            newHead.row = currentHead.row + (getSnakeOrientation() == Orientation_Down ? 1 : -1); // adjust the row, overflow is impossible here
        }
    }

    setSnakeOrientation(nextOrientation); // update the snake orientation

    moveSnake(newHead);
}

BaseType_t gameStep()
{
    stepSnake(); // move the snake one step forward

    // Check for game over
    if (isSegmentInSnake(*getHead(), /* ignoreHead = */ true)) // check if the head stepped onto a field of the snake body
        return 1;

    setNextTurn(TurnDirection_Straight); // reset the turn direction for the next step

    // Check if the snake has eaten the food
    bool hasEaten = isSegmentInSnake(food, /* ignoreHead = */ false);
    if (hasEaten)
    {
        appendSnakeTail(); // grow the snake at its tail

        displaySnakeLength();

        placeFood(); // place a new food item
    }

    displaySnake(); // display the snake after clearing the display
    // Display the food after the snake, so it is not overwritten by the snake;
    // flash the food only if it is newly appearing
    displayFood(/* flash = */ hasEaten);

    return 0;
}
