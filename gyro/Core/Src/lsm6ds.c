/**
 * @file lsm6ds.c
 *
 * @brief This file provide functions to get data from the sensor lsm6ds3
 *
 * @author Bruno Guo <bruno.guo@thunderatz.org>
 *
 * @date 11/2019
 */

/* Uncomment the line with the used protocol and comment the other */

#define SPI
//#define I2C

/* Uncommnet if using interrupt pins */

//#define USE_INTERRUPT

#include "lsm6ds.h"

#include <stdbool.h>

#include <string.h>
#if defined(SPI)
//#include "spi.h"
#elif defined(I2C)
#include "i2c.h"
#endif

#if defined(SPI)
#define SENSOR_BUS hspi2
#define CS_GPIO_PORT GPIOB
#define CS_PIN GPIO_PIN_12
#elif defined(I2C)
#define SENSOR_BUS hi2c1
#define I2C_INIT MX_I2C1_Init
#endif

#if defined(USE_INTERRUPT)
#define INT_GPIO_PORT_XL GPIOB
#define INT_PIN_XL GPIO_PIN_11
#define INT_GPIO_PORT_G GPIOB
#define INT_PIN_G GPIO_PIN_10
#define INT1_DRDY_XL PROPERTY_ENABLE
#define INT1_DRDY_G PROPERTY_DISABLE
#define INT2_DRDY_XL PROPERTY_DISABLE
#define INT2_DRDY_G PROPERTY_ENABLE
#endif

typedef union {
    int16_t i16bit[3];
    uint8_t u8bit[6];
} axis3bit16_t;

typedef union {
    int16_t i16bit;
    uint8_t u8bit[2];
} axis1bit16_t;

/********************************
 *  Private variables
 ********************************/

static axis3bit16_t data_raw_acceleration;
static axis3bit16_t data_raw_angular_rate;

// static axis1bit16_t data_raw_temperature;
static float acceleration_mg[3];
static float angular_rate_mdps[3];
static float temperature_degC;
static uint8_t whoamI;
uint8_t rst, error_value;
lsm6dso_stmdev_ctx_t lsm6dso_dev_ctx;
lsm6ds3_stmdev_ctx_t lsm6ds3_dev_ctx;
float_t (* acc_conversion_f)(int16_t lsm6ds_xl_fs);
float_t (* gyro_conversion_f)(int16_t lsm6ds_fs_g);
lsm6ds3_int1_route_t lsm6ds3_int_1_reg;
lsm6ds3_int2_route_t lsm6ds3_int_2_reg;

lsm6dso_pin_int1_route_t lsm6dso_int_1_reg;
lsm6dso_pin_int2_route_t lsm6dso_int_2_reg;

bool is_lsm6ds3 = false;

/********************************
 *  Private functions declarations
 ********************************/

/**
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t platform_write(void* handle, uint8_t reg, uint8_t* bufp, uint16_t len);

/**
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t platform_read(void* handle, uint8_t reg, uint8_t* bufp, uint16_t len);

/**
 * @brief  platform specific initialization (platform dependent)
 */
static void platform_init();

/********************************
 *  Public Functions Definitions
 ********************************/

