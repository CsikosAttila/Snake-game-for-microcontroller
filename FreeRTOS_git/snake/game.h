#ifndef GAME
#define GAME

#include <FreeRTOS.h>

/// Represents the main states of the game
typedef enum
{
    GameState_Ready,
    GameState_Playing,
    GameState_GameOver
} GameStates;

/// Gets the current state of the game
GameStates getGameState();
/// Sets the current state of the game
void setGameState(GameStates newGameState);

/// Represents the possible turn directions of the snake
typedef enum
{
    TurnDirection_Straight = 0,
    TurnDirection_Left = 1,
    TurnDirection_Right = 3
} TurnDirection;

/// Sets the next turn direction
void setNextTurn(TurnDirection newNextTurn);
/// Changes the turn direction based on the specified button press.
/// For example, if the next turn direction is left and the user presses the right button, the next turn direction will be straight.
void applyTurn(TurnDirection direction);

/// Initialise the game
void initGame(void);

/// Resets the game to the ready state
void resetGame();

/// Starts the game
void startGame();

/// Steps the snake one step in the direction set by nextTurn
void stepSnake();

/// Performs one complete step of the game and returns whether it is game over
BaseType_t gameStep();

#endif
