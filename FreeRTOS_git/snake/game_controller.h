#ifndef SNAKE_GAME_CONTROLLER_H_
#define SNAKE_GAME_CONTROLLER_H_

#include <stdint.h>
#include <FreeRTOS.h>

void initGameController(void);

/// Processes the key press received from the user.
/// Returns whether a higher priority task was woken and should be signalled to the task scheduler.
BaseType_t sendKeyPressFromIsr(uint8_t key);

#endif /* SNAKE_GAME_CONTROLLER_H_ */
