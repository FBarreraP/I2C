/*
 * File:   main.c
 * Author: LENOVO
 *
 * Created on 27 de Fevereiro de 2021, 11:18
 */

#include <xc.h>

#pragma config FOSC = INTOSC_HS
#pragma config WDT = OFF
#pragma config LVP = OFF

#define _XTAL_FREQ 8000000
#define time 10
#define SlaveAddress 0x30

void settings(void);
void WriteI2C(unsigned char Address, unsigned char Register, unsigned char *Data, int bytes);
void StartI2C(void);
void ReadyI2C(void);
void SendI2C(unsigned char data);
void StopI2C(void);
void ReadI2C(unsigned char Address, unsigned char Register, unsigned char *Data, int bytes);
void RepeatedStartI2C(void);
unsigned char ReceiveI2C(char flag);

unsigned char dataSaved[9], name[2] = {0x01, 0x00};
int i;

void main(void) {
    settings();

    //WriteI2C(SlaveAddress,0xFF,&name[2],1);
    //ReadI2C(SlaveAddress,0xB3,dataSaved,9);
    while (1) {
        WriteI2C(SlaveAddress, 0xB5, &name[0], 1);
        __delay_ms(time * 10);
        WriteI2C(SlaveAddress, 0xB5, &name[1], 1);
        __delay_ms(time * 10);
    }
}

void settings(void) {
    OSCCON = 0x72;
    ADCON1 = 0x0F;
    TRISBbits.RB0 = 1;
    TRISBbits.RB1 = 1;
    SSPSTAT = 0x80;
    SSPCON1 = 0x28;
    SSPCON2 = 0;
    SSPADD = 0x13;
}

void WriteI2C(unsigned char Address, unsigned char Register, unsigned char *Data, int bytes) {
    StartI2C();
    SendI2C(Address);
    SendI2C(Register);
    for (i = 0; i < bytes; i++) {
        SendI2C(*Data);
        Data++;
    }
    StopI2C();
    __delay_ms(time * 100);
}

void StartI2C(void) {
    ReadyI2C();
    SEN = 1;
}

void ReadyI2C(void) {
    while ((SSPCON2 & 0x1F) || (SSPSTAT & 0x04));
}

void SendI2C(unsigned char data) {
    ReadyI2C();
    SSPBUF = data;
}

void StopI2C(void) {
    ReadyI2C();
    PEN = 1;
}

void ReadI2C(unsigned char Address, unsigned char Register, unsigned char *Data, int bytes) {
    StartI2C();
    SendI2C(Address);
    SendI2C(Register);
    RepeatedStartI2C();
    SendI2C(Address | 0x01);
    for (i = 0; i < bytes; i++) {
        if (i + 1 == bytes) {
            *Data = ReceiveI2C(1);
        } else {
            *Data = ReceiveI2C(0);
        }
        Data++;
    }
    StopI2C();
    __delay_ms(time * 100);
}

void RepeatedStartI2C(void) {
    ReadyI2C();
    RSEN = 1;
}

unsigned char ReceiveI2C(char flag) {
    unsigned char buffer;
    ReadyI2C();
    RCEN = 1;
    ReadyI2C();
    buffer = SSPBUF;
    ReadyI2C();
    ACKDT = flag == 1 ? 1 : 0;
    ACKEN = 1;
    return buffer;
}
