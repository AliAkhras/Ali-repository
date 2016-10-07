/*
	AMTIlib.cpp

*/


#include "AMTIlib.h"
#include "AMTIUSBDeviceDefinitions.h"
// #include "afxwin.h"

#include "table.h"
// #include <atlstr.h>

// AMTI library
Table configAMTI;
float *currentPtr;

// DLL Initialization Using a Sleep Statement
int InitializeDeviceDLL(void)
{
	fmDLLInit();
	Sleep(250);
	int countdown = 20;
	while(fmDLLIsDeviceInitComplete() == 0)
	{
		Sleep(250);
		if ( countdown-- <= 0 )
			{
			printf("Timeout Error: Not completed initializing the DLL \n%s\n"); 
			}
	}
	printf("The DLL is initialized \n%s\n"); 
	int ret = fmDLLSetupCheck();
	// If return is not 1 configuration has changed
	// Go to funtion description for more information
	//ConfigureDataCollection();
	switch(ret){
	case 0:
		printf("No signal conditioners were found\n");
		break;
	case 1:
		printf("The current setup is the same as the last saved configuration\n");
		break;
	case 211:
		printf("The configuration file was not found\n");
		break;
	case 213:
		printf("A configuration file was found but for the wrong version of the software\n");
		break;
	case 214:
		printf("The configuration has changed: a different number of signal conditioners were detected than the previously saved setup\n");
		break;
	case 215:
		printf("The configuration has changed: the serial numbers of the signal conditioners don’t match the previously saved setup\n");
		break;
	default:
		break;
	}
	return ret;
};


// An MFC dialog class being setup to do data collection using windows messaging
//Standard Data collection settings that should be set
void ConfigureDataCollection(int DataCollectionMode, int DataSyncronizationMode, int RunMode)
{
	switch(DataCollectionMode){
	case 0:
		fmDLLPostDataReadyMessages(FALSE);
		break;
	case 1:
		// HWND h_Wnd;
		//Decided to post messages to a window
		fmDLLPostDataReadyMessages(TRUE);
		/*
		h_Wnd = GetSafeHwnd();
		fmDLLPostWindowMessages((HWND) h_Wnd);
		*/
		break;
	default:
		break;
	}
	/*
		The current size of a packet is 512 bytes. Each packet has 16 datasets, 
		with 8 elements in each dataset. Each element is a 4-byte IEEE floating point value.
		The 8 elements consist of a dataset counter, 6 data channels, and a trigger channel.
	*/
	fmDLLSetUSBPacketSize(512);				// Set the packet size to 512
	/*
	// AMTI digital signal conditioners have a genlock input port, 
	   which is designed to receive a clocking pulse that will cause a dataset to be recorded. 
	   When genlock is on, the signal conditioner will collect a single dataset on either 
	   the rising or falling edge of an analog input signal, usually a square wave of some sort.
	*/
	fmBroadcastGenlock(DataSyncronizationMode); // Make sure Genlock is off
	/*
		For digital data, if the units are metric the forces are newtons and the moments are newton-meters. 
		If the units are English the forces are pounds and the moments are foot-pounds. If the units are bits
		the full scale range is ±16384 bits for all channels.
	*/
	fmBroadcastRunMode(RunMode);				// Set collection mode 
	fmBroadcastResetSoftware();					//Apply the settings
	Sleep(250);	
}
void setDeviceConfiguration(char *CONFIG_FILE){
	int numDevices;
	// int cableLen;
	int currentGain[6];
	float currentExc[6];
	float zeroOffset[6];
	// Configure task
	// read configuration file for EMS
	configAMTI.ReadTable(CONFIG_FILE);
	int row=0;
	int format					 = (int) configAMTI.Get(row++,0);
	int acqRate					 = (int) configAMTI.Get(row++,0);
	int currentGainValue         = (int) configAMTI.Get(row++,0);
	float currentExcValue        = configAMTI.Get(row++,0);
	float zeroOffsetValue	     = configAMTI.Get(row++,0);
	int DataCollectionMode	     = (int) configAMTI.Get(row++,0);
	int DataSyncronizationMode	 = (int) configAMTI.Get(row++,0);
	int RunMode					 = (int) configAMTI.Get(row++,0);
	float PlatformRotation		 = configAMTI.Get(row++,0);
	// set configurations
	fmDLLSetDataFormat(format);
	fmBroadcastAcquisitionRate(acqRate);
	ConfigureDataCollection(DataCollectionMode, DataSyncronizationMode,RunMode);
	fmSetPlatformRotation(PlatformRotation);

	for(int i = 0; i<6; i++)
	{
		currentGain[i] = currentGainValue; 
		currentExc[i]  = currentExcValue; 
		zeroOffset[i]  = zeroOffsetValue;
	}
	numDevices = fmDLLGetDeviceCount();
	for(int i = 0;i < numDevices;i++)
	{
		fmDLLSelectDeviceIndex(i);
		fmSetCurrentExcitations((long*)currentExc);
		fmSetCurrentGains((long*)currentGain);
		fmSetChannelOffsetsTable((float*)zeroOffset);
	}
	// fmSetCableLength(cableLen);
	fmResetSoftware();
	Sleep(250);	
	// Print settings and device information
	// printf("Successfully configured the task\n");
	getDeviceConfiguration();
	printf("Data format: %u \n",format);
	printf("\t  \t \t ******************************************************************* \n");
	
};

