#ifndef PINCONFIG_H
#define PINCONFIG_H

const int ADS_RDY = 39; // ADS1115 ready pin at SENSOR_VN
const int BAT = 34; // Read battery voltage
const int C_SDA = 32; // I2C data
const int C_SCL = 33; // I2C clock
const int VBAT = 25; // High to enable battery voltage measurement
const int PA_1 = 26; // Pump at AIN1 (PWM) 3V to H1 
const int HB_1 = 27; // Heater at BIN1 (PWM) 5V to H4
const int V1_8 = 14; // 1.8V enable for gas sensor heater
const int SOL = 23; // Solenoid valve
const int LCD_BL = 22; // LCD backlight
const int SPI3_MISO = 21; // SPI3 MISO
const int SPI3_MOSI = 19; // SPI3 MOSI
const int LCD_CS = 18; // LCD chip select
const int LCD_DC = 5; // LCD data/command
const int SPI3_CLK = 4; // SPI3 clock
const int LCD_RST = 2; // LCD reset
const int TCH_CS = 15; // Touch chip select

const int PumpPWM = 0; // PWM channel for pump
const int HeaterPWM = 1; // PWM channel for heater
const int SolenoidPWM = 2; // PWM channel for solenoid valve
const int PumpFREQ = 20000;
const int HeaterFREQ = 20000;
const int SolenoidFREQ = 20000;
const int pwmRES = 8;

#endif // PINCONFIG_H