#include "pico/stdlib.h"

void make_usize_axis_reversed(uint8_t *data, uint16_t *d_item, uint16_t *param, uint16_t num)
{
    for (uint16_t item = 0; item < num; item++)
    {
        *(data + *d_item + 1) = (*(param + item) >> 8) & 0xFF;
        *(data + *d_item) = *(param + item) & 0xFF;
        *d_item += 2;
    }
}

void make_usize_axis_reversed(uint8_t *data, uint16_t *d_item, int16_t *param, uint16_t num)
{
    for (uint16_t item = 0; item < num; item++)
    {
        *(data + *d_item + 1) = (*(param + item) >> 8) & 0xFF;
        *(data + *d_item) = *(param + item) & 0xFF;
        *d_item += 2;
    }
}

void make_usize_axis(uint8_t *data, uint16_t *d_item, int8_t *param, uint16_t num)
{
    for (uint16_t item = 0; item < num; item++)
    {
        *(data + *d_item) = *(param + item);
        *d_item += 1;
    }
}

void make_usize_axis(uint8_t *data, uint16_t *d_item, uint8_t *param, uint16_t num)
{
    for (uint16_t item = 0; item < num; item++)
    {
        *(data + *d_item) = *(param + item);
        *d_item += 1;
    }
}

void make_usize_axis(uint8_t *data, uint16_t *d_item, uint32_t *param, uint16_t num)
{
    for (uint16_t item = 0; item < num; item++)
    {
        *(data + *d_item) = (*(param + item) >> 24) & 0xFF;
        *(data + *d_item + 1) = (*(param + item) >> 16) & 0xFF;
        *(data + *d_item + 2) = (*(param + item) >> 8) & 0xFF;
        *(data + *d_item + 3) = *(param + item) & 0xFF;
        *d_item += 4;
    }
}

void make_usize_axis(uint8_t *data, uint16_t *d_item, uint16_t *param, uint16_t num)
{
    for (uint16_t item = 0; item < num; item++)
    {
        *(data + *d_item) = (*(param + item) >> 8) & 0xFF;
        *(data + *d_item + 1) = *(param + item) & 0xFF;
        *d_item += 2;
    }
}

extern int16_t data_gyro[3], data_accel[3], data_mag[3], temperature;
extern uint16_t current[3], voltage[3];
extern uint8_t coil_orientation[3], coil_state[3], pwm[3];

//* Commands -->
void pico_prepare_telemetry(uint8_t *data, uint16_t *d_len_exit, uint8_t *can_data)
{
    uint16_t d_len = 0;
    make_usize_axis_reversed(data, &d_len, data_gyro, 3);
    make_usize_axis_reversed(data, &d_len, data_accel, 3);
    make_usize_axis_reversed(data, &d_len, data_mag, 3);

    make_usize_axis_reversed(data, &d_len, &temperature, 1);

    make_usize_axis_reversed(data, &d_len, current, 3);
    make_usize_axis_reversed(data, &d_len, voltage, 3);

    uint8_t coil_what = coil_state[0] && coil_state[1] && coil_state[2];
    make_usize_axis(data, &d_len, &coil_what, 1);

    uint8_t zeros[34];
    for (uint8_t i = 0; i < 34; i++)
        zeros[i] = i + 1;

    make_usize_axis(data, &d_len, zeros, 34);

    *d_len_exit = d_len;
}

void null_function(uint8_t *data, uint16_t *d_len_exit, uint8_t *can_data)
{
    asm volatile("nop");
    *d_len_exit = 0;
}

void pico_reset(uint8_t *data, uint16_t *d_len_exit, uint8_t *can_data)
{
    *((volatile uint32_t*)(PPB_BASE + 0x0ED0C)) = 0x5FA0004;
    *d_len_exit = 0;
}

void pico_zeros(uint8_t *data, uint16_t *d_len_exit, uint8_t *can_data)
{
    uint8_t paint = 8;
    for (uint8_t item = 0; item < paint; item++)
        *(data + item) = 0;
    *d_len_exit = paint;
}

