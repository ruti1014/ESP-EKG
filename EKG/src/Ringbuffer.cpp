#include "Ringbuffer.h"


Ringbuffer::Ringbuffer(int buffersize, void (*intervalCallback)(void), int callbackInterval):_RBsize(buffersize)          //Konstruktor 
{   
    //intialize variables
    _callbackInterval = callbackInterval; //no upper limit check
    _RBArray = new uint16_t[_RBsize];
    _arrayptr = _RBArray;
    _intervalCallback = intervalCallback;
}


void Ringbuffer::addData(uint16_t sensorData){

    _lastVal = sensorData;
    //counter Handling
    static int circularCount = -1;
    if ((circularCount >= _RBsize - 1) && (!_circularMode)) _circularMode = true; //circularMode false until array is for the first time 

    if (circularCount >= _RBsize - 1) circularCount = 0; //circularCount keeping track of elements added
    else circularCount++;

    //callback Handling
    if (((circularCount+1)%_callbackInterval == 0) && (_callbackInterval > 0)){ //callback function in set interval e.g.: _RBsize = 10, call every 5th added element
        _intervalCallback();
    } 


    //array and data handling
    if (!_circularMode) {   //buffer is not yet Full
        _arrayptr[circularCount] = sensorData;
    }
    else {                          //Buffer is full and works in circular mdoe
        *_arrayptr = sensorData;

        //pointer arithmetic
        if (_arrayptr >= (_RBArray + _RBsize - 1)) {  //if pointerADC is at the end wrap pointer around
            _arrayptr = _RBArray;
        }
        else {
            _arrayptr++;      //increase both pointers symmetrical
        }

    }

}


uint16_t Ringbuffer::getData(int i) {

    i = abs(i); // if negative flip sign
    //check if i is out of bounds; if so return last Element
    if (i > _RBsize - 1) i = _RBsize - 1;

    int absIndex = _convertIndex(i); //convert index from relative to absolute -> starting position changes with every added element

    return _RBArray[absIndex];
}

void Ringbuffer::getCurrPointer(uint16_t *tempPtr){
    tempPtr = _arrayptr;
}

void Ringbuffer::getStartPointer(uint16_t *tempPtr){
    tempPtr = _RBArray;
}

uint16_t Ringbuffer::getLastVal(){
    return _lastVal;
}

int Ringbuffer::_convertIndex(int i) {
    int index;
    if (!_circularMode) { //until first filled absolute index = relative index
        index = i;
    }
    else {
        index = ((_arrayptr - _RBArray) + i) % (_RBsize); //absolut index can be retrieved with modulo
    }

    return index;
}