int8_t lsm6ds_init(lsm6ds_settings_t lsm6ds_settings, SPI_HandleTypeDef hspi2) {
    lsm6ds3_dev_ctx.write_reg = platform_write;
    lsm6ds3_dev_ctx.read_reg = platform_read;
    lsm6ds3_dev_ctx.handle = &SENSOR_BUS;

    lsm6dso_dev_ctx.write_reg = platform_write;
    lsm6dso_dev_ctx.read_reg = platform_read;
    lsm6dso_dev_ctx.handle = &SENSOR_BUS;

    /* Initialize platform specific hardware */
    platform_init(&lsm6ds3_dev_ctx.handle);
    platform_init(&lsm6dso_dev_ctx.handle);

    /* Check device ID */
    lsm6ds3_device_id_get(&lsm6ds3_dev_ctx, &whoamI);

    //return whoamI;

    if (whoamI != LSM6DS3_ID) {
        lsm6dso_device_id_get(&lsm6dso_dev_ctx, &whoamI);

        if (whoamI != LSM6DSO_ID) {
            return LSM6DS_ERROR_WHO_AM_I;
        } else {
            is_lsm6ds3 = false;
        }
    } else {
        is_lsm6ds3 = true;
    }

    if (is_lsm6ds3) {
        /* Restore default configuration */
        lsm6ds3_reset_set(&lsm6ds3_dev_ctx, PROPERTY_ENABLE);

        do {
            lsm6ds3_reset_get(&lsm6ds3_dev_ctx, &rst);
        } while (rst);

        /* Enable Block Data Update */
        error_value = lsm6ds3_block_data_update_set(&lsm6ds3_dev_ctx, PROPERTY_ENABLE);

        if (error_value != 0) {
            return LSM6DS_ERROR_WRITE_REGISTER;
        }

        /* Set full scale */
        error_value = lsm6ds3_xl_full_scale_set(&lsm6ds3_dev_ctx, lsm6ds_settings.lsm6ds_xl_fs);

        if (error_value != 0) {
            return LSM6DS_ERROR_WRITE_REGISTER;
        }

        error_value = lsm6ds3_gy_full_scale_set(&lsm6ds3_dev_ctx, lsm6ds_settings.lsm6ds_fs_g);

        if (error_value != 0) {
            return LSM6DS_ERROR_WRITE_REGISTER;
        }

        /* Select the function to convert acc raw data to mg */
        switch (lsm6ds_settings.lsm6ds_xl_fs) {
            case 0:
                acc_conversion_f = lsm6ds3_from_fs2g_to_mg;
                break;

            case 1:
                acc_conversion_f = lsm6ds3_from_fs16g_to_mg;
                break;

            case 2:
                acc_conversion_f = lsm6ds3_from_fs4g_to_mg;
                break;

            case 3:
                acc_conversion_f = lsm6ds3_from_fs8g_to_mg;
                break;

            default:
                acc_conversion_f = lsm6ds3_from_fs2g_to_mg;
                break;
        }

        /* Select the function to convert gyro raw data to mdps */
        switch (lsm6ds_settings.lsm6ds_fs_g) {
            case 0:
                gyro_conversion_f = lsm6ds3_from_fs250dps_to_mdps;
                break;

            case 1:
                gyro_conversion_f = lsm6ds3_from_fs125dps_to_mdps;
                break;

            case 2:
                gyro_conversion_f = lsm6ds3_from_fs500dps_to_mdps;
                break;

            case 4:
                gyro_conversion_f = lsm6ds3_from_fs1000dps_to_mdps;
                break;

            case 6:
                gyro_conversion_f = lsm6ds3_from_fs2000dps_to_mdps;
                break;

            default:
                gyro_conversion_f = lsm6ds3_from_fs250dps_to_mdps;
                break;
        }

        /* Set Output Data Rate for Acc and Gyro */
        error_value = lsm6ds3_xl_data_rate_set(&lsm6ds3_dev_ctx, lsm6ds_settings.lsm6ds_odr_xl);

        if (error_value != 0) {
            return LSM6DS_ERROR_WRITE_REGISTER;
        }

        error_value = lsm6ds3_gy_data_rate_set(&lsm6ds3_dev_ctx, lsm6ds_settings.lsm6ds_odr_g);

        if (error_value != 0) {
            return LSM6DS_ERROR_WRITE_REGISTER;
        }

#if defined(USE_INTERRUPT)

        /* Enable interrupt generation on DRDY INT1 and INT2 pin */
        lsm6ds3_pin_int1_route_get(&lsm6ds3_dev_ctx, &lsm6ds3_int_1_reg);
        lsm6ds3_int_1_reg.int1_drdy_xl = INT1_DRDY_XL;
        lsm6ds3_int_1_reg.int1_drdy_g = INT1_DRDY_G;
        lsm6ds3_pin_int1_route_set(&lsm6ds3_dev_ctx, &lsm6ds3_int_1_reg);

        lsm6ds3_pin_int2_route_get(&lsm6ds3_dev_ctx, &lsm6ds3_int_2_reg);
        lsm6ds3_int_2_reg.int2_drdy_xl = INT2_DRDY_XL;
        lsm6ds3_int_2_reg.int2_drdy_g = INT2_DRDY_G;
        lsm6ds3_pin_int2_route_set(&lsm6ds3_dev_ctx, &lsm6ds3_int_2_reg);
#endif

        return LSM6DS_ERROR_NONE;
    } else {
        /* Restore default configuration */
        lsm6dso_reset_set(&lsm6dso_dev_ctx, PROPERTY_ENABLE);

        do {
            lsm6dso_reset_get(&lsm6dso_dev_ctx, &rst);
        } while (rst);

        /* Enable Block Data Update */
        error_value = lsm6dso_block_data_update_set(&lsm6dso_dev_ctx, PROPERTY_ENABLE);

        if (error_value != 0) {
            return LSM6DS_ERROR_WRITE_REGISTER;
        }

        /* Set full scale */
        error_value = lsm6dso_xl_full_scale_set(&lsm6dso_dev_ctx, lsm6ds_settings.lsm6ds_xl_fs);

        if (error_value != 0) {
            return LSM6DS_ERROR_WRITE_REGISTER;
        }

        error_value = lsm6dso_gy_full_scale_set(&lsm6dso_dev_ctx, lsm6ds_settings.lsm6ds_fs_g);

        if (error_value != 0) {
            return LSM6DS_ERROR_WRITE_REGISTER;
        }

        /* Select the function to convert acc raw data to mg */
        switch (lsm6ds_settings.lsm6ds_xl_fs) {
            case 0:
                acc_conversion_f = lsm6dso_from_fs2_to_mg;
                break;

            case 1:
                acc_conversion_f = lsm6dso_from_fs16_to_mg;
                break;

            case 2:
                acc_conversion_f = lsm6dso_from_fs4_to_mg;
                break;

            case 3:
                acc_conversion_f = lsm6dso_from_fs8_to_mg;
                break;

            default:
                acc_conversion_f = lsm6dso_from_fs2_to_mg;
                break;
        }

        /* Select the function to convert gyro raw data to mdps */
        switch (lsm6ds_settings.lsm6ds_fs_g) {
            case 0:
                gyro_conversion_f = lsm6dso_from_fs250_to_mdps;
                break;

            case 1:
                gyro_conversion_f = lsm6dso_from_fs125_to_mdps;
                break;

            case 2:
                gyro_conversion_f = lsm6dso_from_fs500_to_mdps;
                break;

            case 4:
                gyro_conversion_f = lsm6dso_from_fs1000_to_mdps;
                break;

            case 6:
                gyro_conversion_f = lsm6dso_from_fs2000_to_mdps;
                break;

            default:
                gyro_conversion_f = lsm6dso_from_fs250_to_mdps;
                break;
        }

        /* Set Output Data Rate for Acc and Gyro */
        error_value = lsm6dso_xl_data_rate_set(&lsm6dso_dev_ctx, lsm6ds_settings.lsm6ds_odr_xl);

        if (error_value != 0) {
            return LSM6DS_ERROR_WRITE_REGISTER;
        }

        error_value = lsm6dso_gy_data_rate_set(&lsm6dso_dev_ctx, lsm6ds_settings.lsm6ds_odr_g);

        if (error_value != 0) {
            return LSM6DS_ERROR_WRITE_REGISTER;
        }
/*
#if defined(USE_INTERRUPT)


        lsm6dso_pin_int1_route_get(&lsm6dso_dev_ctx, &lsm6dso_int_1_reg);
        lsm6dso_int_1_reg.int1_drdy_xl = INT1_DRDY_XL;
        lsm6dso_int_1_reg.int1_drdy_g = INT1_DRDY_G;
        lsm6dso_pin_int1_route_set(&lsm6dso_dev_ctx, &lsm6dso_int_1_reg);

        lsm6dso_pin_int2_route_get(&lsm6dso_dev_ctx, &lsm6dso_int_2_reg);
        lsm6dso_int_2_reg.int2_drdy_xl = INT2_DRDY_XL;
        lsm6dso_int_2_reg.int2_drdy_g = INT2_DRDY_G;
        lsm6dso_pin_int2_route_set(&lsm6dso_dev_ctx, &lsm6dso_int_2_reg);
#endif
*/
        return LSM6DS_ERROR_NONE;
    }
}

