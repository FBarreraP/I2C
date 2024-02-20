/*
 * File:   main.c
 * Author: LENOVO
 *
 * Created on 9 de Março de 2021, 20:18
 */

#include <xc.h>
#include <stdio.h>
#include <string.h>

#pragma config FOSC = INTOSC_HS
#pragma config WDT = OFF
#pragma config LVP = OFF

#define _XTAL_FREQ 8000000
#define time 10
#define SlaveAddress 0x80
//LCD
#define CD 0x01
#define RH 0x02
#define EMS 0x06
#define DC 0x0F
#define DSr 0x1C
#define DSl 0x18
#define FS 0x28
#define RAW1 0x80
#define RAW2 0xC0
#define E LATE0
#define RS LATE1
//INA219
#define INA219_REG_CALIBRATION 0x05 //CALIBRATION REGISTER (R/W)
#define INA219_REG_CONFIG 0x00
#define INA219_REG_BUSVOLTAGE 0x02 // BUS VOLTAJE REGISTER (R)
#define INA219_REG_SHUNTVOLTAGE 0x01
#define INA219_CONFIG_BVOLTAGERANGE_32V 0x2000  // 0-32V Range
#define INA219_CONFIG_GAIN_8_320MV 0x1800 // Gain 8, 320mV Range
#define INA219_CONFIG_BADCRES_12BIT 0x0400  // 12-bit bus res = 0..4097
#define INA219_CONFIG_SADCRES_12BIT_1S_532US 0x0018 // 1 x 12-bit shunt sample
#define INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS 0x0007 

//I2C
void settings(void);
void WriteI2C(unsigned char Address, unsigned char Register, unsigned int *Data, int bytes);
void StartI2C(void);
void ReadyI2C(void);
void SendI2C(unsigned char data);
void StopI2C(void);
void ReadI2C(unsigned char Address, unsigned char Register, unsigned char *Data, int bytes);
void RepeatedStartI2C(void);
unsigned char ReceiveI2C(char flag);
//LCD
void SettingsLCD(unsigned char word);
void WriteLCD(unsigned char word);
void LCD(unsigned char data);
//INA219
void setCalibration_32V_2A(void);
float getBusVoltage_V(void);
int getBusVoltage_raw(void);
float getShuntVoltage_mV(void);
int getShuntVoltage_raw(void);

unsigned int ina219_calValue;
unsigned char ina219_currentDivider_mA, ina219_powerDivider_mW;
int i;
float busVoltage_V, shuntVoltage_mV, voltage_V = 0;
char text[15];

void main(void) {
    settings();
    setCalibration_32V_2A();
    while(1){
        busVoltage_V = getBusVoltage_V();
        shuntVoltage_mV = getShuntVoltage_mV(); 
        voltage_V = busVoltage_V + (shuntVoltage_mV / 1000);
        sprintf(text,"%.4f V",voltage_V);
        SettingsLCD(RAW2);
        for(i=0; i<strlen(text); i++){
            WriteLCD(text[i]);
        }
    }
}

void settings(void){
    OSCCON = 0x72;
    ADCON1 = 0x0F;
    //I2C
    TRISBbits.RB0 = 1;
    TRISBbits.RB1 = 1;
    SSPSTAT = 0x80;
    SSPCON1 = 0x28;
    SSPCON2 = 0;
    SSPADD = 0x13;   
    //LCD
    TRISE = 0;
    LATE = 0;
    TRISD = 0;
    LATD = 0;
    SettingsLCD(0x02); //Iniciar la LCD con el método nibble (4 MSB y 4 LSB)
    SettingsLCD(EMS);
    SettingsLCD(DC);
    SettingsLCD(FS);
    SettingsLCD(CD);
}

void WriteI2C(unsigned char Address, unsigned char Register, unsigned int *Data, int bytes){
    StartI2C();
    SendI2C(Address);
    SendI2C(Register);
    SendI2C((*Data>>8)&0xFF);
    SendI2C(*Data&0xFF);
//    for(i=0; i<bytes; i++){
//        SendI2C(*Data);
//        Data++;
//    }
    StopI2C();
    __delay_ms(time*100);
}

void StartI2C(void){
    ReadyI2C();
    SEN = 1;
}

void ReadyI2C(void){
    while((SSPCON2 & 0x1F) || (SSPSTAT & 0x04));
}

void SendI2C(unsigned char data){
    ReadyI2C();
    SSPBUF = data;
}

void StopI2C(void){
    ReadyI2C();
    PEN = 1;
}

void ReadI2C(unsigned char Address, unsigned char Register, unsigned char *Data, int bytes){
    StartI2C();
    SendI2C(Address);
    SendI2C(Register);
    RepeatedStartI2C();
    SendI2C(Address|0x01);
    for(i=0; i<bytes; i++){
        if(i+1 == bytes){
            *Data = ReceiveI2C(1);
        }else{
            *Data = ReceiveI2C(0);
        }
        Data++;
    }
    StopI2C();
    __delay_ms(time*100);
}

void RepeatedStartI2C(void){
    ReadyI2C();
    RSEN = 1;
}

unsigned char ReceiveI2C(char flag){
    unsigned char buffer;
    ReadyI2C();
    RCEN = 1;
    ReadyI2C();
    buffer = SSPBUF;
    ReadyI2C();
    ACKDT = flag==1? 1 : 0;
    ACKEN = 1;
    return buffer;
}

void SettingsLCD(unsigned char word){
    RS = 0;
    LCD(word >> 4); // 4 MSB
    LCD(word & 0x0F); // 4 LSB
}

void WriteLCD(unsigned char word){
    RS = 1;
    LCD(word >> 4);
    LCD(word & 0x0F);
}

void LCD(unsigned char data){ //Opción bits
    E = 1;
    __delay_us(time);
    LATD = data;
    __delay_us(time);
    E = 0;
    __delay_us(time);
}

void setCalibration_32V_2A(void){
    unsigned int config;
    ina219_calValue = 4096;
    ina219_currentDivider_mA = 10;
    ina219_powerDivider_mW = 2;
    WriteI2C(SlaveAddress,INA219_REG_CALIBRATION,&ina219_calValue,2);
    config = INA219_CONFIG_BVOLTAGERANGE_32V | INA219_CONFIG_GAIN_8_320MV |
            INA219_CONFIG_BADCRES_12BIT |
            INA219_CONFIG_SADCRES_12BIT_1S_532US |
            INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
    WriteI2C(SlaveAddress,INA219_REG_CONFIG,&config,2);
}

float getBusVoltage_V(void) {
    unsigned int value = getBusVoltage_raw();
    return value * 0.001;
}

int getBusVoltage_raw(void) {
    unsigned char value[2];
    unsigned int temp;
    ReadI2C(SlaveAddress, INA219_REG_BUSVOLTAGE, value, 2);
    temp = ((value[0] << 8) | value[1]);
    // Shift to the right 3 to drop CNVR and OVF and multiply by LSB
    return ((temp >> 3) * 4);
}

float getShuntVoltage_mV(void) {
    unsigned int value;
    value = getShuntVoltage_raw();
    return value * 0.01;
}

int getShuntVoltage_raw(void) {
    unsigned char value[2];
    unsigned int temp;
    ReadI2C(SlaveAddress, INA219_REG_SHUNTVOLTAGE, value, 2);
    temp = ((value[0] << 8) | value[1]);
    return temp;
}