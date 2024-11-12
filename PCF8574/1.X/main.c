/*
 * File:   main.c
 * Author: LENOVO
 *
 * Created on 4 de Junho de 2021, 16:26
 */


#include <xc.h>

#pragma config FOSC = INTOSC_HS
#pragma config WDT = OFF
#pragma config LVP = OFF

#define _XTAL_FREQ 8000000
#define time 10
#define SlaveAddress1 0x4E

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
//I2C
void settings(void);
void WriteI2C(unsigned char Address, unsigned char Data);
void StartI2C(void);
void ReadyI2C(void);
void SendI2C(unsigned char data);
void StopI2C(void);
void ReadI2C(unsigned char Address, unsigned char Register, unsigned char *Data, int bytes);
void RepeatedStartI2C(void);
unsigned char ReceiveI2C(char flag);

int i;

void main(void) {
    settings();
    WriteI2C(SlaveAddress1,0x00);
    //nibbles (0x02)
    WriteI2C(SlaveAddress1,0x04);//E=1
    WriteI2C(SlaveAddress1,(0x04&~0x04));//E=0
    WriteI2C(SlaveAddress1,0x24);
    WriteI2C(SlaveAddress1,(0x24&~0x04));
    //EMS (0x06)
    WriteI2C(SlaveAddress1,0x04);
    WriteI2C(SlaveAddress1,(0x04&~0x04));
    WriteI2C(SlaveAddress1,0x64);
    WriteI2C(SlaveAddress1,(0x64&~0x04));
    //DC (0x0F)
    WriteI2C(SlaveAddress1,0x04);
    WriteI2C(SlaveAddress1,(0x04&~0x04));
    WriteI2C(SlaveAddress1,0xF4);
    WriteI2C(SlaveAddress1,(0xF4&~0x04));
    //FS (0x28)
    WriteI2C(SlaveAddress1,0x24);
    WriteI2C(SlaveAddress1,(0x24&~0x04));
    WriteI2C(SlaveAddress1,0x84);
    WriteI2C(SlaveAddress1,(0x84&~0x04));
    //CD (0x01)
    WriteI2C(SlaveAddress1,0x04);
    WriteI2C(SlaveAddress1,(0x04&~0x04));
    WriteI2C(SlaveAddress1,0x14);
    WriteI2C(SlaveAddress1,(0x14&~0x04));
    //F (0x46) RS=1
    WriteI2C(SlaveAddress1,0x45);
    WriteI2C(SlaveAddress1,(0x45&~0x04));
    WriteI2C(SlaveAddress1,0x65);
    WriteI2C(SlaveAddress1,(0x65&~0x04));
    //DC (0x0E)
    WriteI2C(SlaveAddress1,0x04);
    WriteI2C(SlaveAddress1,(0x04&~0x04));
    WriteI2C(SlaveAddress1,0xE4);
    WriteI2C(SlaveAddress1,(0xE4&~0x04));
    while(1){
        
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
//    TRISD = 0x00;
//    LATD = 0x00;
//    TRISE = 0x00;
//    LATE = 0x00;
//    SettingsLCD(0x02);
//    SettingsLCD(EMS);
//    SettingsLCD(DC);
//    SettingsLCD(FS);
//    SettingsLCD(CD);   
}


void WriteI2C(unsigned char Address, unsigned char Data){
    StartI2C();
    SendI2C(Address);
    SendI2C(Data);
    StopI2C();
    __delay_ms(100);
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