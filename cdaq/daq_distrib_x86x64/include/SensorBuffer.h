#ifndef SENSOR_BUFFER_H
#define SENSOR_BUFFER_H

#define TB_ERROR_MSG_SIZE	100

/** 
 * Implement a circular FIFO 3D array of float. The array can handle several sensors,
 * each having several channels. 	New information is inserted using SetData(sensor,channel,value) 
 * and retrieved with GetData(sensor,channel,value). 
 *
 * The methods NewFrame and NextFrame manage two internal pointers that indicate where to insert 
 * and retrieve data. Once the information associated with a frame has been fully inserted by
 * calling SetData as many time as necessary, then calling NewFrame will commit this insertion
 * and update the current internal pointer. NewFrame will fail if the next frame has not yet
 * been read (Overwrite error).
 * 
 * Data can be retrieved using the GetData and NextFrame methods. Once a frame has been completely
 * read by calling possibly several times GetData, NextFrame will update an internal pointer to the
 * next unread frame. NextFrame will fail if all frames have been read. 
 *	
 * The method GetNumFramesAvailable returns the number of unread frames.
 */
class SensorBuffer {
public:
	SensorBuffer();
	~SensorBuffer();

	int init(int nFrames, int nSensors, int nChannels);

	// increment lastRead pointer and decrement numFramesAvaialbe pointer  
	int nextFrame();    
	// increment numFramesAvaialbe current pointer 
	int newFrame();	
	int getNewFrame();	
	int getNumFramesAvailable();
	int getCurrent();

	// get data from frame pointed by firstUnRead pointer 
	int getData(int sensor, int channel, float &value); 
	// get data from a specific frame
	int getDataF(int frame, int sensor, int channel, float &value); 

	//	int getLatestData(int frame, int sensor, int channel, float &value); 

	// set data to frame pointed by current pointer
	int setData(int sensor, int channel, float value);

	/** Get a string with information on the error
     * @return a pointer toward the string with the error information
     */
	char* getErrorInfo();
private:
	// circular buffer used to store the data
	float *data;
	float ***dataSample;		// adress dataSample[sample][sensor][channel]
		 
	// size of the circular buffer
	int numFrames;
	int numSensors;
	int numChannels;

	int current;				// pointer in the circular buffer toward the last acquired frame
	int firstUnRead;				// pointer toward the last read frame

	int numFramesAvailable;		// number of unread frames in the circular buffer

	char errorMsg[TB_ERROR_MSG_SIZE];
};

#endif // SENSOR_BUFFER_H
