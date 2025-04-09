#include "pico/stdlib.h"
#include <stdio.h>
// Can module and Can function
#include "satframe.hpp"
// MPU9250 sensor
#include "mp92plus.h"
// INA3221 sensor
#include "ina3221.h"
// Orientation coils
#include "ecoil.h"

// Some defines ->
#define CAN_INTERRUPT 22
#define MPU_INTERRUPT 27

#define MODULE_ID 0x05

#define Mhz 1000000UL
// Some defines <-

// Functions ->
uint8_t init_all_sensors(void);
void add_interrupts(uint interrupt_pin, gpio_irq_callback_t callback);

uint8_t cmd_response_make(uint16_t cmd, uint8_t *data, uint16_t *d_len_exit, uint8_t *can_data);
void create_commands_list(void);
// Functions <-

// Modules instances ->
MCP2515 can_bus(spi0, 17, 19, 16, 18, Mhz);
mpu9250_t mpu92;
ina3221_t ina3221;

coil_config_t coil1;
coil_config_t coil2;
coil_config_t coil3;
// Modules instances <-

// Global sensor variables ->
int16_t data_gyro[3], data_accel[3], data_mag[3], temperature;
uint16_t current[3], voltage[3];
//? Coils ->
uint8_t coil_state[3] = {1, 1, 1};
uint8_t pwm[3] = {100, 200, 30};
uint8_t coil_orientation[3] = {1, 1, 1};
//? Coils <-
// Global sensor variables <-


// Global variables ->
uint8_t response_ready;
// Global variables <-

// Global structs ->
struct can_frame rx;
frame::sat_t sat_decode;
// Global structs <-

// Interrupt handler
void gpio_callback(uint gpio, uint32_t events)
{
    if (gpio == CAN_INTERRUPT)
    {
        //printf("__can_int__\n");
        uint8_t type = frame::can::read(can_bus, &sat_decode, &rx, MODULE_ID);
        if (type == 0x0)
            response_ready = 1;
        else if (type == 0x1)
        {
            response_ready = 0;
            printf("Frame response: id: %x\n", rx.can_id);
        }
        else
            printf("Can error\n");
    }
    else if (gpio == MPU_INTERRUPT)
    {
        static uint8_t interrupt_count = 0;
        if (interrupt_count == 8)
        {
            //printf("_mpu_int_\n");
            mpu9250_read_raw_motion(&mpu92, data_accel, data_gyro);
            mpu9250_read_raw_mag(&mpu92, data_mag);
            mpu9250_read_raw_temperature(&mpu92, &temperature);
            interrupt_count = 0;
        }
        else if (interrupt_count == 4)
        {
            //printf("_ina_int_\n");
            current[0] = ina3221_get_shunt_voltage_raw(&ina3221, 1);
            voltage[0] = ina3221_get_voltage_raw(&ina3221, 1);
            current[1] = ina3221_get_shunt_voltage_raw(&ina3221, 2);
            voltage[1] = ina3221_get_voltage_raw(&ina3221, 2);
            current[2] = ina3221_get_shunt_voltage_raw(&ina3221, 3);
            voltage[2] = ina3221_get_voltage_raw(&ina3221, 3);
        }
        interrupt_count++;
        mpu9250_continue_interrupt(&mpu92);
    }
}

int main()
{
    stdio_init_all();

    // Init led to check errors
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    // gpio_put(PICO_DEFAULT_LED_PIN, 1);
    // busy_wait_ms(1000); // TODO: Delete this
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    // <-

    if (init_all_sensors())
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
    // Can interrupt
    add_interrupts(CAN_INTERRUPT, gpio_callback);
    // Mpu9250 interrupt
    mpu9250_attach_interrupt(&mpu92, MPU_INTERRUPT, gpio_callback);

    create_commands_list();

    uint8_t data[70];

    uint32_t time = time_us_32();

    while (true)
    {
        if (response_ready)
        {
            response_ready = 0;
            uint16_t data_len_to_send;
            if (!cmd_response_make(sat_decode.cmd_id, data, &data_len_to_send, rx.data))
                frame::can::response(can_bus, &sat_decode, data, data_len_to_send);
            else
                printf("Unknown command: %i\n", sat_decode.cmd_id);
        }
        if(time_us_32() - time > 10000){
            time = time_us_32();

            coil_enable(&coil1, coil_state[0]);
            coil_set_state(&coil1, pwm[0], coil_orientation[0]);
            coil_enable(&coil2, coil_state[1]);
            coil_set_state(&coil2, pwm[1], coil_orientation[1]);
            coil_enable(&coil3, coil_state[2]);
            coil_set_state(&coil3, pwm[2], coil_orientation[2]);
        }
    }
}

// Add interrupt to pin
void add_interrupts(uint interrupt_pin, gpio_irq_callback_t callback)
{
    gpio_init(interrupt_pin);
    gpio_set_dir(interrupt_pin, GPIO_IN);
    gpio_pull_up(interrupt_pin);

    if (!irq_is_enabled(IO_IRQ_BANK0))
    {
        gpio_set_irq_enabled_with_callback(
            interrupt_pin,
            GPIO_IRQ_EDGE_FALL,
            true,
            callback);
    }
    else
    {
        gpio_set_irq_enabled(
            interrupt_pin,
            GPIO_IRQ_EDGE_FALL,
            true);
    }
}

// Dc-dc step up enable
void mt3608_enable(uint pin, uint8_t state)
{
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, state);
}

#define PWM_FREQ 1600
// Init all sensors
uint8_t init_all_sensors(void)
{
    // mpu9250 ->
    spi_pin_t my_spi = {
        .miso = 12,
        .mosi = 15,
        .sck = 14,
        .cs = 13,
    };
    if (mpu9250_reset(&mpu92, &my_spi, spi1, 1000 * 1000))
        return 1;

    mpu_settings_t set = {
        .accel_range = ACCEL_RANGE_16G,
        .gyro_range = GYRO_RANGE_2000DPS,
        .dlpf_filter = DLPF_10HZ,
        .sample_rate_divider = 249};
    int16_t correct_gyro[] = {-83, 43, -9};
    if (mpu9250_setup(&mpu92, &set, correct_gyro))
        return 2;
    // <- mpu9250

    // ina3221 ->
    if (ina3221_init(&ina3221, 0x43, i2c1, 6, 7, 400 * 1000, 200))
        return 3;
    // <- ina3221

    // Can bus ->
    if (can_bus.reset())
        return 4;

    if (can_bus.setBitrate(CAN_1000KBPS))
        return 5;

    if (can_bus.setNormalMode())
        return 6;
    // <- Can bus

    // Coils ->
    mt3608_enable(11, 1);

    coil_init(&coil1, 0, 1, 2, PWM_FREQ, 8);
    coil_enable(&coil1, coil_state[0]);
    coil_set_state(&coil1, 100, 1);

    coil_init(&coil2, 4, 5, 3, PWM_FREQ, 8);
    coil_enable(&coil2, coil_state[1]);
    coil_set_state(&coil2, 200, 1);

    coil_init(&coil3, 8, 9, 10, PWM_FREQ, 8);
    coil_enable(&coil3, coil_state[2]);
    coil_set_state(&coil3, 30, 1);
    // <- Coils
    return 0;
}