void lsm6ds_update_data() {
    if (is_lsm6ds3) {
        uint8_t reg;

        /* Read output only if new value is available */
        lsm6ds3_xl_flag_data_ready_get(&lsm6ds3_dev_ctx, &reg);

        if (reg) {
            /* Read acceleration field data */
            memset(data_raw_acceleration.u8bit, 0x00, 3 * sizeof(int16_t));
            lsm6ds3_acceleration_raw_get(&lsm6ds3_dev_ctx, data_raw_acceleration.u8bit);

            for (int i = 0; i < 3; i++) {
                acceleration_mg[i] = acc_conversion_f(data_raw_acceleration.i16bit[i]);
            }
        }

        lsm6ds3_gy_flag_data_ready_get(&lsm6ds3_dev_ctx, &reg);

        if (reg) {
            /* Read angular rate field data */
            memset(data_raw_angular_rate.u8bit, 0x00, 3 * sizeof(int16_t));
            lsm6ds3_angular_rate_raw_get(&lsm6ds3_dev_ctx, data_raw_angular_rate.u8bit);

            for (int j = 0; j < 3; j++) {
                angular_rate_mdps[j] = gyro_conversion_f(data_raw_angular_rate.i16bit[j]);
            }
        }

        // lsm6ds3_temp_flag_data_ready_get(&lsm6ds3_dev_ctx, &reg);
        // if (reg) {
        /// * Read temperature data */
        // memset(data_raw_temperature.u8bit, 0x00, sizeof(int16_t));
        // lsm6ds3_temperature_raw_get(&lsm6ds3_dev_ctx, data_raw_temperature.u8bit);
        // temperature_degC = lsm6ds3_from_lsb_to_celsius(data_raw_temperature.i16bit);
        // }
    } else {
        uint8_t reg;

        /* Read output only if new value is available */
        lsm6dso_xl_flag_data_ready_get(&lsm6dso_dev_ctx, &reg);

        if (reg) {
            /* Read acceleration field data */
            memset(data_raw_acceleration.u8bit, 0x00, 3 * sizeof(int16_t));
            lsm6dso_acceleration_raw_get(&lsm6dso_dev_ctx, data_raw_acceleration.u8bit);

            for (int i = 0; i < 3; i++) {
                acceleration_mg[i] = acc_conversion_f(data_raw_acceleration.i16bit[i]);
            }
        }

        lsm6dso_gy_flag_data_ready_get(&lsm6dso_dev_ctx, &reg);

        if (reg) {
            /* Read angular rate field data */
            memset(data_raw_angular_rate.u8bit, 0x00, 3 * sizeof(int16_t));
            lsm6dso_angular_rate_raw_get(&lsm6dso_dev_ctx, data_raw_angular_rate.u8bit);

            for (int j = 0; j < 3; j++) {
                angular_rate_mdps[j] = gyro_conversion_f(data_raw_angular_rate.i16bit[j]);
            }
        }

        // lsm6dso_temp_flag_data_ready_get(&lsm6dso_dev_ctx, &reg);
        // if (reg) {
        /// * Read temperature data */
        // memset(data_raw_temperature.u8bit, 0x00, sizeof(int16_t));
        // lsm6dso_temperature_raw_get(&lsm6dso_dev_ctx, data_raw_temperature.u8bit);
        // temperature_degC = lsm6dso_from_lsb_to_celsius(data_raw_temperature.i16bit);
        // }
    }
}

