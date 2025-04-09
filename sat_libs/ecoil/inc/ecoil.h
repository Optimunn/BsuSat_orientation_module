#ifndef ELECTRIC_COIL_H__
#define ELECTRIC_COIL_H__

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct coil_config_s
    {
        uint8_t chanel_1;
        uint8_t chanel_2;
        uint8_t enable_pin;
        uint duty_cycle;
    } coil_config_t;

    void coil_init(coil_config_t *config, uint chanel_1, uint chanel_2, uint enable_pin, uint32_t frequency, uint8_t bit_depth);
    void coil_enable(coil_config_t *config, uint8_t state);
    void coil_set_state(coil_config_t *config, uint16_t coefficient, uint8_t state);
    // private function
    static void configure_pwm_pin(uint pin, uint freq, uint duty_c);

#ifdef __cplusplus
}
#endif

#endif