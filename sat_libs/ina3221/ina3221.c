#include "ina3221.h"

static void ina_read_register(ina3221_t *config, uint8_t addr, uint16_t *buf, uint8_t len)
{
	uint8_t pbuf[2 * len];
	i2c_write_blocking(config->inst, config->addr, &addr, 1, true);
	i2c_read_blocking(config->inst, config->addr, pbuf, sizeof(pbuf), false);
	*buf = (uint16_t)(pbuf[0] << 8 | pbuf[1]);
}

static void ina_write_register(ina3221_t *config, uint8_t addr, uint16_t reg)
{
	uint8_t u_reg = (reg >> 8) & 0xFF;
	uint8_t l_reg = reg & 0xFF;
	uint8_t buf[] = {addr, u_reg, l_reg};
	i2c_write_blocking(config->inst, config->addr, buf, 3, false);
}

static uint8_t verify_id(ina3221_t *config)
{
	uint16_t id = 0;
	ina_read_register(config, __MANUFACTURER_ID, &id, 1);
	return id == 0x5449? 0 : 1;
}

uint16_t ina3221_get_voltage_raw(ina3221_t *config, uint8_t channel)
{
	uint8_t busvoltage = 0;

	switch (channel)
	{
	case 1:
		busvoltage = __REG_BUSVOLTAGE_1;
		break;
	case 2:
		busvoltage = __REG_BUSVOLTAGE_2;
		break;
	case 3:
		busvoltage = __REG_BUSVOLTAGE_3;
		break;
	default:
		return 34404;
	}

	uint16_t buf;

	ina_read_register(config, busvoltage, &buf, 1);

	return buf;
}

uint16_t ina3221_get_shunt_voltage_raw(ina3221_t *config, uint8_t channel)
{
	uint8_t busvoltage = 0;

	switch (channel)
	{
	case 1:
		busvoltage = __REG_SHUNTVOLTAGE_1;
		break;
	case 2:
		busvoltage = __REG_SHUNTVOLTAGE_2;
		break;
	case 3:
		busvoltage = __REG_SHUNTVOLTAGE_3;
		break;
	default:
		return 34404;
	}

	uint16_t buf;

	ina_read_register(config, busvoltage, &buf, 1);

	return buf;
}

float ina3221_get_voltage(ina3221_t *config, uint8_t channel)
{
	return (float)(ina3221_get_voltage_raw(config, channel) >> 3) * 0.008f;
}

float ina3221_get_current(ina3221_t *config, uint8_t channel)
{
	return (float)(ina3221_get_shunt_voltage_raw(config, channel) >> 3) * 0.04f / config->shunt_m_ohm;
}

float ina3221_get_power(ina3221_t *config, uint8_t channel)
{
	return (float)(ina3221_get_current(config, channel) * ina3221_get_voltage(config, channel));
}

error_t ina3221_init(ina3221_t *config, int8_t addr, i2c_inst_t *inst, uint sda, uint scl, uint32_t frequency, uint16_t shunt_m_ohm)
{
	config->addr = addr;
	config->inst = inst;
	config->shunt_m_ohm = shunt_m_ohm;

	i2c_init(inst, frequency);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);

	uint16_t buf = (RESET_SYSTEM << 15) |
				(ALL_CHANNEL << 12) |
				(AVG2_3 << 9) |
				(Vbus_CTV2_4 << 6) |
				(Vsh_CTV2_4 << 3) |
				(__MODE7);

	ina_write_register(config, __REG_CONFIG, buf);

	if (verify_id(config))
		return 1;	

	return 0;
}