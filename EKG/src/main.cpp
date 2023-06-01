#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "SSD1306Wire.h" 
#include "Ringbuffer.h"
#include "Graph.h"


//#define I2Cspeed 1000000
#define analogIn 33 
#define adcFreq 250
#define dataCaptureTime 30




//function prototypes
void IRAM_ATTR onADCTimer();
void onBuffer();
void initdisplay();
void initTimer();
void initWiFi();
void initUDP();
void startUpScreen();
void sendPacket();
void calcRuntime();

//global settings
//static variables
static int timerPreScaler = 80; //80 for 1 MHz increments at a cpu speed of 80 MHz
static int adcTimeStep = (CPU_CLK_FREQ/timerPreScaler) / adcFreq; //calculate timerFrequency in µs based on cpu speed, prescaler and user set frequency in Hz
static int bufferSize = (dataCaptureTime * 1000 * 1000)/adcTimeStep; //calculate buffersize to contain values for the set time range
//Interval in wich packets of the buffer are sending
static uint16_t packageAmount = 2;
static uint16_t packetInterval = bufferSize/packageAmount;

//variables 
bool newData = false;
float estGraphTime;

//objects
hw_timer_t *adcTimer = NULL;
SSD1306Wire display(0x3c, SDA, SCL);
//SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_128_64, I2C_ONE, 1000000);
Ringbuffer buffer(bufferSize, &onBuffer, packetInterval);
GraphSettings settings;
Graph graph(&settings, &display);

//Wifi credentials
const char * ssid = "";
const char * pass = ""; 
const int localPort = 4069;

//Udp stuff
WiFiUDP udp;

void setup() {

  Serial.begin(9600);
  Serial.print("ADC running at "); Serial.print(adcTimeStep); Serial.println(" µs.");
  Serial.print("Buffer size "); Serial.print(bufferSize); Serial.print(" capturing for "); Serial.print(dataCaptureTime); Serial.println(" s.");
  
  //init display
  initdisplay();


  //----- graph settings -----
  settings.title = "EKG";
  settings.x = 10;
  settings.y = 30;
  settings.length = 110;
  settings.height = 30;
  //optional graph settings
  settings.movingAverage = 8;
  settings.drawAxis = true;
  settings.drawArrowheads = true;
  settings.linemode = true;


  estGraphTime = ((settings.movingAverage * settings.length) * (adcTimeStep/1000000.0)); //avg = datapoints per pixel  times graph length times adcStepTime in seconds for estimate graph time
  Serial.print("Estimated graph time: "); Serial.print(estGraphTime); Serial.println(" s");
  Serial.print("Approximately "); Serial.print((estGraphTime/dataCaptureTime)*100); Serial.println("% of the Total capture Time.");


  //startUpScreen();
  graph.drawGraphMeta();

  
  //init WiFi
  initWiFi();

  //UDP init
  initUDP();

  //Timer setup
  initTimer();

}

uint16_t dummyData = 0;

void loop() {
  if (newData){
    if (dummyData >= 7500) dummyData = 0;
    else dummyData++;
    //buffer.addData(dummyData);
    buffer.addData(analogRead(analogIn));
    graph.updateGraph(buffer.getLastVal());
    newData = false;
  }


  calcRuntime();
}

//----------------------------- Custom Functions -----------------------------

// ISR

//timer isr
void IRAM_ATTR onADCTimer(){ 
  //buffer.addData(analogRead(analogIn));
  newData = true;
}

//buffer isr
void onBuffer(){
  Serial.println("sending");
  sendPacket();
}

//custom functions

void initdisplay(){
  display.init();
  display.flipScreenVertically();
  display.setFont(DejaVu_Sans_Mono_10);
  display.clear();

  display.setColor(WHITE);
}

//setup functions
void initWiFi(){
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    //Wait for Wifi
    Serial.print("Waiting for wifi ");
    while (WiFi.status() != WL_CONNECTED){
      delay(500);
    }
    Serial.println("\nConnected!");
    Serial.print("ESP32-IP: "); Serial.println(WiFi.localIP());
}

void initTimer(){
  //ADC-Timer
  adcTimer = timerBegin(0, timerPreScaler, true);
  timerAttachInterrupt(adcTimer, &onADCTimer, true);
  timerAlarmWrite(adcTimer, adcTimeStep, true);
  timerAlarmEnable(adcTimer);
}

void initUDP(){
    uint8_t buffer[50];
    Serial.print("Waiting for udp!");
    udp.begin(WiFi.localIP(), localPort);
    //wait for matlab handshake
    bool connected = false;
    while (!connected){
        udp.parsePacket();
        //await response 
        int remBytes = udp.available();
        if(udp.read(buffer, remBytes) > 0){
            Serial.print("Message recieved: ");
            Serial.println((char *)buffer);
            connected = true;
        }
    }
    Serial.println("Connection found proceeding");
    delay(500);

    //send buffersize to matlab
    /*
    udp.beginPacket(udp.remoteIP(),udp.remotePort());
    udp.write((uint8_t*)&packetInterval,2);
    udp.write((uint8_t*)&packageAmount, 2);
    udp.endPacket();
    */

    Serial.print("Connected to Matlab ");
    Serial.print(udp.remoteIP());
    Serial.print(":"); 
    Serial.println(udp.remotePort());
}

void sendPacket(){
  uint16_t *packetstart = buffer.getCurrPointer();
  //packetstart--; //reduce pointer to compensate for 0
  packetstart = packetstart - packetInterval; //move pointer to beginning of packet
  
  
  
  udp.beginPacket(udp.remoteIP(), udp.remotePort());
  udp.write((uint8_t*)packetstart, bufferSize);
  udp.endPacket();
  

}

void startUpScreen(){
  //Welcome screen
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(DejaVu_Sans_Mono_16);
  display.drawString(64, 5, "EKG-Labor");
  display.setFont(DejaVu_Sans_Mono_10);
  display.drawXbm(34, 25, 60, 30, logo);
  display.display();
  delay(3000);
  display.clear();

  //info screen
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  String info = "Adc freq: " + (String)adcFreq + " Hz";
  //info += "\nadc time: " + (String)adcTimeStep + " s";
  info += "\nbuffer size: " + (String)bufferSize;
  info += "\ngraph time: " + (String) estGraphTime + " s";
  display.drawString(5, 5, info);
  display.display();
  delay(6000);
  display.clear();

  //network screen
  info = "192.168.152.53";
  info += "\n4069";
  display.drawString(5, 5, info);
  display.display();
  delay(3000);
  display.clear();

}

void calcRuntime(){
  static long int loopCount = 0;
  static long int lastStamp = micros();
  static int loopTime = micros()-lastStamp;

  loopTime = micros()-lastStamp;
  if (loopTime >= 1000*1000){
    loopTime = (loopTime)/loopCount;
    lastStamp = micros();
    loopCount = 0;
    
    String t = String(loopTime);
    display.setColor(WHITE);
    display.setFont(DejaVu_Sans_Mono_10);
    display.drawString(5, 5, t);
    display.drawString(25, 5, "µs");
    display.display();
  }
  loopCount++;
}