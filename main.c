#include <stdio.h>
#include "pico/stdlib.h"
#include "nrf.h"

int main()
{
    stdio_init_all();
    sleep_ms(2000);

    printf("Initializing NRF...\n");

    if (!nrf_init())
    {
        printf("NRF not detected!\n");
        while (1)
            tight_loop_contents();
    }

    nrf_configure();
    nrf_start_listening();

    printf("Waiting for packets...\n");

    while (1)
    {
    uint8_t status = nrf_read_register(STATUS);
    printf("STATUS = 0x%02X\n", status);
    sleep_ms(500);
    }
}