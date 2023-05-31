#ifndef GRAPH_H
#define GRAPH_H
#include <Arduino.h>
#include "SSD1306Wire.h"

struct GraphSettings{
  String title;
  int x, y, //top right corner -> starting positon
  length, height,
  movingAverage = -1;
  bool drawAxis = false, //Axis and arrow dont regard the display dimension
  drawArrowheads = false,
  linemode = false; //instead of single pixel, there is a line drawn between current and last (possibly slower)
};

class Graph{
    public:
        Graph(GraphSettings *graph, SSD1306Wire *display);

        void updateGraph(int newDataPoint);
        void drawGraphMeta(); //note that axis are additional to lenght and height

    private:
        GraphSettings *_graph;
        SSD1306Wire *_display;
        int _currX;
        int _currY;
        int _movingAverage;
        int _lowDataRange = 0; //lowest and highest value of the incoming data
        int _highDataRange = 4095;   //default is set to analogRead range
};


//Arrowhead
static uint8_t arrowHeadXMB_X[] = {
    0x04, 0x04, 0x0E, 0x1B, 0x11
};

static uint8_t arrowHeadXMB_Y[] = {
    0x03, 0x06, 0x1C, 0x06, 0x03
};

//graph Logo
static uint8_t logo[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 
  0x00, 0x80, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 
  0x00, 0x06, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 
  0x00, 0x80, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 
  0x00, 0x06, 0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0x06, 0x00, 0x00, 
  0x00, 0x80, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 
  0x00, 0x0F, 0x00, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0x05, 0x00, 0x00, 
  0x00, 0xC0, 0x02, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 
  0x00, 0x09, 0x00, 0x00, 0x00, 0xC0, 0x02, 0x00, 0x00, 0x0D, 0x00, 0x00, 
  0x00, 0x40, 0x02, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 
  0x00, 0x0D, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x09, 0x00, 0x00, 
  0x00, 0x60, 0x02, 0x00, 0x00, 0x09, 0x00, 0x00, 0xE0, 0x7F, 0xFA, 0xFF, 
  0xFF, 0xC9, 0xFF, 0x00, 0xA0, 0x1A, 0x7A, 0xAA, 0xA5, 0xE8, 0x5A, 0x00, 
  0x00, 0x00, 0x0E, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 
  0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x38, 0x00, 0x00, 
  0x00, 0x00, 0x0C, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 
  0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x18, 0x00, 0x00, 
  0x00, 0x00, 0x04, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 
  0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  };


#endif