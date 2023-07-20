#include "game_controller.h"
#include "game.h"
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "../uart_comm.h"

#define KEY_RESET 113
#define KEY_LEFT  52
#define KEY_RIGHT 54

/// Stores the character (key press) the game received from the user
QueueHandle_t queueCommandCharacter;

TaskHandle_t taskHandle_InterpretCommand;

void task_InterpretCommand()
{
    uint8_t key;

    while (1)
    {
        vTaskSuspend(NULL); // Wait for a key press to be received

        if (xQueueReceive(queueCommandCharacter, &key, portMAX_DELAY) != pdPASS) {
            // print("?");

            vTaskSuspend(NULL); // If failed to receive a key, suspend itself
            continue;
        }

        // Interpret the key based on the game state
        switch (getGameState())
        {
          case GameState_Playing:
            // In the playing state, only the reset and turn keys are valid
            switch (key)
            {
              //case KEY_RESET:
                //resetGame();
                //break;
              case KEY_LEFT:
                applyTurn(TurnDirection_Left);
                break;
              case KEY_RIGHT:
                applyTurn(TurnDirection_Right);
                break;
            }

            break;

          case GameState_GameOver:
            resetGame();

            break;

          default: // GameState_Ready
            if (key != KEY_RESET) // for anything else than the reset key, start the game
                startGame();

            break;
        }
    }
}

BaseType_t sendKeyPressFromIsr(uint8_t key)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xQueueSendFromISR(queueCommandCharacter, &key, &xHigherPriorityTaskWoken);

    xHigherPriorityTaskWoken |= xTaskResumeFromISR(taskHandle_InterpretCommand); // Run the task to interpret the command

    return xHigherPriorityTaskWoken;
}

void initGameController(void)
{
    initGame();

    queueCommandCharacter = xQueueCreate(1, sizeof(uint8_t));

    xTaskCreate(
        task_InterpretCommand, // pxTaskCode
        "task_InterpretCommand", // pcName
        configMINIMAL_STACK_SIZE, // usStackDepth
        NULL, // pvParameters
        tskIDLE_PRIORITY + 1, // uxPriority
        &taskHandle_InterpretCommand // pxCreatedTask
        );
}
