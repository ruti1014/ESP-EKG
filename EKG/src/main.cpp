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

//--------------------------- function prototypes ---------------------------
//hard and soft interrupt
void IRAM_ATTR onADCTimer();
void onBuffer();

//init functions
void initdisplay();
void initWiFi();
void initTimer();
void initUDP();
void startUpScreen();

//Utility functions
void sendPacket();
uint16_t readData(bool dummy);
void matlabPing();
void calcRuntime();
void loadingCircle();

//--------------------------- global settings ---------------------------
//static variables
const int timerPreScaler = 80; //80 for 1 MHz increments at a cpu speed of 80 MHz
const int adcTimeStep = (CPU_CLK_FREQ/timerPreScaler) / adcFreq; //calculate timerFrequency in µs based on cpu speed, prescaler and user set frequency in Hz
const int bufferSize = (dataCaptureTime * 1000 * 1000)/adcTimeStep; //calculate buffersize to contain values for the set time range
//Interval in which packets of the buffer are sending
const uint16_t packageAmount = 2;
const uint16_t packetInterval = bufferSize/packageAmount;

//variables 
bool newData = false;
bool connected = false;
float estGraphTime;

//--------------------------- objects ---------------------------
hw_timer_t *adcTimer = NULL;
SSD1306Wire display(0x3c, SDA, SCL);
//SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_128_64, I2C_ONE, 1000000);
Ringbuffer buffer(bufferSize, &onBuffer, packetInterval);
GraphSettings settings;
Graph graph(&settings, &display);

//--------------------------- Wifi credentials ---------------------------
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

  //avg = datapoints per pixel  times graph length times adcStepTime in seconds for estimate graph time
  estGraphTime = ((settings.movingAverage * settings.length) * (adcTimeStep/1000000.0)); 
  Serial.print("Estimated graph time: "); Serial.print(estGraphTime); Serial.println(" s");
  Serial.print("Approximately "); Serial.print((estGraphTime/dataCaptureTime)*100); Serial.println("% of the Total capture Time.");


  //init WiFi
  initWiFi();

  //Startup information
  startUpScreen();

  //UDP init
  initUDP();

  //Timer setup
  initTimer();

  //setup fixed graph
  graph.drawGraphMeta();

}

bool stop = false;

void loop() {
 //matlabPing();

  //add new data (sensor or dummy)
  if (newData){
    buffer.addData(readData(true));
    graph.updateGraph(buffer.getLastVal());
    //graph.drawCompleteFrame(&buffer);
    newData = false;
  }


  calcRuntime();
}

//----------------------------- Custom Functions -----------------------------

//----- ISR -----
void IRAM_ATTR onADCTimer(){ 
  //buffer.addData(analogRead(analogIn));
  newData = true;
}

void onBuffer(){
  //sendPacket();
}

//----- setup functions -----
void initdisplay(){
  display.init();
  display.flipScreenVertically();
  display.setFont(DejaVu_Sans_Mono_10);
  display.clear();

  display.setColor(WHITE);
}

void initWiFi(){
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    //Wait for Wifi
    Serial.print("Waiting for wifi ");
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 10, "Waiting on WiFi.");
    while (WiFi.status() != WL_CONNECTED){
      loadingCircle();
      delay(5);
    }
    display.clear();
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
    Serial.println("Waiting for udp!");
    udp.begin(WiFi.localIP(), localPort);



    //wait for matlab connection
    while (!connected){
        loadingCircle();
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

    Serial.print("Connected to Matlab ");
    Serial.print(udp.remoteIP());
    Serial.print(":"); 
    Serial.println(udp.remotePort());

    /*
    //send packagesize and package amount
    //calculate simple checksum for header validation
    uint16_t * amount = &packageAmount; 
    uint16_t * packagesize = &packetInterval; 
    delay(2000);
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write((uint8_t*)amount, 2);
    udp.write((uint8_t*)packagesize, 2);
    udp.endPacket();
    */
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
  info += "\nPackages: " + (String)packageAmount;
  info += "\nPackagesize: " + (String)packetInterval;
  display.drawString(5, 5, info);
  display.display();
  delay(2000);
  display.clear();

  //network screen
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  info = WiFi.localIP().toString();
  info += ":";
  info += localPort;
  display.drawString(64, 0, info);
  display.drawString(64, 10, "Waiting for Matlab!");
  display.display();
}


