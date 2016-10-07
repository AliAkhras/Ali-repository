/*
   SensorBuffer.cpp

*/
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "SensorBuffer.h"

#define FAILURE	0
#define SUCCESS 1


/*

  constructor and destructors

*/
SensorBuffer::SensorBuffer(void){
	data=NULL;
	dataSample=NULL;
	current=firstUnRead=numFramesAvailable=0;
};

SensorBuffer::~SensorBuffer(){
	free(data);
	if(dataSample!=NULL) {
		for(int sample=0;sample<numFrames;sample++) {
			free(dataSample[sample]);
		}
		free(dataSample);
	}
};

char* SensorBuffer::getErrorInfo(){
	return errorMsg;
};

int SensorBuffer::init(int nFrames, int nSensors, int nChannels){

	numChannels= nChannels;								 
	numSensors = nSensors;								 
	numFrames = nFrames;

	// allocate a big block of contiguous memory for the buffer
	data = (float *)calloc(numFrames*numSensors*numChannels, sizeof(float));
	if(data==NULL) {
		sprintf_s(errorMsg,TB_ERROR_MSG_SIZE,"SensorBuffer::Init - Memory allocation error\n");
		return FAILURE;
	}

	// dataSample provide a quick way to accessing to data in the buffer
	//	 dataSample[sample][sensor][channel]
	// It assumes that data are grouped by sample, sensor and then channel 
	dataSample = (float ***)malloc(numFrames*sizeof(float **));
	if(dataSample==NULL) {
		sprintf_s(errorMsg,TB_ERROR_MSG_SIZE,"SensorBuffer::Init - Memory allocation error\n");
		free(data);
		return FAILURE;
	}

	for(int sample=0;sample<numFrames;sample++) {
		dataSample[sample] = (float **)malloc(numSensors*sizeof(float *));
		if(dataSample[sample]==NULL) {
			sprintf_s(errorMsg,TB_ERROR_MSG_SIZE,"SensorBuffer::Init - Memory allocation error\n");
			for(int i=0;i<sample;i++)free(dataSample[i]);
			free(data);
			return FAILURE;
		}
		for(int sensor=0; sensor<numSensors; sensor++) {
			// data are assumed to be grouped by sample, sensor and then channel in the 1D array v
			dataSample[sample][sensor] = data + sample*numSensors*numChannels + sensor*numChannels;		}
	}
	
	return SUCCESS;
};

// Given that it is a circular buffer, it is not possible to rely 
// on the difference between current and lastRead to know the number of
// available frames. For example, current-lastRead will give a wrong
// results when current has wrapped around the end of the buffer.
int SensorBuffer::getNumFramesAvailable(){
	return numFramesAvailable;
};

// increment numFramesAvaialbe current pointer 
int SensorBuffer::newFrame(){
	numFramesAvailable++;
	if(numFramesAvailable==numFrames){
		numFramesAvailable=numFrames-1;
		sprintf_s(errorMsg,TB_ERROR_MSG_SIZE,"SensorBuffer::NewFrame - Buffer is full\n");
		return FAILURE;
	}
	current++;
	if(current==numFrames){
		current=0;
	}
	return SUCCESS;
};		

// get number of frame available in the data buffer after increaments 
int SensorBuffer::getNewFrame(){
	return numFramesAvailable;
};
// get update pointer toward current frame in the data buffer
int SensorBuffer::getCurrent(){
	//current++;
	//if(current==numFrames) {
	//	current=0;
	//}
	return current;
};
// increment firstUnRead pointer and decrement numFramesAvaialbe pointer  
int SensorBuffer::nextFrame(){
	if(numFramesAvailable==0) {
		sprintf_s(errorMsg,TB_ERROR_MSG_SIZE,"SensorBuffer::NextFrame  - No Frame available\n");
		return FAILURE;
	}
	numFramesAvailable--;
	firstUnRead++;
	if(firstUnRead==numFrames) {
		firstUnRead=0;
	};
	return SUCCESS;
};    


// get data from frame pointed by lastRead pointer 
int SensorBuffer::getData(int sensor, int channel, float &data){
	if(numFramesAvailable==0){
		sprintf_s(errorMsg,TB_ERROR_MSG_SIZE,"SensorBuffer::GetData - No Frame available\n");
		return FAILURE;
	}
	if(sensor<0 || sensor>=numSensors || channel<0 || channel>=numChannels) {
		sprintf_s(errorMsg,TB_ERROR_MSG_SIZE,"SensorBuffer::GetData - Subscript out of range )%d %d)\n",
			sensor,channel);
		return FAILURE;
	}
	data=dataSample[firstUnRead][sensor][channel];
	return SUCCESS;
}; 


int SensorBuffer::getDataF(int frame, int sensor, int channel, float &data){
	if(frame<0 || frame>=numFrames || sensor<0 || sensor>=numSensors || channel<0 || channel>=numChannels) {
		sprintf_s(errorMsg,TB_ERROR_MSG_SIZE,"SensorBuffer::GetData - Subscript out of range (%d %d %d)\n",
			frame,sensor,channel);
		return FAILURE;
	}
	data=dataSample[frame][sensor][channel];
	return SUCCESS;
}; 


// set data to frame pointed by current pointer
int SensorBuffer::setData(int sensor, int channel, float value){
	if(sensor<0 || sensor>=numSensors || channel<0 || channel>=numChannels) {
		sprintf_s(errorMsg,TB_ERROR_MSG_SIZE,"SensorBuffer::GetData - Subscript out of range )%d %d)\n",
			sensor,channel);
		return FAILURE;
	}
	dataSample[current][sensor][channel]=value;
	return SUCCESS;
};
