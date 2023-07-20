#include "app.h"
#include "uart_comm.h"
#include "snake/game_controller.h"

void app_init(void)
{
    UART0_Init();

    initGameController();
}
