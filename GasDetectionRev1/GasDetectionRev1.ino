// This is an example of implementation using ESP8266
// Never connect the sensor direct to the ESP8266, sensor high level is 5V
// ADC of ESP8266 high level is 3.3
// To connect use a voltage divisor, where 5V will 3v3 on the middle point like
// this {{URL}}

/*
  MQUnifiedsensor Library - reading an MQSensor using ESP8266 board

  For this example wi will demonstrates the use a MQ2 sensor.
  Library originally added 01 may 2019
  by Miguel A Califa, Yersson Carrillo, Ghiordy Contreras, Mario Rodriguez
 
  Added ESP8266 example 
  29.03.2020
  Wiring:
  https://github.com/miguel5612/MQSensorsLib_Docs/blob/master/static/img/MQ_ESP8266.PNG
*/

//Include the library
#include <MQUnifiedsensor.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
/************************Hardware Related Macros************************************/
#define         Board                   ("ESP8266")
#define         Pin                     (A0)
#define         buzzer                  (D3)
#define         fire                    (D5)
#define         SCREEN_WIDTH            (128)
#define         SCREEN_HEIGHT           (64)
#define         OLED_RESET              (-1)
#define         SCREEN_ADDRESS          0x3C
#define         DPIN                    (D6)
#define         DTYPE                   (DHT11)
/***********************Software Related Macros************************************/
#define         Type                    ("MQ-2") //MQ2
#define         Voltage_Resolution      (3.3) // 3V3 <- IMPORTANT
#define         ADC_Bit_Resolution      (10) // For ESP8266
#define         RatioMQ2CleanAir        (9.83)
/****************************Blynk Template ID**************************************/
#define BLYNK_TEMPLATE_ID "TMPL66Si-QhR"
#define BLYNK_TEMPLATE_NAME "Gas and Fire detector"
#define BLYNK_AUTH_TOKEN "1B4OVDeoihQ2PZPuyYEtQO5qDTer6k7N"
/********Wifi Connection************/
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "uwuntu"; // Change your Wifi/ Hotspot Name
char pass[] = "ravioliravioliwhatsinthepocketoli"; // Change your Wifi/ Hotspot Password
//char auth[] = BLYNK_AUTH_TOKEN;
//char ssid[] = "B.OKE"; // Change your Wifi/ Hotspot Name
//char pass[] = "0123456789";
/*****************************Globals***********************************************/
MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(DPIN, DTYPE);
float gas;
int temp;
int attempts = 0;
BlynkTimer timer;
/*****************************Globals***********************************************/

void setup() {
  //Init the serial port communication - to debug the library
  Serial.begin(9600); //Init serial port
  pinMode(buzzer, OUTPUT);
  pinMode(fire, INPUT);
  digitalWrite(buzzer, HIGH);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(50);
  Blynk.connectWiFi(ssid, pass);
  Blynk.config(auth);
  if(WiFi.status() != WL_CONNECTED)
  {
    timer.setInterval(10L, SensorValue);
  dhtSetup();
  delay(2000);
  MQ2Setup();
  }
  else
  {
  timer.setInterval(10L, SensorValue);
  Blynk.connect();
  dhtSetup();
  delay(2000);
  MQ2Setup();
  }
  
}

void loop() {
  MQ2.update(); // Update data, the arduino will be read the voltage on the analog pin
  MQ2.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  MQ2.serialDebug(); // Will print the table on the serial port
  Blynk.run();
  timer.run();
  delay(10); //Sampling frequency
}
void SensorValue(){
  gas = MQ2.readSensor(); // variable for gas volume
  temp = dht.readTemperature(); // variable for temperature
  int fireValue = digitalRead(fire); //
  digitalWrite(buzzer, HIGH); 
  DisplayValue();
  Blynk.virtualWrite(V2, gas);
  Blynk.virtualWrite(V0, 0);
  Blynk.virtualWrite(V3, temp);
  if (gas >=300 && gas <2000)
  {
    digitalWrite(buzzer, LOW);
    delay(300);
    digitalWrite(buzzer, HIGH);
    digitalWrite(buzzer, LOW);
    delay(500);
    digitalWrite(buzzer, HIGH);
  }
  if (gas >=2000 && gas <8000)
  {
    Blynk.logEvent("gas_detected");
    digitalWrite(buzzer, LOW);
    delay(250);
    digitalWrite(buzzer, HIGH);
    digitalWrite(buzzer, LOW);
    delay(400);
    digitalWrite(buzzer, HIGH);
  }
  if (gas >=8000)
  {
    Blynk.logEvent("gas_detected");
    digitalWrite(buzzer, LOW);
    delay(100);
    digitalWrite(buzzer, HIGH);
    digitalWrite(buzzer, LOW);
    delay(150);
    digitalWrite(buzzer, HIGH);
  }
if (fireValue == LOW || temp >= 40)
  {
    Blynk.logEvent("fire_detected");
    Blynk.virtualWrite(V0, 1);
    digitalWrite(buzzer, LOW);
    delay(500);
    digitalWrite(buzzer, HIGH);
  }
}
void DisplayValue(){
  display.clearDisplay();
  display.setTextSize(2); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print(F("PPM: "));
  display.println(gas);
  display.print(F("Temp: "));
  display.print(temp);
  display.println(" C");   
  display.display();          
}
void dhtSetup(){
 dht.begin();
}
void MQ2Setup(){
    //Set math model to calculate the PPM concentration and the value of constants
  MQ2.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ2.setA(574.25); MQ2.setB(-2.222); // Configurate the ecuation values to get LPG concentration
  /*
    Exponential regression:
    Gas    | a      | b
    H2     | 987.99 | -2.162
    LPG    | 574.25 | -2.222
    CO     | 36974  | -3.109
    Alcohol| 3616.1 | -2.675
    Propane| 658.71 | -2.168
  */

  /*****************************  MQ Init ********************************************/ 
  //Remarks: Configure the pin of arduino as input.
  /************************************************************************************/ 
  MQ2.init(); 
 
  /* 
    //If the RL value is different from 10K please assign your RL value with the following method:
    MQ2.setRL(10);
  */
  
  /*****************************  MQ CAlibration ********************************************/ 
  // Explanation: 
  // In this routine the sensor will measure the resistance of the sensor supposing before was pre-heated
  // and now is on clean air (Calibration conditions), and it will setup R0 value.
  // We recomend execute this routine only on setup or on the laboratory and save on the eeprom of your arduino
  // This routine not need to execute to every restart, you can load your R0 if you know the value
  // Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ2.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ2.calibrate(RatioMQ2CleanAir);
    Serial.print(".");
  }
  MQ2.setR0(calcR0/10);
//  MQ2.setR0(20);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply"); while(1);}
  /*  MQ CAlibration */ 
  MQ2.serialDebug(true);
}
