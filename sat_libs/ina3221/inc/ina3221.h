#ifndef __INA3221_H_
#define __INA3221_H_

#define RESET_SYSTEM (uint8_t)1 // bit to reset system

// Averaging mode.
// These bits set the number of samples that are collected and averaged together.

// channel ON
#define CHANNEL_1 (uint8_t)4   // only first channel
#define CHANNEL_2 (uint8_t)2   // only second channel
#define CHANNEL_3 (uint8_t)1   // only third channel
#define ALL_CHANNEL (uint8_t)7 // all channel

#define AVG2_0 (uint8_t)0 // 1. (default)
#define AVG2_1 (uint8_t)1 // 4.
#define AVG2_2 (uint8_t)2 // 16.
#define AVG2_3 (uint8_t)3 // 64.
#define AVG2_4 (uint8_t)4 // 128.
#define AVG2_5 (uint8_t)5 // 256.
#define AVG2_6 (uint8_t)6 // 512.
#define AVG2_7 (uint8_t)7 // 1024.

// Bus-voltage conversion time.
// These bits set the conversion time for the bus-voltage measurement.

#define Vbus_CTV2_0 (uint8_t)0 // 140us.
#define Vbus_CTV2_1 (uint8_t)1 // 240us.
#define Vbus_CTV2_2 (uint8_t)2 // 332us.
#define Vbus_CTV2_3 (uint8_t)3 // 588us.
#define Vbus_CTV2_4 (uint8_t)4 // 1.1ms.(default)
#define Vbus_CTV2_5 (uint8_t)5 // 2.116ms.
#define Vbus_CTV2_6 (uint8_t)6 // 4.156ms.
#define Vbus_CTV2_7 (uint8_t)7 // 8.244ms.

// Shunt-voltage conversion time. These bits set the conversion time for the shunt-voltage measurement.
// The conversion-time bit settings for VSHCT2-0 are the same as VBUSCT2-0 (bits 8-6) listed in the previous row.

#define Vsh_CTV2_0 (uint8_t)0 // 140us.
#define Vsh_CTV2_1 (uint8_t)1 // 240us.
#define Vsh_CTV2_2 (uint8_t)2 // 332us.
#define Vsh_CTV2_3 (uint8_t)3 // 588us.
#define Vsh_CTV2_4 (uint8_t)4 // 1.1ms.(default)
#define Vsh_CTV2_5 (uint8_t)5 // 2.116ms.
#define Vsh_CTV2_6 (uint8_t)6 // 4.156ms.
#define Vsh_CTV2_7 (uint8_t)7 // 8.244ms.

// registrs

#define __REG_CONFIG (uint8_t)0x00
#define __MANUFACTURER_ID (uint8_t)0xFE

#define __REG_SHUNTVOLTAGE_1 (uint8_t)0x01
#define __REG_BUSVOLTAGE_1 (uint8_t)0x02
#define __REG_SHUNTVOLTAGE_2 (uint8_t)0x03
#define __REG_BUSVOLTAGE_2 (uint8_t)0x04
#define __REG_SHUNTVOLTAGE_3 (uint8_t)0x05
#define __REG_BUSVOLTAGE_3 (uint8_t)0x06

#define __MODE7 (uint8_t)7 // Shunt and bus, continuous (default)
#define __MODE6 (uint8_t)6 // Bus voltage, continuous
#define __MODE5 (uint8_t)5 // Shunt voltage, continuous
#define __MODE4 (uint8_t)4 // Power-down
#define __MODE3 (uint8_t)3 // Shunt and bus, single-shot (triggered)
#define __MODE2 (uint8_t)2 // Bus voltage, single-shot (triggered)
#define __MODE1 (uint8_t)1 // Shunt voltage, single-shot (triggered)
#define __MODE0 (uint8_t)0 // Power-down

#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <errno.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct ina3221_s
{
    uint8_t addr;
    i2c_inst_t *inst;
    uint16_t shunt_m_ohm;
} ina3221_t;

static void ina_read_register(ina3221_t *config, uint8_t addr, uint16_t *buf, uint8_t len);
static void ina_write_register(ina3221_t *config, uint8_t addr, uint16_t reg);
static uint8_t verify_id(ina3221_t *config);

uint16_t ina3221_get_voltage_raw(ina3221_t *config, uint8_t channel);
uint16_t ina3221_get_shunt_voltage_raw(ina3221_t *config, uint8_t channel);

float ina3221_get_voltage(ina3221_t *config, uint8_t channel);
float ina3221_get_current(ina3221_t *config, uint8_t channel);
float ina3221_get_power(ina3221_t *config, uint8_t channel);

error_t ina3221_init(ina3221_t *config, int8_t addr, i2c_inst_t *inst, uint sda, uint scl, uint32_t frequency, uint16_t shunt_m_ohm);

#ifdef __cplusplus
}
#endif

#endif