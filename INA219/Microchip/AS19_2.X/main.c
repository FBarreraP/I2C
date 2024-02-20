/*
 * File:   main.c
 * Author: LENOVO
 *
 * Created on 9 de Dezembro de 2020, 17:17
 */

#include <xc.h>
#include <string.h>
#include <stdio.h>

#pragma config FOSC = INTOSC_HS
#pragma config WDT = OFF
#pragma config LVP = OFF

#define _XTAL_FREQ 8000000
#define time 10
#define SlaveAddress1 0xA0
#define SlaveAddress2 0xA2
//LCD
#define CD 0x01 //Clear Display
#define RH 0x02 //(0x03) Return Home
#define EMS 0x06 //Entry Mode Set
#define DC 0x0F //(0x0E) Display Control
#define DSr 0x1C //Display Shift Rigth
#define DSl 0x18 //Display Shift Left
#define FS 0x28 //(0x3C) Function Set
#define RAW1 0x80 //DDRAM display
#define RAW2 0xC0 //DDRAM display
#define RS LATE1 //Register Selection
#define E LATE0 //Enable

void settings(void);
void WriteI2C(unsigned char Address, unsigned char Register, unsigned char *Data, int bytes);
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
void Data1LCD(void);
void Data2LCD(void);

unsigned char dataSaved[9], name[6] = {'f','A','b','I','a','N'};
int i;
char texto1[] = {"EEPROM:"},texto2[20];

void main(void) {
    settings();
    Data1LCD();
    WriteI2C(SlaveAddress1,0xB5,name,6);
    WriteI2C(SlaveAddress1,0xFF,&name[2],1);
    ReadI2C(SlaveAddress1,0xB3,dataSaved,9);
    Data2LCD();
    while(1){
        
    }
}

void settings(void){
    OSCCON = 0x72;
    ADCON1 = 0x0F;
    TRISBbits.RB0 = 1;
    TRISBbits.RB1 = 1;
    SSPSTAT = 0x80;
    SSPCON1 = 0x28;
    SSPCON2 = 0;
    SSPADD = 0x13;   
    //LCD
    TRISD = 0x00;
    LATD = 0x00;
    TRISE = 0x00;
    LATE = 0x00;
    SettingsLCD(0x02);
    SettingsLCD(EMS);
    SettingsLCD(DC);
    SettingsLCD(FS);
    SettingsLCD(CD);   
}

void WriteI2C(unsigned char Address, unsigned char Register, unsigned char *Data, int bytes){
    StartI2C();
    SendI2C(Address);
    SendI2C(Register);
    for(i=0; i<bytes; i++){
        SendI2C(*Data);
        Data++;
    }
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
    __delay_ms(time);
    LATD = data;
    __delay_ms(time);
    E = 0;
    __delay_ms(time);
}

void Data1LCD(void){
    SettingsLCD(RAW1);
    for(i=0; i<strlen(texto1); i++){
        WriteLCD(texto1[i]);
    }
}

void Data2LCD(void){
    SettingsLCD(RAW2);
    sprintf(texto2,"%x,%x,%d,%c,%X",dataSaved[0],dataSaved[1],dataSaved[2],dataSaved[3],dataSaved[4]);
    for(i=0; i<strlen(texto2); i++){
        WriteLCD(texto2[i]);
    }
}
