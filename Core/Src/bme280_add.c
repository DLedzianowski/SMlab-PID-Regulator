/*
 * bme280_add.c
 *
 *  Created on: Dec 27, 2024
 *      Author: dominik
 */

#include "bme280.h"
#include "bme280_add.h"
#include "i2c.h"
#include <stdio.h>

static struct bme280_dev bme;
static struct bme280_data comp_data;
static int8_t init_done;

int8_t BME280_init(void) {
	int8_t rslt = BME280_OK;
	uint8_t settings_sel;
	init_done = BME280_E_DEV_NOT_FOUND;

	bme.dev_id = (BME280_I2C_ADDR_PRIM<<1);
	bme.intf = BME280_I2C_INTF;
	bme.read = i2c_read;
	bme.write = i2c_write;
	bme.delay_ms = delay_ms;

	rslt = bme280_init(&bme);

	/* Recommended mode of operation: Indoor navigation */
	bme.settings.osr_h = BME280_OVERSAMPLING_1X;
	bme.settings.osr_p = BME280_OVERSAMPLING_16X;
	bme.settings.osr_t = BME280_OVERSAMPLING_2X;
	bme.settings.filter = BME280_FILTER_COEFF_16;
	bme.settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

	if(rslt == BME280_OK)
	{
		settings_sel = BME280_OSR_PRESS_SEL;
		settings_sel |= BME280_OSR_TEMP_SEL;
		settings_sel |= BME280_OSR_HUM_SEL;
		settings_sel |= BME280_STANDBY_SEL;
		settings_sel |= BME280_FILTER_SEL;

		rslt = bme280_set_sensor_settings(settings_sel, &bme);

		if(rslt == BME280_OK)
		{
			rslt = bme280_set_sensor_mode(BME280_NORMAL_MODE, &bme);
			init_done = rslt;
		}
	}

	return rslt;
}

float BME280_read_temperature(void) {
    if (init_done == BME280_OK) {
        // tylko temperatura
        bme280_get_sensor_data(BME280_TEMP, &comp_data, &bme);
    }
    return comp_data.temperature;
}

void delay_ms(uint32_t period) {
	HAL_Delay(period);
}

int8_t i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */
    HAL_I2C_Mem_Read(&hi2c1, dev_id, reg_addr, 1, reg_data, len, 100);
    return rslt;
}

int8_t i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */
    HAL_I2C_Mem_Write(&hi2c1, dev_id, reg_addr, 1, reg_data, len, 100);
    return rslt;
}
