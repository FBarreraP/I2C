/*
 * File:   main.c
 * Author: MICHAEL
 * 
 * Created on 9 de marzo de 2021, 10:57 AM
 */

#include <xc.h>

#pragma config FOSC = INTOSC_HS
#pragma config LVP = OFF
#pragma config WDT = OFF 

#define _XTAL_FREQ 400000
#define time 100

#define Direccion_pantalla 0x78

int i = 0, j = 0;

void settings(void);
void ReadyI2C(void);
void SendI2C(unsigned char data);
void StartI2C(void);
void StopI2C(void);
void RepeatedStart(void);

void iniciar_pantalla(void);


unsigned char ReadI2C();
char contador = 0, dir = 0x30;
int data;

void main(void) {
    settings();
    while (1) {
        for (j = 0; j < 128; j++) {
            StartI2C();
            SendI2C(Direccion_pantalla);
            SendI2C(0x40);
            SendI2C(0x81);
            StopI2C();
            __delay_ms(time * 5);
        }
    }
}

void settings(void) {
    OSCCON = 0x72;
    ADCON1 = 0x0F;
    //I2C
    TRISBbits.TRISB0 = 1;
    TRISBbits.TRISB1 = 1;
    SSPSTAT = 0x80;
    SSPCON1 = 0x28;
    SSPCON2 = 0x00;
    SSPADD = 0x13;
    iniciar_pantalla();
}

void StartI2C(void) {
    ReadyI2C();
    SEN = 1;
}

void SendI2C(unsigned char data) {
    ReadyI2C();
    SSPBUF = data;
}

void ReadyI2C(void) {
    while ((SSPCON2 & 0x1F) || (SSPSTAT & 0x04));
}

void StopI2C(void) {
    ReadyI2C();
    PEN = 1;
}

void RepeatedStart(void) {
    ReadyI2C();
    RSEN = 1;
}

unsigned char ReadI2C() {
    unsigned char buffer;
    ReadyI2C();
    RCEN = 1;
    ReadyI2C();
    buffer = SSPBUF;
    ReadyI2C();
    ACKDT = 1;
    ACKEN = 1;
    return buffer;
}

void iniciar_pantalla(void) {
    StartI2C();
    SendI2C(Direccion_pantalla);
    SendI2C(0x00); //Lista de comandos configuracion
    SendI2C(0xAE); //Apaga la pantalla
    //Establecer filas
    SendI2C(0xA8);
    SendI2C(0x3F);
    //offset en 0
    SendI2C(0xD3);
    SendI2C(0x00);
    //inicio de linea en 0
    SendI2C(0x40);
    //invertir el eje X de la pantalla cambiar por A0 si se desea cambiar la orientacion
    SendI2C(0xA1);
    //invertir eje Y cambiar por 0xC0 si se desea cambiar
    SendI2C(0xC8);
    //Mapeo de pines com
    SendI2C(0xDA);
    SendI2C(0x02); //probar en caso de no funcionar con 0x02
    //configurar el contraste
    SendI2C(0x81);
    SendI2C(0xFF); // este valor debe estar entre 0x00 y oxFF maximo
    //comando ordena el chip activa el modo output de la pantalla
    SendI2C(0xA4);
    //poner pantalla en modo normal
    SendI2C(0xA6);
    //establece la velocidad del oscilador 
    SendI2C(0xD5);
    SendI2C(0x80);
    //Activa 'charge pump'
    SendI2C(0x8D);
    SendI2C(0x14);
    //encender la pantalla
    SendI2C(0xAF);
    //rango de columnas y paginas
    SendI2C(0x21);
    SendI2C(0x00);
    SendI2C(0x7F);
    SendI2C(0x22);
    SendI2C(0x00);
    SendI2C(0x07);
    StopI2C();

//    for (i = 0; i < 1024; i++) {
//        StartI2C();
//        SendI2C(Direccion_pantalla);
//        SendI2C(0x40);
//        SendI2C(0x00);
//        StopI2C();
//        __delay_ms(time * 5);
//    }
}