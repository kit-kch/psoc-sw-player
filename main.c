
#include <neorv32.h>
#include "psoc_board.h"

int main()
{
    psoc_board_setup(true);
    neorv32_rte_setup();

    neorv32_uart0_printf("\n\n#=========================================================#\n");
    neorv32_uart0_printf("#                     PSoC OLED Test                      #\n");
    neorv32_uart0_printf("#=========================================================#\n\n\n");

    while (true) {}
    return 0;
}