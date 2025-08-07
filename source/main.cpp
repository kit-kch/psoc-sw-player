/**
 * Author: Johannes Pfau
 */

#include <neorv32.h>
#include "psoc_board.h"
#include "psoc/Player.h"

extern "C" int main()
{
    psoc_board_setup(true);
    neorv32_rte_setup();

    neorv32_uart0_printf("\n\n");
    neorv32_uart0_printf("#===========================================================================#\n");
    neorv32_uart0_printf("#                              PSoC OLED Test                               #\n");
    neorv32_uart0_printf("#===========================================================================#\n");

    psoc::Player player;
    player.init();
    player.run();

    return 0;
}