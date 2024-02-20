#include <Wire.h>

#define Frequencia 50 // VALOR DA FREQUENCIA DO SERVO en Hz
#define I2C_ADDRESS_PCA9685 0x40
#define SERVOMIN 110 // VALOR PARA UM PULSO MAIOR QUE 1 mS
#define SERVOMAX 510 // VALOR PARA UM PULSO MENOR QUE 2 mS
#define PWM_Frequencia 1000
//Registros
#define PCA9685_MODE1 0x00 /**< Mode Register 1 */
#define PCA9685_PRESCALE 0xFE /**< Prescaler for PWM output frequency */
//Datos
#define MODE1_RESTART 0x80 /**< Restart enabled */
#define FREQUENCY_CALIBRATED 26075000 /**< Oscillator frequency measured at 104.3% */
#define MODE1_RESTART 0x80 /**< Restart enabled */
#define MODE1_AI 0x20 /**< Auto-Increment enabled */
#define PCA9685_PRESCALE_MIN 3 /**< minimum prescale value */
#define PCA9685_PRESCALE_MAX 255 /**< maximum prescale value */
#define MODE1_SLEEP 0x10 /**< Low power mode. Oscillator off */

#define PCA9685_LED0_ON_L 0x06 /**< LED0 output and brightness control byte 0 */

uint8_t data[1];
int pos, DriverPin = 0;

void void_write(uint8_t Address, uint8_t Register, uint8_t Data);
void void_write2(uint8_t Address, uint8_t Register, uint8_t num, uint16_t on, uint16_t off);
void void_read(uint8_t Address, uint8_t Register, uint8_t Nbytes, uint8_t* Data);
void setPWMFreq(float freq);

void setup() {
  //.....................................................................
  //                        Inicializacao I2C
  //.....................................................................
  Wire.begin(); //Inicializando comunicacao I2C   
  Serial.begin(115200); // Abrindo o canal de comunicação serial a 115200 baudios/segundo
  delay(100);
  Serial.println("INICIALIZANDO LA COMUNICACIÓN I2C");
  Serial.println();
  //begin
  void_write(I2C_ADDRESS_PCA9685, PCA9685_MODE1, MODE1_RESTART);
  delay(10);
  //// set a default frequency
  setPWMFreq(PWM_Frequencia);
  //// set a servo frequency
  setPWMFreq(Frequencia);
  delay(300);
}

void loop() {
  // put your main code here, to run repeatedly:
  //writeServos
  pos = map (180 , 0 , 180 , SERVOMIN, SERVOMAX);
  void_write2(I2C_ADDRESS_PCA9685, PCA9685_LED0_ON_L, DriverPin, 0, pos);
  delay(1000);
  pos = map (0 , 0 , 180 , SERVOMIN, SERVOMAX);
  void_write2(I2C_ADDRESS_PCA9685, PCA9685_LED0_ON_L, DriverPin, 0, pos);
  delay(1000);
  pos = map (45 , 0 , 180 , SERVOMIN, SERVOMAX);
  void_write2(I2C_ADDRESS_PCA9685, PCA9685_LED0_ON_L, DriverPin, 0, pos);
  delay(1000);
}

