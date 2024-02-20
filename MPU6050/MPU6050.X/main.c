/*
 * File:   main.c
 * Author: USER
 * Project: MPU6050, PIC18F4550 e LCD
 * Created on 8 de Julho de 2019, 17:03
 */

#include <xc.h>
#include <stdio.h>
#include <string.h>

#pragma config FOSC = HS
#pragma config WDT = OFF
#pragma config LVP = OFF

#define _XTAL_FREQ 20000000
#define time 10
//I2C
#define Slave_Address 0xA0//0x68 //Endereco da MPU6050
#define GYRO_FULL_SCALE_250_DPS 0x00 // SCALE_250 (°/s) = 0 (0x00 = 000|00|000)
#define ACC_FULL_SCALE_2_G 0x00 // SCALE_2_G (g) = 0 (0x00 = 000|00|000)
#define SENSITIVITY_ACCEL 2.0/32768.0             // Valor de conversão do Acelerômetro (g/LSB) para 2g e 16 bits de comprimento da palavra
#define SENSITIVITY_GYRO 250.0/32768.0           // Valor de conversão do Girôscopio ((°/s)/LSB) para 250 °/s e 16 bits de comprimento da palavra
#define SENSITIVITY_TEMP 333.87                  // Valor de sensitividade do Termometro (Datasheet: MPU-9250 Product Specification, pag. 12)
//LCD
#define RS LATEbits.LATE2 //Register Selection
#define E LATEbits.LATE1 //Enable
#define CD 0x01 //Clear Display
#define RH 0x02 //(0x03) Return Home
#define EMS 0x06 //Entry Mode Set
#define DC 0x0F //(0x0E) Display Control
#define DSr 0x1C //Display Shift Rigth
#define DSl 0x18 //Display Shift Left
#define FS 0x38 //(0x3C) Function Set
#define DDRAM_RAW1 0x80 //DDRAM display
#define DDRAM_RAW2 0xC0 //DDRAM display

void settings(void);
void start(void);
//I2C
void WriteI2C(unsigned char Address, unsigned char Register, unsigned char *Data, int Ndatos);
void ReadI2C(unsigned char Address, unsigned char Register, int Nbytes, unsigned char *Data);
void StartI2C (void);
void RepeatedStartI2C (void);
void WriteAddressI2C (unsigned char address);
void SendI2C (unsigned char data);
void ReadyI2C (void);
void StopI2C (void);
unsigned char ReceiveI2C (char flag);
//LCD
void clearLCD(void);
void LCD(char data);
void WriteLCD(unsigned char *a, unsigned char DDRAM_RAW, int Size);
//MPU6050
void TestConnection(void);
void settingsIMU(void);

int i,raw_accelx;
unsigned char a=0x00,byte[3],word[6]={'f','a','b','i','a','n'},text[20],text1[]={"Teste de conexao:"},text2[]={"Erro com a MPU6050"},text3[]={"Conexao sucedida"},text4[]={"Configuracao"},data,GyrAccel[14];

void main(void) {
    settings();
    WriteI2C(Slave_Address,0x6B,&a,1);
    TestConnection();
    settingsIMU();
    while(1){
        start();
    }
}

void settings(void){
    //I2C
    TRISBbits.RB0 = 1;
    TRISBbits.RB1 = 1;
    SSPADD = 0x31; 
    SSPSTAT = 0x80; //Velocidad padrao no bus I2C
    SSPCON1 = 0x28; //Modo Mestre habilitado no I2C
    SSPCON2 = 0x00; //Comunicacao I2C nao inicializada
    //LCD
    TRISD = 0x00;
    LATD = 0x00;
    TRISE = 0x01;
    LATE = 0x00;
    LCD(EMS);
    LCD(DC);
    LCD(FS);
    //
    TRISBbits.RB6 = 0;
    TRISBbits.RB7 = 0;
    LATB = 0;
}

