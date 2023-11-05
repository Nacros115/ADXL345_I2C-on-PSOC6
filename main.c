#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include <assert.h>
#include <stdio.h>

cyhal_i2c_t mI2C;

// Déclaration des adresses du module
#define ADXL345_Adresse 0x53 // adresse de l'ADXL345
#define POWER_CTL 0x2D // registre Power Control
#define DATA_FORMAT 0x31 // registre Data Format
#define DATAX0 0x32 // bit de poids faible axe X
#define DATAX1 0x33 // bit de poids fort axe X
#define DATAY0 0x34 // bit de poids faible axe Y
#define DATAY1 0x35 // bit de poids fort axe Y
#define DATAZ0 0x36 // bit de poids faible axe Z
#define DATAZ1 0x37 // bit de poids fort axe Z

// Configuration du module
#define ADXL345_Precision2G 0x00
#define ADXL345_Precision4G 0x01
#define ADXL345_Precision8G 0x02
#define ADXL345_Precision16G 0x03
#define ADXL345_ModeMesure 0x08


void writeToADXL345(uint8_t regAddress, uint8_t value) {
    uint8_t buffer[2];
    buffer[0] = regAddress;
    buffer[1] = value;
    cyhal_i2c_master_write(&mI2C, ADXL345_Adresse, buffer, 2, 0, false);
}

void readFromADXL345(uint8_t regAddress, uint8_t* data, uint8_t len) {
    cyhal_i2c_master_write(&mI2C, ADXL345_Adresse, &regAddress, 1, 0, false);
    cyhal_i2c_master_read(&mI2C, ADXL345_Adresse, data, len, 0, false);
}

int main(void) {
    cy_rslt_t result;
    cyhal_i2c_cfg_t mI2C_cfg;
    uint8_t buffer[6]; // stockage des données du module

    int composante_X;
    int composante_Y;
    int composante_Z;

    result = cybsp_init();
    assert(result == CY_RSLT_SUCCESS);

    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
    assert(result == CY_RSLT_SUCCESS);

    printf("\x1b[2J\x1b[;H");

    printf("ADXL345 Sensor Data\n");

    printf(">> Configuring I2C Master..... ");
    mI2C_cfg.is_slave = false;
    mI2C_cfg.frequencyhal_hz = 100000;

    result = cyhal_i2c_init(&mI2C, CYBSP_I2C_SDA, CYBSP_I2C_SCL, NULL);
    assert(result == CY_RSLT_SUCCESS);

    result = cyhal_i2c_configure(&mI2C, &mI2C_cfg);
    assert(result == CY_RSLT_SUCCESS);

    printf("Done\r\n\n");

    __enable_irq();

    // Configure the ADXL345 module
    writeToADXL345(0x31, 0x01); // Set data format to 4g
    writeToADXL345(0x2D, 0x08); // Set to measurement mode

    while (1) {
        readFromADXL345(0x32, buffer, 6);

        composante_X=(buffer[1] << 8) | buffer[0]; // élaboration des 3 composantes
        composante_Y=(buffer[3] << 8) | buffer[2];
        composante_Z=(buffer[5] << 8) | buffer[4];

        printf("X=%d\tY=%d\tZ=%d\n", composante_X, composante_Y, composante_Z);
        cyhal_system_delay_ms(1000);
    }
}