void setPWMFreq(float freq){
  // Range output modulation frequency is dependant on oscillator
  if (freq < 1) freq = 1;
  if (freq > 3500) freq = 3500; // Datasheet limit is 3052=50MHz/(4*4096)
  /*
  freq *= 0.9; // Correct for overshoot in the frequency setting (see issue #11)
  float prescaleval = FREQUENCY_OSCILLATOR;
  */
  uint32_t prescaleval = FREQUENCY_CALIBRATED;
  prescaleval /= freq; // required output modulation frequency
  // rounding to nearest number is equal to adding 0,5 and floor to nearest number
  prescaleval += 2048;
  prescaleval /= 4096;
  prescaleval -= 1;
  if (prescaleval < PCA9685_PRESCALE_MIN) prescaleval = PCA9685_PRESCALE_MIN;
  if (prescaleval > PCA9685_PRESCALE_MAX) prescaleval = PCA9685_PRESCALE_MAX;
  uint8_t prescale = (uint8_t) prescaleval;
  void_read(I2C_ADDRESS_PCA9685, PCA9685_MODE1, 1, data);
  uint8_t newmode = (data[0] & ~MODE1_RESTART) | MODE1_SLEEP; // sleep
  void_write(I2C_ADDRESS_PCA9685, PCA9685_MODE1, newmode);            // go to sleep
  void_write(I2C_ADDRESS_PCA9685, PCA9685_PRESCALE, prescale);        // set the prescaler
  void_write(I2C_ADDRESS_PCA9685, PCA9685_MODE1, data[0]);
  delay(5);
  // This sets the MODE1 register to turn on auto increment.
  void_write(I2C_ADDRESS_PCA9685, PCA9685_MODE1, data[0] | MODE1_RESTART | MODE1_AI);
}
 

//----------------------------------------------------------------------------
//  FUNCAO ESCRITA DADOS [Endereco(8bits) | Regitrador(8bits) | Dado(8bits)]
//----------------------------------------------------------------------------
//Address = 0x68 (Endereco da MPU9250 "acel e giro") ou 0x0C (Endereco do Magnetômetro)
//Register = 0x1C ou 0x1B (Endereco do registrador a acessar na MPU9250 "acel e giro") ou 0x (Endereco do Magnetômetro)
//Data = Nome_Data (Dado que vai ser configurado)

void void_write(uint8_t Address, uint8_t Register, uint8_t Data){
  Wire.beginTransmission(Address); //Inicia comunicação com o endereço I2C do sensor   
  Wire.write(Register); //Define o endereço inicial
  Wire.write(Data); //Escreve o dado a configurar
  Wire.endTransmission(); //Deixa aberta a comunicação com o endereço I2C do sensor
}

void void_write2(uint8_t Address, uint8_t Register, uint8_t num, uint16_t on, uint16_t off){
  Wire.beginTransmission(Address); //Inicia comunicação com o endereço I2C do sensor   
  Wire.write(Register  + 4 * num); //Define o endereço inicial
  Wire.write(on); //Escreve o dado a configurar
  Wire.write(on >> 8); //Escreve o dado a configurar
  Wire.write(off); //Escreve o dado a configurar
  Wire.write(off >> 8); //Escreve o dado a configurar
  Wire.endTransmission(); //Deixa aberta a comunicação com o endereço I2C do sensor
}
//----------------------------------------------------------------------------
//  FUNCAO LEITURA DADOS [Endereco(8bits) | Regitrador(8bits) | N°Bytes(#) | Dado(8bits)]
//----------------------------------------------------------------------------
//Address = 0x68 (Endereco da MPU9250 "acel e giro") ou 0x0C (Endereco do Magnetômetro)
//Register = 0x3B (Endereco do registrador a acessar na MPU9250 "acel e giro") ou 0x03 (Endereco do Magnetômetro)
//Nbytes = 6 para acelerômetro + 2 termometro + 6 girôscopio ou 6 magnetometro (Numero de bytes a transportar)
//Data = Buffer (Nome da variavel onde vai ser salvados os bytes transportados)

void void_read(uint8_t Address, uint8_t Register, uint8_t Nbytes, uint8_t *Data){
  Wire.beginTransmission(Address); //Inicia comunicação com o endereço I2C do sensor   
  Wire.write(Register); //Define o endereço inicial
  Wire.endTransmission(); //Deixa aberta a comunicação com o endereço I2C do sensor

  // Read Nbytes
  Wire.requestFrom(Address, Nbytes); //Solicita os dados de 14 endereços de 8 bits do sensor (6 do Acelerômetro, 2 do Termometro e 6 do Girôscopio))
  uint8_t index = 0; // Índice do vetor, o qual chegará até Nbytes
  while (Wire.available()) //Enquanto fique disponível a conexao com os sensores
    Data[index++] = Wire.read(); //Vetor onde vai armazenar os dados dos sensores
}