void getDeviceConfiguration(void){
	int i;
	char len[30];
	char width[30];
	char model[30];
	char serial[30];
	char theDate[30];
	// float lenWdth[2];
	float offsets[3];
	float chOffsets[6];
	float sen[36];
	float bridgeResis[6];

	memset(len,'\0',30);
	memset(width,'\0',30);
	memset(model,'\0',30);
	memset(serial,'\0',30);
	memset(theDate,'\0',30);
	offsets[0] = 0.0;
	offsets[1] = 0.0;
	offsets[2] = 0.0;
	for(i = 0;i < 6;i++)
	{
		bridgeResis[i] = 0.0;
		chOffsets[i] = 0.0;
	}
	for(i =0;i< 36;i++)
	{
		sen[i] = 0.0;
	}

	//Print settings and device information
	printf("AMTI Settings:\n");
	printf("\tNumber of Devices: %u \n",fmDLLGetDeviceCount());
	printf("\tProduct Type: %ld \n",fmGetProductType());
	printf("\tVersion ID: %u \n", fmDLLGetVersionID());
	printf("\tCable Length: %f \n",fmGetCableLength());
	printf("\tDLL acquisition rate: %u \n", fmDLLGetAcquisitionRate());
	printf("\tThe Acquisition Rate being Broadcast to the Signal Conditioners: %u \n", fmGetAcquisitionRate());
	printf("\tPlatform Rotation: %f \n",fmGetPlatformRotation());
	printf("\tRun mode: %d \n",fmDLLGetRunMode());
	// Retrieving Some Parameters
	int NumberOfSensors=fmDLLGetDeviceCount();
	printf("Platform parameters:\n");
	for (int k = 0; k<NumberOfSensors; k++){
		fmDLLSelectDeviceIndex(k);
		fmGetPlatformModelNumber(model);
		fmGetPlatformSerialNumber(serial);
		fmGetPlatformDate(theDate);
		fmGetPlatformLengthAndWidth((char *)len,(char *) width);
		fmGetPlatformXYZOffsets(offsets);
		fmGetPlatformBridgeResistance(bridgeResis);
		fmGetInvertedSensitivityMatrix(sen);
		fmGetChannelOffsetsTable(chOffsets);
		printf("\tPlatform %d \n",k+1);
		printf("\tPlatform model: %s \n",model);
		printf("\tPlatform serial: %s \n",serial);
		printf("\tPlatform date: %s \n",theDate);
		printf("\tPlatform length: %s \n",len);
		printf("\tPlatform width: %s \n",width);
		printf("\tX, Y, and Z platform offsets: %6.3f, %6.3f, %6.3f \r\n", offsets[0], offsets[1], offsets[2]);
		printf("\tBridge resistance array in ohms Fx, Fy, Fz, Mx, My, Mz: %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f \r\n", bridgeResis[0], bridgeResis[1], bridgeResis[2], bridgeResis[3], bridgeResis[4], bridgeResis[5]);
		printf("\tChannel offset for each force and moment channel Fx, Fy, Fz, Mx, My, Mz: %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f \r\n", chOffsets[0], chOffsets[1], chOffsets[2], chOffsets[3], chOffsets[4], chOffsets[5]);
		printf("\tInverted sensitivity matrix \n");
		for(int x=0;x<6;x++){
            printf("\t %6.4f,\t %6.4f,\t %6.4f,\t %6.4f,\t %6.4f,\t %6.4f \r\n", sen[6*x+0], sen[6*x+1], sen[6*x+2],sen[6*x+3], sen[6*x+4], sen[6*x+5]);
		}
	}
	
};

// Create the acquisition task     
void creatTask(void){    
	 
	int ok=InitializeDeviceDLL();         // DLL Initialization Using a Sleep Statement
	//if (ok == 0, 211)
	// ConfigureDataCollection();     // Configure the device     
	fmBroadcastZero();             // The Platforms being Zeroed
   
};

// Start the acquisition
void startTask(void){  
		fmBroadcastStart();
		printf("Acquistion started\n");
};

// Stop acquisition task
void stopTask(void){  
	fmBroadcastStop();
	printf("Acquistion stoped\n");
};

long isDataAvailable(){  
	int ret;
	float *ptr;
	//getting the Data
	ret = fmDLLTransferFloatData((float *&)ptr);
	// ret = fmDLLTransferFloatData(ptr);
	if(ret == 0 )
	{
		return 0;
	}
	setDataset(ptr);

	return ret;
};
float *setDataset(float *ptr){
	currentPtr=ptr;
	return currentPtr;
}
float *getDataset(void){
	return currentPtr;
}

float getSamplingRate(void){  
		float SamplingRate=(float)fmGetAcquisitionRate(); 
	return SamplingRate;
};
int getNumSensor(void){
	int numDevices = fmDLLGetDeviceCount();
	return numDevices;
};

int saveConfiguration(void){
	// Save the current DLL settings to a configuration file stored in the AMTI configuration directory. 
	// The configuration file is named AMTIUsbSetup.cfg.
	int ok=fmDLLSaveConfiguration();
	if (ok){
		printf("Data saved to a configuration file is named AMTIUsbSetup.cfg\n");
	}
	else {
		printf("Failed: it could not save DLL settings \n");
	}
	return ok;
};

//close task
void closeTask(void){ 
	fmDLLShutDown();
};