void TestConnection(void){
    clearLCD();
    WriteLCD(text1,DDRAM_RAW1,sizeof(text1));
    ReadI2C(Slave_Address,0x75,1,&data);
    if (data != 0x68){
        WriteLCD(text2,DDRAM_RAW2,sizeof(text2));
        sprintf(text,"Eu sou: %x",data);
        WriteLCD(text,DDRAM_RAW2,sizeof(text));
        LATBbits.LATB6 = 1;
//        while(1);
    }else{
        WriteLCD(text3,DDRAM_RAW2,sizeof(text3));
        LATBbits.LATB7 = 1;
    }        
    __delay_ms(time*5);
}

void settingsIMU(void){
    clearLCD();
    WriteLCD(text4,DDRAM_RAW1,sizeof(text4));
    WriteI2C(Slave_Address,0x1B,&a,1);
    WriteI2C(Slave_Address,0x1C,&a,1);
}

void start(void){
    ReadI2C(Slave_Address,0x3B,14,GyrAccel);
    raw_accelx = (GyrAccel[0]<<8|GyrAccel[1]);
    sprintf(text,"ax, ay, az:%d",-32767);
    WriteLCD(text,DDRAM_RAW2,sizeof(text));
//    WriteI2C(Slave_Address,0x01,&word[5],1);
//    WriteI2C(Slave_Address,0x05,word,6);
//    ReadI2C(Slave_Address,0x05,3,byte);
}

void WriteI2C(unsigned char Address, unsigned char Register, unsigned char *Data, int Ndatos){
//    Address = Address<<1;
    StartI2C();
    SendI2C(Address);
    SendI2C(Register);
    for(i=0; i<Ndatos; i++){
        SendI2C(*Data);
        Data++;
    }
    StopI2C();
    __delay_ms(1000);
 
}

void ReadI2C(unsigned char Address, unsigned char Register, int Nbytes, unsigned char *Data){
    StartI2C();
    SendI2C(Address);
    SendI2C(Register);
    RepeatedStartI2C();
    SendI2C(Address|0x01);
    for(i=0; i<Nbytes; i++){
        if (i+1 == Nbytes){
            *Data = ReceiveI2C(1);
        }else{
            *Data = ReceiveI2C(0);
        }
        Data++;
    }
    StopI2C();
    __delay_ms(1000);
}

void StartI2C (void){
    ReadyI2C();
    SSPCON2bits.SEN = 1; //Envia o pulso do 'start'
}

void RepeatedStartI2C (void){
    ReadyI2C();
    SSPCON2bits.RSEN = 1; //Envia o pulso do 'start'
}

void SendI2C (unsigned char data){
    ReadyI2C();
    SSPBUF = data;		// Send Byte value
//	return ACKSTAT;		// Return ACK/NACK from slave
}

void ReadyI2C (void){
    while((SSPCON2 & 0x1F) || (SSPSTAT & 0x04));
//    while(PIR1bits.SSPIF == 0);	/* Wait for operation complete */
//    PIR1bits.SSPIF = 0;		/* Clear SSPIF interrupt flag*/
}

void StopI2C (void){
    ReadyI2C();
    SSPCON2bits.PEN = 1; //Envia o pulso do 'stop'
	
}

unsigned char ReceiveI2C (char flag){
    unsigned char buffer;
    ReadyI2C();
    SSPCON2bits.RCEN = 1; //Habilita o modo receptor
    ReadyI2C();
    buffer = SSPBUF;
    ReadyI2C();
    SSPCON2bits.ACKDT = (flag==1)? 1 : 0; 
    SSPCON2bits.ACKEN = 1;
    return buffer;
}

void clearLCD(void){
    RS = 0;
    LCD(CD);    
}

void LCD(char data){
    E = 1;
    __delay_ms(time);
    LATD = data;
    __delay_ms(time);
    E = 0;
    __delay_ms(time);
}

void WriteLCD(unsigned char *a, unsigned char DDRAM_RAW, int Size){
    RS = 0;
    LCD(DDRAM_RAW);
    RS = 1;
    for(i=0; i<=Size; i++){
        LCD(*a);
        a++;
        __delay_ms(time);
    }
}