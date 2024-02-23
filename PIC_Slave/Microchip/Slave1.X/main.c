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

void settings(void);
void __interrupt() Slave_I2C(void);

char buffer;
unsigned char mibuffer, byte;

void main(void) {
    settings();
    while (1) {
        byte = mibuffer;
        if (byte == 0x01) {
            LATD = 0x80;
        }else if(byte == 0x00){
            LATD = 0x00;
        }
    }
}

void settings(void) {
    OSCCON = 0x72;
    ADCON1 = 0x0F;
    TRISD = 0x00;
    LATD = 0x00;
    TRISBbits.RB0 = 1;
    TRISBbits.RB1 = 1;
    SSPSTAT = 0x80;
    SSPADD = 0x30; //Address slave
    SSPCON1 = 0x36;
    SSPCON2 = 0x01;
    GIE = 1;
    PEIE = 1;
    SSPIE = 1;
    SSPIF = 0;
}

void __interrupt() Slave_I2C(void) {
    if (SSPIF == 1) {
        SSPCON1bits.CKP = 0; //apaga señales de reloj 
        if (SSPCON1bits.SSPOV || SSPCON1bits.WCOL) { //LLega la informacion incorrecta, colision de datos o sobreflujo 
            buffer = SSPBUF;
            SSPCON1bits.SSPOV = 0; // limpia
            SSPCON1bits.WCOL = 0; // limpia
            SSPCON1bits.CKP = 1; // inicia nuevamente reloj 
        }
        if (!SSPSTATbits.D_nA || !SSPSTATbits.R_nW) {
            /*DA si esta en 0 es direccion y si esta en 1 es data, igual con RW 0 para escribir y 1 para leer
             * maestro desea escribir en el esclavo   
             * El esclavo va a funcionar como receptor solamente.     
             */
            buffer = SSPBUF;
            while (BF); //Pasa a 0 automaticamente cuando SSPBUF es leido
            mibuffer = SSPBUF; //El pulso de ack se genera automaticamente y el bf cambia a 1 
            SSPCON1bits.CKP = 1;
            SSPM3 = 0; // para modo slave 7 bits sin interrupcion de star y stop
        } else if (!SSPSTATbits.D_nA || SSPSTATbits.R_nW) {
            /* El maestro desea leer del esclavo, 
             * el esclavo va a funcionar como transmisor.
             */
            buffer = SSPBUF;
            BF = 0; //Opcional
            SSPBUF = PORTC;
            SSPCON1bits.CKP = 1;
            while (SSPSTATbits.BF);
        }
        SSPIF = 0;
    }
}