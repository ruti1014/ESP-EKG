#include <Arduino.h>
#include "SSD1306Wire.h" 
#include "Ringbuffer.h"
#include "Graph.h"


//#define I2Cspeed 1000000
#define analogIn 27
#define adcFreq 250
#define dataCaptureTime 30




//function prototypes
void IRAM_ATTR onADCTimer();
void onBuffer();
void initdisplay();
void initTimer();
void startUpScreen();

//global settings
//static variables
static int timerPreScaler = 80; //80 for 1 MHz increments at a cpu speed of 80 MHz
static int adcTimeStep = (CPU_CLK_FREQ/timerPreScaler) / adcFreq; //calculate timerFrequency in µs based on cpu speed, prescaler and user set frequency in Hz
static int bufferSize = (dataCaptureTime * 1000 * 1000)/adcTimeStep; //calculate buffersize to contain values for the set time range

//variables 
bool newData = false;
float estGraphTime;

//objects
hw_timer_t *adcTimer = NULL;
SSD1306Wire display(0x3c, SDA, SCL);
//SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_128_64, I2C_ONE, 1000000);
Ringbuffer buffer(bufferSize, &onBuffer, bufferSize/2);
GraphSettings settings;
Graph graph(&settings, &display);

void setup() {
  Serial.begin(9600);
  Serial.print("ADC running at "); Serial.print(adcTimeStep); Serial.println(" µs.");
  Serial.print("Buffer size "); Serial.print(bufferSize); Serial.print(" capturing for "); Serial.print(dataCaptureTime); Serial.println(" s.");

  initdisplay();


  //----- graph settings -----
  settings.title = "EKG";
  settings.x = 10;
  settings.y = 20;
  settings.length = 110;
  settings.height = 40;
  //optional graph settings
  settings.movingAverage = 30;
  settings.drawAxis = true;
  settings.drawArrowheads = true;
  settings.linemode = true;


  estGraphTime = ((settings.movingAverage * settings.length) * (adcTimeStep/1000000.0)); //avg = datapoints per pixel  times graph length times adcStepTime in seconds for estimate graph time
  Serial.print("Estimated graph time: "); Serial.print(estGraphTime); Serial.println(" s");

  //Timer setup
  initTimer();

  //init screens
  //startUpScreen();


  graph.drawGraphMeta();
}

void loop() {
  if (newData){
  graph.updateGraph(buffer.getLastVal());
  newData = false;
  }
}

//----------------------------- Custom Functions -----------------------------

// ISR

//timer isr
void IRAM_ATTR onADCTimer(){ 
  buffer.addData(analogRead(analogIn));
  newData = true;
}

//buffer isr
void onBuffer(){

}

//custom functions

void initdisplay(){
  display.init();
  display.flipScreenVertically();
  display.setFont(DejaVu_Sans_Mono_10);
  display.clear();

  display.setColor(WHITE);
}

void initTimer(){
  //ADC-Timer
  adcTimer = timerBegin(0, timerPreScaler, true);
  timerAttachInterrupt(adcTimer, &onADCTimer, true);
  timerAlarmWrite(adcTimer, adcTimeStep, true);
  timerAlarmEnable(adcTimer);
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