//----- utility functions -----
void sendPacket(){
  Serial.print("Sending packet to "); Serial.print(udp.remoteIP()); Serial.print(":"); Serial.println(udp.remotePort());
  uint16_t* packetstart = buffer.getCurrPointer();
  const uint16_t * amount = &packageAmount; 
  //packetstart--; //reduce pointer to compensate for 0
  packetstart = packetstart - packetInterval; //move pointer to beginning of packet
  udp.beginPacket(udp.remoteIP(), udp.remotePort());
  udp.write((uint8_t*)amount, 2);
  udp.write((uint8_t*)packetstart, bufferSize); //Using full buffersize to compensate 8bit sending
  udp.endPacket();
}

uint16_t readData(bool dummy){
  static uint16_t data = 0;
  int dummySawValue = 4095; //sawtooth signal amplitude 
  int sawAmplifier = 4;



      if (!dummy){//sensor data
        data = analogRead(analogIn);
        //clip faulty data
        if (data > 4095) data = 4095;
        else if(data < 0) data = 0;

      }else{//dummy data
        if (data > dummySawValue) data = 0;
        data += sawAmplifier;
        data = 42;
      }
  return data;
}

void matlabPing(){
  uint8_t buffer[50];

  //await response 
  int remBytes = udp.parsePacket();
  //await response 
  if(udp.read(buffer, remBytes) > 0){
      Serial.print("Message recieved: ");
      Serial.println((char *)buffer);
      connected = true;
  }

}

void calcRuntime(){
  static long int loopCount = 0;
  static long int lastStamp = micros();
  static int loopTime = micros()-lastStamp;
  static String t = "";

  loopTime = micros()-lastStamp;
  if (loopTime >= 1000*1000){
    loopTime = (loopTime)/loopCount;
    lastStamp = micros();
    loopCount = 0;
    
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(DejaVu_Sans_Mono_10);
    display.setColor(BLACK);
    display.drawString(0, 5, t); //remove old time
    display.setColor(WHITE);
    t = String(loopTime);
    display.drawString(0, 5, t);
    display.drawString(30, 5, "µs");
    display.display();
  }
  loopCount++;
}

void loadingCircle(){
  int amountBars = 12;
  int zeroX = 64;
  int zeroY = 45; 
  int innerRadius = 8;
  int outerRadius = 15;
  float angle = (2*PI) / amountBars;
  static int x1, x2, y1, y2;
  static int lastX1 = -1, lastX2, lastY1, lastY2;
  static int i = 0;

  static int barDelay = 100;
  static int stamp = millis();


  if (millis() - stamp > barDelay){

    //Serial.print("White Line index "); Serial.print(i); Serial.print(" from ["); Serial.print(x1); Serial.print("|"); Serial.print(y1); Serial.print("]");
    //Serial.print(" to ["); Serial.print(x2); Serial.print("|"); Serial.print(y2); Serial.println("]");

    x1 = zeroX + cos((float)i*angle) * innerRadius;
    x2 = zeroX + cos((float)i*angle) * outerRadius;
    y1 = zeroY + sin((float)i*angle) * innerRadius;
    y2 = zeroY + sin((float)i*angle) * outerRadius;

    display.setColor(BLACK);
    display.drawLine(x1, y1, x2, y2);

    if (lastX1 != -1){
      display.setColor(WHITE);
      display.drawLine(lastX1, lastY1, lastX2, lastY2);
      //Serial.print("Black Line index "); Serial.print(i); Serial.print(" from ["); Serial.print(lastX1); Serial.print("|"); Serial.print(lastY1); Serial.print("]");
      //Serial.print(" to ["); Serial.print(lastX2); Serial.print("|"); Serial.print(lastY2); Serial.println("]");
    }

    display.display();

    lastX1 = x1;
    lastX2 = x2;
    lastY1 = y1;
    lastY2 = y2;



    i++;
    if (i >= amountBars) i = 0;
    stamp = millis();
  }
}


