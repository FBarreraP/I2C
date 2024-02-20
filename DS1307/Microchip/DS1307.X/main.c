/*
 * File:   main.c
 * Author: LENOVO
 *
 * Created on 19 de Julho de 2020, 14:14
 */

#include <xc.h>
#include <stdio.h>
#include <string.h>

#pragma config FOSC = INTOSC_HS
#pragma config WDT = OFF
#pragma config LVP = OFF

#define _XTAL_FREQ 8000000
#define time 1000
#define time2 100
#define SlaveAddress 0xD0

//LCD
#define CD 0x01 //Clear Display
#define RH 0x02 //(0x03) Return Home
#define EMS 0x06 //Entry Mode Set
#define DC 0x0F //(0x0E) Display Control
#define DSr 0x1C //Display Shift Rigth
#define DSl 0x18 //Display Shift Left
#define FS 0x38 //(0x3C) Function Set
#define RAW1 0x80 //DDRAM display
#define RAW2 0xC0 //DDRAM display
#define button PORTBbits.RB2 //Button start
#define RS LATEbits.LATE1 //Register Selection
#define E LATEbits.LATE0 //Enable

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

unsigned char hora[3]={0x50,0x59,0x71};//segundos,minutos,hora
unsigned char calendario[4]={0x01,0x20,0x02,0x17};//nombre del día, día, mes y año
unsigned char DatosHora[3],DatosCalendario[4],name[6] = {'f','A','b','I','a','N'};
int i,z,k;
char buffer[20],Clock_type = 0x06, AM_PM = 0x05;
char smh[20],dma[20];
char text1[5]={"PM"}, text2[5]={"AM"}, days[7] = {'S','M','T','W','t','F','s'}, text3[5]={"Sun"},text4[5]={"Mon"},text5[5]={"Tue"},text6[5]={"Wed"},text7[5]={"Thu"},text8[5]={"Fri"},text9[5]={"Sat"};
    
void main(void) {
    settings();
    WriteI2C(SlaveAddress,0x00,hora,3);//Configuración de la hora
    WriteI2C(SlaveAddress,0x03,calendario,4);//Configuración del calendario
    while(1){
        ReadI2C(SlaveAddress,0x00,DatosHora,3);
        if(DatosHora[2] & (1<<Clock_type)){     /* check clock is 12hr or 24hr */
            SettingsLCD(RAW1+14); 
            if(DatosHora[2] & (1<<AM_PM)){      /* check AM or PM */
                for(i=0; i<=strlen(text1); i++){
                    WriteLCD(text1[i]);
                }
            }
            else{
                for(i=0; i<=strlen(text2); i++){
                    WriteLCD(text2[i]);
                }
            }
            SettingsLCD(RAW1); 
            DatosHora[2] = DatosHora[2] & (0x1f);            
            sprintf(smh,"Hor:%x:%x:%x",DatosHora[2],DatosHora[1],DatosHora[0]); /*%x for reading BCD format from RTC DS1307*/
            for(i=0; i<=strlen(smh); i++){
                WriteLCD(smh[i]);
            }
        }
        else{
            SettingsLCD(RAW1); 
            DatosHora[2] = DatosHora[2] & (0x3f);
            sprintf(smh,"Hor:%x:%x:%x",DatosHora[2],DatosHora[1],DatosHora[0]); /*%x for reading BCD format from RTC DS1307*/
            for(i=0; i<=strlen(smh); i++){
                WriteLCD(smh[i]);
            }
        }
        ReadI2C(SlaveAddress,0x03,DatosCalendario,4);
        SettingsLCD(RAW2); 
        sprintf(dma,"Cal:%x/%x/%x",DatosCalendario[1],DatosCalendario[2],DatosCalendario[3]); /*%x for reading BCD format from RTC DS1307*/
        for(i=0; i<=strlen(dma); i++){
            WriteLCD(dma[i]);
        }

    /* find day */
        switch(days[DatosCalendario[0]]){
            SettingsLCD(RAW2+13); 
            case 'S':
                for(i=0; i<=strlen(text3); i++){
                    WriteLCD(text3[i]);
                }
                break;
            case 'M':
                for(i=0; i<=strlen(text4); i++){
                    WriteLCD(text4[i]);
                }
                break;                
            case 'T':
                for(i=0; i<=strlen(text5); i++){
                    WriteLCD(text5[i]);
                }
                break;                
            case 'W':   
                for(i=0; i<=strlen(text6); i++){
                    WriteLCD(text6[i]);
                }
                break;                
            case 't':
                for(i=0; i<=strlen(text7); i++){
                    WriteLCD(text7[i]);
                }
                break;
            case 'F':
                for(i=0; i<=strlen(text8); i++){
                    WriteLCD(text8[i]);
                }
                break;                
            case 's':
                for(i=0; i<=strlen(text9); i++){
                    WriteLCD(text9[i]);
                }
                break;
            default: 
                break;           
        }
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
    SSPADD = 0x31;    
    //LCD
    TRISD = 0x00;
    LATD = 0x00;
    TRISE = 0x00;
    LATE = 0x00;
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
    __delay_ms(time);
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
        //WriteLCD(*Data);
        Data++;
    }
    StopI2C();
    __delay_ms(time);
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
    LCD(word);
}

void WriteLCD(unsigned char word){
    RS = 1;
    LCD(word);
}

void LCD(unsigned char data){
    E = 1;
    __delay_us(time2);
    LATD = data;
    __delay_us(time2);
    E = 0;
    __delay_us(time2);
}