#if defined(USE_INTERRUPT)
void lsm6ds_update_data_interrupt() {
    if (is_lsm6ds3) {
        /* Read acceleration field data when corresponding INT is HIGH */
        if (HAL_GPIO_ReadPin(INT_GPIO_PORT_XL, INT_PIN_XL) == 1) {
            memset(data_raw_acceleration.u8bit, 0x00, 3 * sizeof(int16_t));
            lsm6ds3_acceleration_raw_get(&lsm6ds3_dev_ctx, data_raw_acceleration.u8bit);

            for (int i = 0; i < 3; i++) {
                acceleration_mg[i] = acc_conversion_f(data_raw_acceleration.i16bit[i]);
            }
        }

        /* Read angular rate field data when corresponding INT is HIGH */
        if (HAL_GPIO_ReadPin(INT_GPIO_PORT_G, INT_PIN_G) == 1) {
            memset(data_raw_angular_rate.u8bit, 0x00, 3 * sizeof(int16_t));
            lsm6ds3_angular_rate_raw_get(&lsm6ds3_dev_ctx, data_raw_angular_rate.u8bit);

            for (int i = 0; i < 3; i++) {
                angular_rate_mdps[i] = gyro_conversion_f(data_raw_angular_rate.i16bit[i]);
            }
        }
    } else {
        /* Read acceleration field data when corresponding INT is HIGH */
        if (HAL_GPIO_ReadPin(INT_GPIO_PORT_XL, INT_PIN_XL) == 1) {
            memset(data_raw_acceleration.u8bit, 0x00, 3 * sizeof(int16_t));
            lsm6dso_acceleration_raw_get(&lsm6dso_dev_ctx, data_raw_acceleration.u8bit);

            for (int i = 0; i < 3; i++) {
                acceleration_mg[i] = acc_conversion_f(data_raw_acceleration.i16bit[i]);
            }
        }

        /* Read angular rate field data when corresponding INT is HIGH */
        if (HAL_GPIO_ReadPin(INT_GPIO_PORT_G, INT_PIN_G) == 1) {
            memset(data_raw_angular_rate.u8bit, 0x00, 3 * sizeof(int16_t));
            lsm6dso_angular_rate_raw_get(&lsm6dso_dev_ctx, data_raw_angular_rate.u8bit);

            for (int i = 0; i < 3; i++) {
                angular_rate_mdps[i] = gyro_conversion_f(data_raw_angular_rate.i16bit[i]);
            }
        }
    }
}

