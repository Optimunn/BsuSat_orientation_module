#include "ecoil.h"

static void configure_pwm_pin(uint pin, uint freq, uint duty_c)
{
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    float div = (float)clock_get_hz(clk_sys) / (freq * duty_c);
    pwm_config_set_clkdiv(&config, div);
    pwm_config_set_wrap(&config, duty_c);
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0);      
};

void coil_init(coil_config_t *config, uint chanel_1, uint chanel_2, uint enable_pin, uint32_t frequency, uint8_t bit_depth)
{
    gpio_init(enable_pin);
    gpio_set_dir(enable_pin, GPIO_OUT);
    config->enable_pin = enable_pin;

    uint duty_c = (1 << bit_depth) - 1;
    config->duty_cycle = duty_c;

    configure_pwm_pin(chanel_1, frequency, duty_c);
    config->chanel_1 = chanel_1;

    configure_pwm_pin(chanel_2, frequency, duty_c);
    config->chanel_2 = chanel_2;
}

void coil_enable(coil_config_t *config, uint8_t state)
{
    gpio_put(config->enable_pin, state);
}

void coil_set_state(coil_config_t *config, uint16_t coefficient, uint8_t state)
{
    (coefficient > config->duty_cycle) ? coefficient = config->duty_cycle : coefficient;

    if (state)
    {
        pwm_set_gpio_level(config->chanel_1, coefficient);
        pwm_set_gpio_level(config->chanel_2, 0);
    }
    else
    {
        pwm_set_gpio_level(config->chanel_1, 0);
        pwm_set_gpio_level(config->chanel_2, coefficient);
    }
}