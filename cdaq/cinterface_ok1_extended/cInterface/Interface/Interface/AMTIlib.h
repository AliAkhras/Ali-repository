#include <stdio.h> //printf
#include <string.h>
#include <malloc.h>
#include <math.h>	
#include <Windows.h> //Sleep
#include <conio.h>	//_kbhit, getch


// Function to configure, creat and start tasks
int InitializeDeviceDLL(void);
void ConfigureDataCollection(int DataCollectionMode, int DataSyncronizationMode, int RunMode);
void setDeviceConfiguration(char *CONFIG_FILE);
void setAcquisitionConfiguration(char *CONFIG_FILE);
void getDeviceConfiguration(void);
void creatTask(void);
void startTask(void);
void stopTask(void);
long isDataAvailable();
float *setDataset(float *ptr);
float *getDataset(void);
void eventOptions(void);
void InitPlotWave(void);
bool InitPlotBars(void);
void AMTIsetting(void);
float getSamplingRate(void);
int getNumSensor(void);
int saveConfiguration(void);
void closeTask(void);
long int unix_timestamp();











