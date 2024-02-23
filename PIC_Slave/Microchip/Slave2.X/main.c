/*
 * File:   main.c
 * Author: LENOVO
 *
 * Created on 19 de Março de 2021, 20:31
 */

#include <xc.h>
#include <string.h>
#include <stdio.h>

#pragma config FOSC = INTOSC_HS
#pragma config WDT = OFF
#pragma config LVP = OFF

#define _XTAL_FREQ 8000000
#define time 10
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
void __interrupt() Slave_I2C(void);
//LCD
void SettingsLCD(unsigned char word);
void WriteLCD(unsigned char word);
void LCD(unsigned char data);
void Data1LCD(void);
void Data2LCD(void);

char buffer, texto[15], texto2[15];
unsigned char mibuffer, byte, text[6], i = 0, j;

void main(void) {
    settings();
    while (1) {
        Data1LCD();
        Data2LCD();
    }
}

void settings(void) {
    OSCCON = 0x72;
    ADCON1 = 0x0F;
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
    //I2C slave
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
            text[i] = mibuffer;
            i++;
            if (i == 6) {
                i = 0;
            }
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

void Data1LCD(void){
    SettingsLCD(RAW1);
    sprintf(texto,"%#X,%#x,%#X",text[0],text[1],text[2]);
    for(j=0; j<strlen(texto); j++){
        WriteLCD(texto[j]);
    }
}
void Data2LCD(void){
    SettingsLCD(RAW2);
    sprintf(texto2,"%#x,%#X,%#x",text[3],text[4],text[5]);
    for(j=0; j<strlen(texto); j++){
        WriteLCD(texto2[j]);
    }
}