void pico_led(uint8_t *data, uint16_t *d_len_exit, uint8_t *can_data)
{
    static uint8_t state = 0;
    state = !state;
    gpio_put(PICO_DEFAULT_LED_PIN, state);
    *d_len_exit = 0;
}

void pico_off_coil(uint8_t *data, uint16_t *d_len_exit, uint8_t *can_data)
{
    if (*can_data)
    {
        coil_state[0] = 1;
        coil_state[1] = 1;
        coil_state[2] = 1;
        return;
    }
    coil_state[0] = 0;
    coil_state[1] = 0;
    coil_state[2] = 0;
    *d_len_exit = 0;
}

void pico_coil_sleep(uint8_t *data, uint16_t *d_len_exit, uint8_t *can_data)
{
    if (*can_data < 4 && *can_data > 0)
        coil_state[*can_data - 1] = *(can_data + 1);
    *d_len_exit = 0;
}

void pico_coil_full(uint8_t *data, uint16_t *d_len_exit, uint8_t *can_data)
{
    pwm[0] += 25;
    pwm[1] += 35;
    pwm[2] += 15;
    *d_len_exit = 0;
}

void pico_set_coil(uint8_t *data, uint16_t *d_len_exit, uint8_t *can_data)
{
    if (*can_data < 4 && *can_data > 0)
    {
        pwm[*can_data - 1] = *(can_data + 2);
        if (*(can_data + 1))
        {
            coil_orientation[*can_data - 1] = 1;
            return;
        }
        coil_orientation[*can_data - 1] = 0;
    }
    *d_len_exit = 0;
}

void pico_get_time(uint8_t *data, uint16_t *d_len_exit, uint8_t *can_data)
{
    uint32_t time = time_us_32();
    uint16_t d_len = 0;
    make_usize_axis(data, &d_len, &time, 1);
    *d_len_exit = d_len;
}

void pico_get_pwm(uint8_t *data, uint16_t *d_len_exit, uint8_t *can_data)
{
    uint16_t d_len = 0;
    make_usize_axis(data, &d_len, pwm, 3);
    *d_len_exit = d_len;
}
//* <-- Commands

#define NUM_CMD_RESPONSE (uint16_t)11

uint16_t cmd_id_resp[NUM_CMD_RESPONSE];
void (*cmd_func[NUM_CMD_RESPONSE])(uint8_t *data, uint16_t *d_len_exit, uint8_t *can_data);

void push_cmd(uint16_t cmd_id, void (*func)(uint8_t *data, uint16_t *d_len_exit, uint8_t *can_data))
{
    static uint16_t count = 0;
    cmd_id_resp[count] = cmd_id;
    cmd_func[count] = func;
    count += 1;
}

void create_commands_list(void)
{
    //! Alert:
    //! Attention:
    //! Warning: depends on the "NUM_CMD_RESPONSE"
    push_cmd(4, pico_reset);             //* BSUSAT_CMD_RESET
    push_cmd(2, pico_prepare_telemetry); //* BSUSAT_CMD_TELEMETRY_GET
    push_cmd(3, pico_get_time);          //* BSUSAT_CMD_TIME_GET
    push_cmd(251, pico_off_coil);        //* ADCS_SET_COILS_PWR_STATE
    push_cmd(250, pico_set_coil);        //* ADCS_CONTROL_COIL_MANUALLY
    push_cmd(252, pico_coil_sleep);      //* ADCS_SET_SLEEP_MODE
    push_cmd(6, pico_coil_full);         //* custom cmd
    push_cmd(5, pico_led);               //* custom cmd
    push_cmd(7, pico_get_pwm);           //* custom cmd
    push_cmd(8, null_function);          //* custom cmd
    push_cmd(9, pico_zeros);             //* custom cmd
}

uint8_t cmd_response_make(uint16_t cmd, uint8_t *data, uint16_t *d_len_exit, uint8_t *can_data)
{
    if (cmd > (1 << 9))
        return 1;

    for (uint16_t item = 0; item < (sizeof(cmd_id_resp) / sizeof(cmd_id_resp[0])); item++)
    {
        if (cmd_id_resp[item] == cmd)
        {
            cmd_func[item](data, d_len_exit, can_data);
            return 0;
        }
    }
    return 1;
}
