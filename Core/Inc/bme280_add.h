/*
 * bme28_add.h
 *
 *  Created on: Dec 27, 2024
 *      Author: dominik
 */

#ifndef INC_BME280_ADD_H_
#define INC_BME280_ADD_H_

int8_t BME280_init(void);
float BME280_read_temperature(void);
void delay_ms(uint32_t period);
int8_t i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
int8_t i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);


#endif
