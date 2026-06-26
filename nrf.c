#include "nrf.h"

#include "pico/stdlib.h"
#include "hardware/spi.h"

#define SPI_PORT spi0

#define PIN_CSN 17
#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_MISO 20
#define PIN_CE 16

#define NRF_SPI_SPEED 10000000

#define R_REGISTER         0x00
#define W_REGISTER         0x20
#define REGISTER_MASK      0x1F

#define R_RX_PAYLOAD       0x61
#define W_TX_PAYLOAD       0xA0
#define FLUSH_TX           0xE1
#define FLUSH_RX           0xE2
#define NOP                0xFF



#define CONFIG             0x00
#define EN_AA              0x01
#define EN_RXADDR          0x02
#define SETUP_AW           0x03
#define SETUP_RETR         0x04
#define RF_CH              0x05
#define RF_SETUP           0x06
#define STATUS             0x07
#define RX_ADDR_P0         0x0A
#define TX_ADDR            0x10
#define RX_PW_P0           0x11

#define FIFO_STATUS 0x17
#define RX_EMPTY    0
static const uint8_t address[5] = {'0', '0', '0', '0', '1'};


static inline void csn_low()
{
    gpio_put(PIN_CSN, 0);
}

static inline void csn_high()
{
    gpio_put(PIN_CSN, 1);
}

static inline void ce_low()
{
    gpio_put(PIN_CE, 0);
}

static inline void ce_high()
{
    gpio_put(PIN_CE, 1);
}

static uint8_t spi_transfer(uint8_t tx)
{
    uint8_t rx;

    spi_write_read_blocking(
        SPI_PORT,
        &tx,
        &rx,
        1
    );

    return rx;
}

uint8_t nrf_read_register(uint8_t reg)
{
    csn_low();

    spi_transfer(R_REGISTER | (reg & REGISTER_MASK));

    uint8_t value = spi_transfer(NOP);

    csn_high();

    return value;
}
void nrf_configure(void)
{
    ce_low();

    // Enable Auto ACK on pipe 0
    nrf_write_register(EN_AA, 0x01);

    // Enable RX pipe 0
    nrf_write_register(EN_RXADDR, 0x01);

    // 5-byte addresses
    nrf_write_register(SETUP_AW, 0x03);

    // 250 kbps, 0 dBm
    nrf_write_register(RF_SETUP, 0x26);

    // Channel 40 (2440 MHz)
    nrf_write_register(RF_CH, 76);

    // 32-byte payload
    nrf_write_register(RX_PW_P0, 32);

    // Retry: 750 µs delay, 15 retries
    nrf_write_register(SETUP_RETR, 0x2F);

    // Same address for TX and RX pipe 0
    nrf_write_registers(RX_ADDR_P0, address, 5);
    nrf_write_registers(TX_ADDR, address, 5);

    // Power up in PRX (receiver) mode
    nrf_write_register(CONFIG, 0x0E);

    sleep_ms(5);

    ce_high();
}
void nrf_write_register(uint8_t reg, uint8_t value)
{
    csn_low();

    spi_transfer(W_REGISTER | (reg & REGISTER_MASK));

    spi_transfer(value);

    csn_high();
}

void nrf_read_registers(uint8_t reg,
                        uint8_t *buf,
                        uint8_t len)
{
    csn_low();

    spi_transfer(R_REGISTER | (reg & REGISTER_MASK));

    for(uint8_t i=0;i<len;i++)
        buf[i]=spi_transfer(NOP);

    csn_high();
}

void nrf_write_registers(uint8_t reg,
                         const uint8_t *buf,
                         uint8_t len)
{
    csn_low();

    spi_transfer(W_REGISTER | (reg & REGISTER_MASK));

    for(uint8_t i=0;i<len;i++)
        spi_transfer(buf[i]);

    csn_high();
}

void nrf_flush_tx(void)
{
    csn_low();
    spi_transfer(FLUSH_TX);
    csn_high();
}

void nrf_flush_rx(void)
{
    csn_low();
    spi_transfer(FLUSH_RX);
    csn_high();
}

void nrf_write_payload(const uint8_t *data, uint8_t len)
{
    csn_low();

    spi_transfer(W_TX_PAYLOAD);

    for (uint8_t i = 0; i < len; i++)
        spi_transfer(data[i]);

    csn_high();
}

void nrf_read_payload(uint8_t *data, uint8_t len)
{
    csn_low();

    spi_transfer(R_RX_PAYLOAD);

    for (uint8_t i = 0; i < len; i++)
        data[i] = spi_transfer(NOP);

    csn_high();
}
void nrf_start_listening(void)
{
    ce_low();

    uint8_t config = nrf_read_register(CONFIG);
    config |= 0x01;                 // PRIM_RX = 1

    nrf_write_register(CONFIG, config);

    nrf_write_register(STATUS, 0x70);   // Clear IRQ flags

    ce_high();

    sleep_us(150);
}

void nrf_stop_listening(void)
{
    ce_low();

    uint8_t config = nrf_read_register(CONFIG);
    config &= ~0x01;                // PRIM_RX = 0

    nrf_write_register(CONFIG, config);

    sleep_us(150);
}

bool nrf_available(void)
{
    return !(nrf_read_register(FIFO_STATUS) & (1 << RX_EMPTY));
}

uint8_t nrf_receive(uint8_t *buf)
{
    nrf_read_payload(buf, 32);

    nrf_write_register(STATUS, 1 << 6);

    return 32;
}

bool nrf_send(const uint8_t *data, uint8_t len)
{
    nrf_stop_listening();

    nrf_flush_tx();

    nrf_write_payload(data, len);

    ce_high();
    sleep_us(15);
    ce_low();

    while (1)
    {
        uint8_t status = nrf_read_register(STATUS);

        if (status & (1 << 5))      // TX_DS
        {
            nrf_write_register(STATUS, 1 << 5);
            return true;
        }

        if (status & (1 << 4))      // MAX_RT
        {
            nrf_write_register(STATUS, 1 << 4);
            nrf_flush_tx();
            return false;
        }
    }
}


bool nrf_init(void)
{
    spi_init(SPI_PORT, NRF_SPI_SPEED);

    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);

    gpio_init(PIN_CSN);
    gpio_set_dir(PIN_CSN, GPIO_OUT);

    gpio_init(PIN_CE);
    gpio_set_dir(PIN_CE, GPIO_OUT);

    csn_high();
    ce_low();

    sleep_ms(100);

    return true;
}

