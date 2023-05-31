#ifndef RINGBUFFER_H
#define RINGBUFFER_H  
#include <Arduino.h>

class Ringbuffer
{
public:
    Ringbuffer(int buffsize, void (*intervalCallback)(void), int callbackInterval = -1); //callbackinervall default -1 -> no callback function
    void addData(uint16_t sensorData);
    uint16_t getData(int i);
    uint16_t getLastVal();


private:
    int _convertIndex(int i);

    int _RBsize;
    uint16_t * _RBArray;    //Buffer Array
    uint16_t * _arrayptr;
    bool _circularMode = false;
    int _callbackInterval;
    void (*_intervalCallback)(void);
    uint16_t _lastVal;
};

#endif
