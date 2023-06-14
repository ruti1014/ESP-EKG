#include "Graph.h"

Graph::Graph(GraphSettings *graph, SSD1306Wire *display){
    _graph = graph;
    _display = display;
};

void Graph::updateGraph( int newDataPoint){ //dataLow and High set the range of the value, default set to analogRead range
    static int averageCounter = 0;
    static int lastX = _graph->x;
    static int lastY = _graph->y + _graph->height;
    static int timeFrame = millis();
    static float graphTime = -1;

    //check if moving average is ready to calculate
    if (averageCounter >= _graph->movingAverage - 1){
        
        if (_graph->movingAverage > 1){ //prevent division by zero
            _movingAverage += newDataPoint; //add Last datapoint
            _movingAverage = _movingAverage/_graph->movingAverage; //calculate average over give range
        }else{
            _movingAverage = newDataPoint; //if average is set to less than 2 it will displayed directly
        }

        //map value from analogRead range to graph range
        _movingAverage = map(_movingAverage, _lowDataRange, _highDataRange, 0, _graph->height); //map dataPoint to graph range
        
        //calculate absolute x and y positions for graph -> currX represents relative x position on graph
        int x = _graph->x + _currX; //graph x start + currX position
        int y = _graph->y + (_graph->height - _movingAverage); //graph y start plus difference graph height and dataPoint -> "graph is being populated from 'above'"

        //check if x or y clip out of graph range
        if (y > (_graph->y + _graph->height)) y = (_graph->y + _graph->height); 
        if (y < _graph->y) y = _graph->y;
        
        //Set displayBar and deleteBar (maybe delete only the pixel/line from last display???)
        _display->setColor(BLACK);//delete current x point by drawing a black bar over it
        _display->drawLine(x, _graph->y, x, _graph->y + _graph->height);
        //location bar for ui
        if (_currX < (_graph->length)){
            _display->setColor(WHITE);
            _display->drawLine(x+1, _graph->y, x+1, _graph->y + _graph->height);
        }

        //set dataPixel/drawLine
        if (!_graph->linemode){
            _display->setPixel(x, y);
        }else {
            _display->drawLine(lastX, lastY, x, y);

            if (_currX < (_graph->length)) lastX = x;
            else lastX = _graph->x;
            lastY = y;
        }
        _display->display();

        //set pixelPointer to next pixel
        if (_currX >= _graph->length) {
            _currX = 0;


            //display graph time
            //Remove old time
            if (graphTime >= 0){
            _display->setColor(BLACK);
            _display->drawString(_graph->x + _graph->length - 10, _graph->y - 15, ((String)(graphTime) + " s"));
            }
            //draw new Time
            timeFrame = millis() - timeFrame;
            graphTime = timeFrame/1000.0;
            _display->setColor(WHITE);
            _display->drawString(_graph->x + _graph->length - 10, _graph->y - 15, ((String)(graphTime) + " s"));
            _display->display();
            timeFrame = millis();
        }else _currX++;

        //reset moving average
        _movingAverage = 0;
        averageCounter = 0;
    }else {
        //add value for averaging and increase average counter
        _movingAverage += newDataPoint;
        averageCounter++;
    }
};

void Graph::drawCompleteFrame(Ringbuffer *buffer){

    int dataFrame = 500;
    int avgStep = buffer->getSize()/_graph->length;
    int avgCounter = 1;
    int x, y;
    int lastX = _graph->x;
    int lastY = _graph->y;

    //reset graph area
    _display->setColor(BLACK);
    _display->drawRect(_graph->x, _graph->y, _graph->length, _graph->height);
    _display->display();
    _display->setColor(WHITE);
    int data;
    for (int i = 0; i < dataFrame; i++){
        
        //calculate average
        if (avgCounter >= avgStep){
            data += buffer->getData(i);
            avgCounter++;
        }else {
            Serial.println(data);
            data = data/avgCounter;
            avgCounter = 1;

            x = _graph->x + i;
            y = map(data, _lowDataRange, _highDataRange, 0, _graph->height);
            y = y + _graph->height;
            //check if x or y clip out of graph range
            if (y > (_graph->y + _graph->height)) y = (_graph->y + _graph->height); 
            if (y < _graph->y) y = _graph->y;

            if (x > (_graph->x + _graph->length)) x = (_graph->x + _graph->length); 
            if (x < _graph->x) x = _graph->x;
            


            _display->drawLine(lastX, lastY, x, y);
            x = lastX;
            y = lastY;
        }
    }
    _display->display();


}

//Draw fixed graphics for the graph i.e. Axis and arrows
void Graph::drawGraphMeta(){
    _display->clear();
    //set graph title
    _display->setFont(DejaVu_Sans_Mono_10);
    _display->setTextAlignment(TEXT_ALIGN_CENTER);
    _display->drawString(_graph->x + (_graph->length/2), _graph->y - 15, _graph->title);

    //draw axis
    if (_graph->drawAxis){
        _display->drawLine(_graph->x - 1, _graph->y + _graph->height + 1, _graph->x + _graph->length, _graph->y + _graph->height + 1); //x-axis (x0, y0, x1, y0);
        _display->drawLine(_graph->x - 1, _graph->y + _graph->height + 1, _graph->x - 1, _graph->y); //y-axis
    }

    //draw axis arrows
    if (_graph->drawArrowheads){
        _display->drawXbm(_graph->x - 3, _graph->y - 5, 5, 5, arrowHeadXMB_X);
        _display->drawXbm(_graph->x + _graph->length + 1, _graph->y + _graph->height - 1, 5, 5, arrowHeadXMB_Y);
    }
    _display->display();
    _display->setFont(DejaVu_Sans_Mono_8);
}