#endif

float* lsm6ds_get_acc_data_mg() {
    return acceleration_mg;
}

float* lsm6ds_get_gyro_data_mdps() {
    return angular_rate_mdps;
}

float lsm6ds_get_temperature_degC() {
    return temperature_degC;
}

/********************************
 *  Private Functions Definitions
 ********************************/

static int32_t platform_write(void* handle, uint8_t reg, uint8_t* bufp, uint16_t len) {
    if (is_lsm6ds3) {
#if defined(I2C)
        HAL_I2C_Mem_Write(handle, LSM6DS3_I2C_ADD_H, reg, I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
#elif defined(SPI)
        HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_RESET);
        HAL_SPI_Transmit(handle, &reg, 1, 1000);
        HAL_SPI_Transmit(handle, bufp, len, 1000);
        HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_SET);
#endif
        return 0;
    } else {
#if defined(I2C)
        HAL_I2C_Mem_Write(handle, LSM6DSO_I2C_ADD_H, reg, I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
#elif defined(SPI)
        HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_RESET);
        HAL_SPI_Transmit(handle, &reg, 1, 1000);
        HAL_SPI_Transmit(handle, bufp, len, 1000);
        HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_SET);
#endif
        return 0;
    }
}

static int32_t platform_read(void* handle, uint8_t reg, uint8_t* bufp, uint16_t len) {
    if (is_lsm6ds3) {
#if defined(I2C)
        HAL_I2C_Mem_Read(handle, LSM6DS3_I2C_ADD_H, reg, I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
#elif defined(SPI)

        /* MSB must be 1 when reading */
        reg |= 0x80;
        HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_RESET);
        HAL_SPI_Transmit(handle, &reg, 1, 1000);
        HAL_SPI_Receive(handle, bufp, len, 1000);
        HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_SET);
#endif
        return 0;
    } else {
#if defined(I2C)
        HAL_I2C_Mem_Read(handle, LSM6DSO_I2C_ADD_H, reg, I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
#elif defined(SPI)

        /* MSB must be 1 when reading */
        reg |= 0x80;
        HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_RESET);
        HAL_SPI_Transmit(handle, &reg, 1, 1000);
        HAL_SPI_Receive(handle, bufp, len, 1000);
        HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_SET);
#endif
        return 0;
    }
}

static void platform_init() {
#if defined(I2C)
    I2C_INIT();
#elif defined(SPI)
    //SPI_INIT();
#endif
}
