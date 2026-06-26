#ifndef NRF_H
#define NRF_H

#include <stdint.h>
#include <stdbool.h>

// Register addresses
#define CONFIG         0x00
#define EN_AA          0x01
#define EN_RXADDR      0x02
#define SETUP_AW       0x03
#define SETUP_RETR     0x04
#define RF_CH          0x05
#define RF_SETUP       0x06
#define STATUS         0x07
#define RX_ADDR_P0     0x0A
#define TX_ADDR        0x10
#define RX_PW_P0       0x11

bool nrf_init(void);

void nrf_configure(void);

uint8_t nrf_read_register(uint8_t reg);
void nrf_write_register(uint8_t reg, uint8_t value);

void nrf_read_registers(uint8_t reg, uint8_t *buf, uint8_t len);
void nrf_write_registers(uint8_t reg, const uint8_t *buf, uint8_t len);
void nrf_flush_tx(void);
void nrf_flush_rx(void);

void nrf_write_payload(const uint8_t *data, uint8_t len);
void nrf_read_payload(uint8_t *data, uint8_t len);

void nrf_start_listening(void);
void nrf_stop_listening(void);

bool nrf_available(void);

bool nrf_send(const uint8_t *data, uint8_t len);

uint8_t nrf_receive(uint8_t *buf);

void nrf_flush_tx(void);
void nrf_flush_rx(void);
#endif