#include <Windows.h> //Sleep & current value of the performance counter which is a high resolution (<1us) time stamp that can be used for time-interval measurements
#include <conio.h>	//_kbhit, getch
#include <stdio.h> //printf
#include <math.h> 
#include <ctype.h> 
#include <Mmsystem.h>
#include <iostream>
#include <string>
#include <time.h>  
#include <ctime>
#include <sstream>
#include <stdlib.h> // malloc, exit, atof

extern "C" {
#include "timer.h"
}

#include "table.h"

// Graphics
#include "GL/freeglut.h"
#include "PlotBar.h"
#include "plotWaves.h"
// AMTI  
#include "AMTIlib.h" 
// #include "AMTIUSBDeviceDefinitions.h" //Include Dll header
#include "SensorBuffer.h"

// yarp
#include <yarp/os/all.h>
#include <yarp/sig/all.h>
// #include <yarp/dev/all.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
// using namespace yarp::dev;

// configuration files
#define CONFIG_FILE_AMTI "C:\\Ali\\cdaq\\daq_distrib_x86x64\\config\\ConfigAMTI.txt" //Configuration file for AMTI
#define CONFIG_FILE_ACQ  "C:\\Ali\\cdaq\\daq_distrib_x86x64\\config\\ConfigAcq.txt" //Configuration file for AMTI
#define DESIGN_FILE      "C:\\Ali\\cdaq\\daq_distrib_x86x64\\config\\Design.txt"    //design file

#define MAX_LABEL_SIZE 11
#define ESC 27
#define PI  3.1415926535f
 

//State of the task
int taskState;
#define TASK_NOT_READY	 0
#define TASK_CREATED	 1
#define ACQUIRING        2
#define DONE             3
// #define RECORDING		 4
#define INIT_ACQUISITION 4
#define PULSE            5
#define TRIAL_START      6

// state of trigger
int trigState;
#define trigDisable	 0
#define trigEnable	 1
#define recording    2

//Type of graph
int graphType; // graphType: 0 =  wave, 1 = bar

 /*
 Global variables
 */
//AMTI 
Table configAcq;
int maxSamplesAMTI;
int numSamples;    
int preTrigSamples;
int finiteMaxSamples;
const int numChann = 14; // force, torque, pulse

// design
int gTrial = 0;
int maxNumberTrials;
char trialMode[120];
Table		design;

// trial variables
//int trial;
int indxB;
int iEnco=preTrigSamples;
int trigIndx;

// buffers
Table       bufferAMTI;
//Table       dataAMTI;
// SensorBuffer AMTIdataToYarp;
Matrix YarpMat(2,6);         // force, torque vectors to be sent to yarp
double **matTimeStamp ;      // timeStamp storage 

// yarp
Port platform1; // Create a port 1.
Port platform2; // Create a port 2.

int timerID;
TIMER timer;
double timeStartInitAcquisition;

//GLUT and GLUI Windows
int _MainWindow;
int _MainWindowSubject;
PlotWave *pw;
BarPlot *pbAMTI;
BarPlot *pb;
// colors
float WhiteColorArray[3] = {1.0F,1.0F, 1.0F};
float RedColorArray[3] = {1.0F, 0.25F, 0.25F};
float GreenColorArray[3] = {0.25F, 1.0F, 0.25F};

/*
// Configure data recording
void setAcquisitionConfiguration(char *CONFIG_FILE){
	// read acquisition configuration file
	configAcq.ReadTable(CONFIG_FILE);
	int row=0;
	maxSamplesAMTI			 = (const int) configAcq.Get(row++,0);
	numSamples				 = (const int) configAcq.Get(row++,0);
	preTrigSamples           = (const int) configAcq.Get(row++,0);
	//numChann				 = (const int) configAcq.Get(row++,0);
}
*/
// trial management 
int PreviousTrial(){
	if(gTrial>0) gTrial--;
	else printf("First trial\n");
	return gTrial;
}
int NextTrial(){
	if(gTrial < maxNumberTrials-1) gTrial++; // < design.GetRow()-1) gTrial++;
	else {
		printf("All trials are done\n");
		//delete[] trialMode;
		// strcat (trialMode,"End of experiment");
		gTrial=maxNumberTrials;
	}
	return gTrial;
}

void SetTrialParameter(int trial){
	if(trial<0 || trial >= maxNumberTrials) return; // >= design.GetRow() ) return;
	printf("Current trial number: %d\n",trial+1);
	int task = (int) design.Get(trial/(maxNumberTrials/design.GetRow()),0);
	printf("\tSubject Task: ");
	char taskCase[30];
	char condCase[30];

	switch(task){
	case 1:
		strcpy (taskCase,"Squat");

		//currentSamp=new float[getCurrentSample()];// [getChanN
		//InitPlotBarsAMTI();
		printf(taskCase);
		break;
	case 2:
		strcpy (taskCase,"Bowling");
		//currentSamp=new float[getCurrentSample()];// [getChanN
		//InitPlotBarsAMTI();
		printf(taskCase);
		break;
	default:
		break;
	}
	printf("\n condition: ");
	int cond = (int) design.Get(trial/(maxNumberTrials/design.GetRow()),1);
	switch(cond){
	case 1:
		strcpy (condCase,"only human\n");
		printf(condCase);
		break;
	case 2:
		strcpy (condCase,"human robot interaction\n");
		printf(condCase);
		break;
	default:
		break;
	}	

	strcpy (trialMode,"\nTask: ");
	strcat (trialMode,taskCase);
	strcat (trialMode,"\n - ");
	strcat (trialMode,condCase);
}

// saving buffer to an external file 
/*
void SaveRefTrigData(char *filename){
		if (trigIndx>=preTrigSamples-1){
			for (int i=0; i<preTrigSamples; i++){
				for(int j=0; j<bufferAMTI.GetCol(); j++){
						dataAMTI.Set(i, j, bufferAMTI.Get((trigIndx-preTrigSamples+i), j)); 
				}
			}
		}
		else
		{
			for (int i=0; i<(preTrigSamples-trigIndx); i++){
				for(int j=0; j<bufferAMTI.GetCol(); j++){
						dataAMTI.Set(i, j, bufferAMTI.Get(maxSamplesAMTI-(preTrigSamples-trigIndx)+i, j)); 
				}
			}
			for (int i=0; i<trigIndx; i++){
				for(int j=0; j<bufferAMTI.GetCol(); j++){
						dataAMTI.Set(i+(preTrigSamples-trigIndx), j, bufferAMTI.Get(i, j)); 
				}
			}
		}
		dataAMTI.SaveTable(filename);
}
*/
void resetBuffers(){
	// reset data buffer
	int NumberOfSensors=getNumSensor();
	int nbCol = 8*NumberOfSensors + 1; // Data counter Fx Fy Fz Mx My Mz Trigger state, op time
	bufferAMTI.MakeTable(maxSamplesAMTI, nbCol);
	//dataAMTI.MakeTable(numSamples, nbCol);
	indxB=0;
	iEnco=preTrigSamples;
	int nbRow;
	int nbCol_TS;
	nbRow = maxSamplesAMTI;
	nbCol_TS = 3;
	for(int r=0;r<nbRow;r++) {
		for(int c=0;c<nbCol_TS;c++)
		 matTimeStamp[r][c]=0.0;
	}
}

//

void KeyBoard(unsigned char key) {
	switch(key) {
	case 'S':
		if (taskState==TASK_CREATED || taskState==TRIAL_START || taskState==DONE  )// taskState==DONE )
		{
			startTask();
			trigState= recording;
			taskState=ACQUIRING;
		}
		else
			printf("Task is not created\n");
		break;
	case 'O':
		//acquisitionONOFF=0;
		stopTask();
		taskState=DONE;
		break;
	/*
	case 'A':

		trigIndx=indxB;
		printf("%d\n", trigIndx);
		trigState= recording;
		break;
		*/
	case 'B':
		trigState= trigDisable;
		double time;
		time = TimerGetTime(&timer)/1000.0F; // time in sec
		timeStartInitAcquisition=time;
		// taskState= DONE;
		break;

	case 'U':
		// save data from AMTI
		char filename[20];
		// save data from AMTI (whole buffer)
		sprintf(filename,"AMTIdata%03d.txt",gTrial+1);
		bufferAMTI.SaveTable(filename);
		/*// save data from AMTI (whole buffer)
		sprintf(filename,"bAMTIdata%03d.txt",gTrial);
		SaveRefTrigData(filename);*/
		
		
		/*
/////////////
		sprintf(filename,"TSdata%03d.txt",gTrial);
		FILE *f;
		int r,c;
		int nbRow=maxSamplesAMTI;
		int nbCol = 3;
		// open text file 
		f = fopen (filename, "wt");

		fprintf(f,"%d %d\n",nbRow,nbCol);
		for(r=0;r<nbRow;r++) 
		{
			for(c=0;c<nbCol;c++)
				fprintf (f, "%f ", matTimeStamp[r][c]);
			fprintf(f,"\n");
		}
		fclose(f);
		
//////////
		*/
		resetBuffers();	
		break;
	case 'P':
		SetTrialParameter(PreviousTrial());
		taskState=TRIAL_START;
		break;
	case 'N':
		if (taskState== TASK_CREATED)
		{
			SetTrialParameter(gTrial);
			taskState=TRIAL_START;
		}
		else if (taskState== DONE){
			SetTrialParameter(NextTrial());
			taskState=TRIAL_START;
		}
		else printf("\tWait until the acquisition is done or stop it\n");
		break;
		case 'G': // go to a specific trial
			int i;
			printf("\tEnter the trial number: ");
			scanf("%d",&i);
			// next trial
			if((0< i ) && (i<=maxNumberTrials)) { 
				gTrial=i-1;
				SetTrialParameter(gTrial);
				taskState=TRIAL_START;
			} 
			else {
				printf("Trial number is not valid\n");
			}	
			break;
	case 'M':
		AMTIsetting();
		break;

	case 'V':
		printf("choose the graph type:\n");
		printf("0 for wave plot (default)\n");
		printf("1 for bar plot \n");
		cin >> graphType;
		if (graphType==0 || graphType==1)
			printf("graph type is set correctly \n");
		else 
			printf("Error: graph type value must be 0 or 1 \n");
		break;

	case 'K':
		eventOptions();
	break;
	case ESC:
		taskState=DONE;
		timeKillEvent(timerID);
		glutLeaveMainLoop();
		// wait a while
		Time::delay(0.25);// delay(1);
		platform1.close();
		platform2.close();
		stopTask();
		closeTask();
		break;
	}
}
void WindowKeyboard(unsigned char key, int x, int y){
	KeyBoard(key);
}
void eventOptions() {
	    printf("Task management: \n================= \n");
		printf("S - start recording \n");
		printf("O - stop recording without saving data\n");
		printf("B - stop recording and save data automatically\n");
		printf("U - save data manually\n");
		printf("P - previous trial\n");
		printf("N - next trial\n");
		printf("G - go to a specific trial\n");
		printf("M - AMTI setting\n");
		printf("V - visualization setting\n");
		printf("K - keyboard events\n");
		printf("ESC - exit program\n");
}
void AMTIsetting() {
	    printf("EAMTI setting: \n============== \n");
	
		printf("Type K to return to the main list\n");

}
void WindowReshape( int w, int h ){
	if(taskState!=TASK_NOT_READY){
		if(graphType==0){
			pw->Reshape(w,h);
		}
		else
			pb->Reshape(w,h);
	}
	glutSetWindow(_MainWindow);
	glutPostRedisplay();
}
void WindowReshapeAMTI( int w, int h ){
	if(taskState!=TASK_NOT_READY){
		    pbAMTI->Reshape(w,h);
	}
	glutSetWindow(_MainWindowSubject);
	glutPostRedisplay();
}

void WindowDisplay( void ){ //Update plot
	//If cond (and plot) have not been created do nothing
	if(taskState==TASK_NOT_READY)
		return;
	
	if(graphType==0){
		float barValues [6];
		//Update plot
		float *currentSamp=getDataset();
		// pw->Plot((int)currentSamp);
		pw->Plot((int)*currentSamp);
		// pw->Plot(getCurrentSample());
		switch(taskState){
		case TRIAL_START:
			pbAMTI->Display();
			if (gTrial == maxNumberTrials) 
				pbAMTI->DisplayText("End of experiment",-0.0f,0.0f);
			else
				pbAMTI->DisplayTrialInstruction(gTrial,trialMode);
			break;
		case ACQUIRING:
			{
				if (trigState==recording){
					double time;
					time = TimerGetTime(&timer)/1000.0F; // time in sec
					char timing[30];
					pbAMTI->Display();
					sprintf(timing,"Time [sec] %f.",time-timeStartInitAcquisition);
					pbAMTI->addText("Acquiring",-0.3f,0.9f,WhiteColorArray);
					pbAMTI->DisplayText(timing,-0.3f,0.5f);
				}
			}
			break;
		case DONE:
			for(int i=0; i<5; i++){
			barValues[i]= 0.0f;
			}
			pbAMTI->SetLevels(barValues);
			pbAMTI->Display();
			pbAMTI->DisplayText("Done",-0.3f,0.9f);
			// Sleep(1000);
			// printf("Press enter to continue\n");
			// _getch();
			break;
		case TASK_CREATED:
			pbAMTI->Display();
			pbAMTI->addText("Please read the task instruction carefully",-0.6f,0.5f,WhiteColorArray);			
			pbAMTI->DisplayText("Human - Robot Interaction Experiment",-0.3f,0.9f);	
			break;
		}
	}
	else{//BarPlot
		// currentSamp=new float[getCurrentSample()];
		float *currentSamp=getDataset();
		pb->SetLevels(currentSamp);
		pb->Display();			
	}
		
}

void WindowMouseMotionCB(int x, int y) {
	if(taskState!=TASK_NOT_READY)
		if(graphType==0)
			pw->MouseMotion(x, y);
}

void WindowMouseCB(int button, int state, int x, int y){
	if(taskState!=TASK_NOT_READY)
		if(graphType==0)
			pw->Mouse(button, state, x, y); 
}
//

void Idle(void) {
	char ch=' ';
    
	//Read keyboard if it has been hit
	if(_kbhit()) { 
		ch=toupper(_getch());
		KeyBoard(ch);
		ch=' ';
	}

	// Wait for manual or automatic stop signal 
	// unsigned long done;
	// float ** data; 
	//isTaskDone(&done);	
	switch(taskState){
	case ACQUIRING:
			if (trigState==trigDisable) { // (taskState==DONE) 

				    taskState=DONE;
					stopTask();
					printf("end of recording\n");

					// save data from DAQ
					char filename[20];
					
					// save data from AMTI (whole buffer)
					sprintf(filename,"AMTIdata%03d.txt",gTrial+1);
					bufferAMTI.SaveTable(filename);
					/*
					// save data from AMTI (whole buffer)
					sprintf(filename,"bAMTIdata%03d.txt",gTrial);
					SaveRefTrigData(filename);*/
								
/////////////
					sprintf(filename,"TSdata%03d.txt",gTrial+1);
					FILE *f;
					int r,c;
					int nbRow=maxSamplesAMTI;
					int nbCol = 3;
					/* open text file */
					f = fopen (filename, "wt");

					fprintf(f,"%d %d\n",nbRow,nbCol);
					for(r=0;r<nbRow;r++) 
					{
						for(c=0;c<nbCol;c++)
							fprintf (f, "%f ", matTimeStamp[r][c]);
						fprintf(f,"\n");
					}
					fclose(f);
					//trial++; 
//////////
					resetBuffers();
					// set next trial
					gTrial=NextTrial();
					SetTrialParameter(gTrial);
					taskState=TRIAL_START;
					break; 				
			}

			break;
		}
		
	// update windows function
	if(taskState!=TASK_NOT_READY){
		if(graphType==0){
			pw->Redisplay();
			//pbAMTI->Redisplay();
		}
		else
			pb->Redisplay();
	}
	else{	
		glutSetWindow(_MainWindow);	
		glutPostRedisplay();
		//glutSetWindow(_MainWindowSubject);	
		//glutPostRedisplay();
	}

	//Sleep(1);
}

void InitPlotWave(){
	/*
	float **initData;
	// create a dynamic matrix for the wave plot
	int nbRow_pw;
	int nbCol_pw;
	nbRow_pw = maxSamplesAMTI; // [maxSamplesAMTI][numChann]
	nbCol_pw = numChann;
	initData=(float **)malloc(nbRow_pw*sizeof(float*));
	for(int r=0;r<nbRow_pw;r++) {
		initData[r]=(float *)malloc(nbCol_pw*sizeof(float));
		for(int c=0;c<nbCol_pw;c++) initData[r][c]=0.0;
	}
	// currentSamp=new float[getData()];
	float initData[maxSamplesAMTI][numChann];
	for(int i=0; i<numChann; i++){
		for(int j=0; j<maxSamplesAMTI; j++){
			initData[j][i]=0.0f;
		}
	}
	*/
	float *initSampe=0;
	pw=new PlotWave(&initSampe,PW_2D_BYSAMPLE,maxSamplesAMTI,numChann,0); // PW_2D_NC_BYSAMPLE
	pw->CreatePlot(_MainWindow);
	glutSetWindow(pw->GetMainWindowId());
	
	float *_min = new float[numChann];
	float *_max=new float[numChann];
	float *step=new float[numChann];
	char **labels=new char*[numChann];
	for (int i=0; i<numChann; i++){
		// signal conditioner 1
		if (i<=2){ // force limits
		_min[i]=-5.0f;
		_max[i]=5.0f;
		}
		else if ((i<=5)&&(i>2)) { // torque limits 
		_min[i]=-50.0f;
		_max[i]=50.0f;
		}
		else if (i==6) { // pulse limits
		_min[i]=-0.0f;
		_max[i]= 5.0f;
		}
		// signal conditioner 2
		if ((i<=9)&&(i>6)){ // force limits
		_min[i]=-5.0f;
		_max[i]=5.0f;
		}
		else if ((i<=12)&&(i>9)) { // torque limits 
		_min[i]=-50.0f;
		_max[i]=50.0f;
		}
		else if (i==13) { // pulse limits
		_min[i]=-0.0f;
		_max[i]= 5.0f;
		}
		// step[i]=(_max[i]-_min[i])/5.0f;
		labels[i]=new char[MAX_LABEL_SIZE];
	}
	// pw->SetSize(1.5F,0.1F,1.25F);
	pw->SetChannels(_min,_max, labels);
	pw->Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	// pw->Plot(0);
}
bool InitPlotBarsAMTI(){
	const int numChan = 6; // =getChanN();
	pbAMTI=new BarPlot(numChan);
	pbAMTI->SetWindow(_MainWindowSubject);

	float *_min = new float[numChan];
	float *_max=new float[numChan];
	float *step=new float[numChan];
	char **labels=new char*[numChan];
	for (int i=0; i<numChan; i++)
		labels[i]=new char[MAX_LABEL_SIZE];

	for (int i=0; i<numChan; i++){
		_min[i]=0.0f;
		_max[i]=5.0f;
		step[i]=(_max[i]-_min[i])/5.0f;
	}
	sprintf_s(labels[0], MAX_LABEL_SIZE*sizeof(char), "T1_x");
	sprintf_s(labels[1], MAX_LABEL_SIZE*sizeof(char), "T1_y");
	sprintf_s(labels[2], MAX_LABEL_SIZE*sizeof(char), "T1_z");
	sprintf_s(labels[3], MAX_LABEL_SIZE*sizeof(char), "T2_x");
	sprintf_s(labels[4], MAX_LABEL_SIZE*sizeof(char), "T2_y");
	sprintf_s(labels[5], MAX_LABEL_SIZE*sizeof(char), "T2_z");

	pbAMTI->SetSize(1.5F,0.1F,1.25F);
	pbAMTI->SetLimits(_min,_max, step);
	pbAMTI->SetLabels((const char **)labels, 11);


	pbAMTI->CreateMenu();


	float initValues[numChan];
	for(int i=0; i<numChan; i++)
		initValues[i]=0.0f;
	pbAMTI->SetLevels(initValues);
	pbAMTI->Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	pbAMTI->Display();

	for (int i=0; i<numChan; i++)
		delete[] labels[i];

	delete[] _min; delete[] _max; delete[] labels;
	return true;
}
bool InitPlotBars(){
	// int numChann=7;
	pb=new BarPlot(numChann);
	pb->SetWindow(_MainWindow);

	float *_min = new float[numChann];
	float *_max=new float[numChann];
	float *step=new float[numChann];
	char **labels=new char*[numChann];
	for (int i=0; i<numChann; i++)
		labels[i]=new char[MAX_LABEL_SIZE];

	for (int i=0; i<numChann; i++){
		_min[i]=0.0f;
		_max[i]= 100;// getMaxVolt();
		step[i]=(_max[i]-_min[i])/5.0f;

		sprintf_s(labels[i], MAX_LABEL_SIZE*sizeof(char), "Ch%d\0",i);
	}
	
	pb->SetSize(1.5F,0.1F,1.25F);
	pb->SetLimits(_min,_max, step);
	pb->SetLabels((const char **)labels, 11);

	pb->CreateMenu();

	float currSamp[numChann];
	for(int i=0; i<numChann; i++)
		currSamp[i]=0.0f;
	pb->SetLevels(currSamp);
	pb->Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	pb->Display();

	for (int i=0; i<numChann; i++)
		delete[] labels[i];

	delete[] _min; delete[] _max; delete[] labels;
	return true;
}
void closePlotWave(){
			taskState=TASK_NOT_READY;
			if(graphType==0){
				delete pw;
				delete pbAMTI;
			}
			else
				delete pb;
			// delete currentSamp;
			glutSetWindow(_MainWindow);
			glutHideWindow();
			glutSetWindow(_MainWindowSubject);
			glutHideWindow();
}

//LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
//LARGE_INTEGER Frequency;

long int unix_timestamp()  
{
    time_t t = std::time(0);
    long int now = static_cast<long int> (t);
    return now;
}

void CALLBACK callback(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{	
	////////////////// windowns time test /////////////

	SYSTEMTIME st;
	float currentTimeStamp; 
	//float currentTimeStampUnix; 
	time_t timeUnix;
	//////////////////////////////////////////////////
	// Activity to be timed
	double timerTime;
	//float AMTIframe[16];
	bool ok; 
	Vector vec;

	int NumberOfSensors=getNumSensor();

	switch(taskState){
	case ACQUIRING: {
			long ret=isDataAvailable();
			if ((ret!=0)&&(indxB<finiteMaxSamples)){
					// currentSamp=new float[getData()];
				    float *currentSamp=getDataset();

					// buffer consists of 16 datasets for each conditioner 
				    //(each dataseta is || Data counter Fx Fy Fz Mx My Mz Trigger state || Data counter Fx Fy Fz Mx My Mz Trigger state) || for device 1, 2
					for(int i = 0;i < 16;i++){

						timerTime = TimerGetTime(&timer)/1000.0F; // timer in sec.millisec
						GetSystemTime(&st);                       // time stamp 
						time(&timeUnix);                          // UNIX time stamp 
						long int now = unix_timestamp();          // UNIX time stamp 2nd method

						long int currTS= (st.wHour*100+st.wMinute);
						currTS = (currTS*100+st.wSecond);
						currTS = (currTS*1000+st.wMilliseconds);
						// currentTimeStampUnix = (float)(timeUnix * (time_t)1000 +  (time_t)st.wMilliseconds);
						time_t currTSU=timeUnix*1000 + st.wMilliseconds;
						
						currentTimeStamp = (float) currTS ;

						if (trigState==recording){
							for(int j = 0; j<8*NumberOfSensors; j++)
							{
								//AMTIframe[j]=currentSamp[j];
								bufferAMTI.Set(indxB, j, currentSamp[j]); // AMTIframe is a vector, bufferAMTI is a matrix 
							}
												
							bufferAMTI.Set(indxB, 8*NumberOfSensors, (float)timerTime);         // ElapsedMicroseconds.QuadPart
							matTimeStamp[indxB][0]=(double) currentSamp[0];
							matTimeStamp[indxB][1]=(double) currTSU;
							matTimeStamp[indxB][2]=(double) currTS;						
						}

						// currentTimeStampUnix = 0.0f;
						// sprintf(currentTime,"%02d%02d%02d%03d", st.wHour, st.wMinute, st.wSecond , st.wMilliseconds);

						//printf("Time stamp unix before: %8.4f\n",  currentTimeStampUnix);
						// bufferAMTI.Set(indxB, 8*NumberOfSensors+1, currentTimeStampUnix);        // UNIX time stampe
						//printf("Time stamp unix after: %8.4f\n",  bufferAMTI.Get(indxB, 8*NumberOfSensors+1));

						//printf("Time wind before: %d\n",  currTS);
						//bufferAMTI.Set(indxB, 8*NumberOfSensors+2, currentTimeStamp); 
						//printf("Time stamp after: %8.4f\n",  bufferAMTI.Get(indxB, 8*NumberOfSensors+2));
						


						for (int k = 0; k<NumberOfSensors; k++){
							for(int j = 1; j<7; j++){
								// AMTIdataToYarp.setData(k, j-1, currentSamp[j + k * 8]);
								// prepare a matrix
								YarpMat[k][j-1] = currentSamp[j + k * 8];
							}		
						}
							
						// send the vectors to yarp
						ok=platform1.isOpen();
						if (ok) {
							vec = YarpMat.getRow (0);
							platform1.write(vec);
							ok=platform1.isWriting();
							if (ok) printf("Sent vector number: %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f \r\n", vec[0], vec[1], vec[2], vec[3], vec[4], vec[5]);
						}

						ok=platform2.isOpen();
						if (ok) {
							vec = YarpMat.getRow (1);
							platform2.write(vec);
							ok=platform2.isWriting();
							if (ok) printf("Sent vector number: %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f \r\n", vec[0], vec[1], vec[2], vec[3], vec[4], vec[5]);
						}

						/*
						vec = YarpMat.getRow (1);
						ok = platform2.write(vec);
						if (ok) printf("Sent vector number: %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f \r\n", vec[0], vec[1], vec[2], vec[3], vec[4], vec[5]);
						
						*/
						// AMTIdataToYarp.newFrame();
						
						printf("%6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f\r\n", currentSamp[0], currentSamp[1], currentSamp[2], currentSamp[3], currentSamp[4], currentSamp[5], currentSamp[6], currentSamp[7],currentSamp[8], currentSamp[9], currentSamp[10], currentSamp[11], currentSamp[12], currentSamp[13], currentSamp[14], currentSamp[15]);
						//printf("Current time: %s\n",  currentTime);						
						indxB++;
						currentSamp += 8*NumberOfSensors; //16;
						
					    /*
						if (trigState==recording){
							for(int i = 0; i<8*NumberOfSensors; i++)
							{
								dataAMTI.Set(iEnco, i, currentSamp[i]); // AMTIframe is a vector, bufferAMTI is a matrix	
							}
							dataAMTI.Set(iEnco, 8*NumberOfSensors, (float)timerTime);
							//dataAMTI.Set(iEnco, 8*NumberOfSensors+1, currentTimeStampUnix);
							//dataAMTI.Set(iEnco, 8*NumberOfSensors+2, currentTimeStamp);
							iEnco++;	
						}*/
						Sleep(1);
					}

			break;
			}
			else if (indxB==finiteMaxSamples) trigState= trigDisable;
		}
					
	}
}
int main(int argc, char* argv[]){
	TimerInit(&timer);
	TimerReset(&timer);

	// Initialize dll of AMTI
	// create task
	creatTask();
	taskState= TASK_CREATED;
	// set configuration 
	setDeviceConfiguration(CONFIG_FILE_AMTI);
	//setAcquisitionConfiguration(CONFIG_FILE_ACQ)
	// read acquisition configuration file
	configAcq.ReadTable(CONFIG_FILE_ACQ);
	int row=0;
	maxSamplesAMTI			 = (const int) configAcq.Get(row++,0);
	numSamples				 = (const int) configAcq.Get(row++,0);
	preTrigSamples           = (const int) configAcq.Get(row++,0);
	finiteMaxSamples         = (const int) configAcq.Get(row++,0);
	// numChann				 = (const int) configAcq.Get(row++,0);
	// create a dynamic matrix for the time stamps
	int nbRow;
	int nbCol_TS;
	nbRow = maxSamplesAMTI;
	nbCol_TS = 3;
	matTimeStamp=(double **)malloc(nbRow*sizeof(double*));
	for(int r=0;r<nbRow;r++) {
		matTimeStamp[r]=(double *)malloc(nbCol_TS*sizeof(double));
		for(int c=0;c<nbCol_TS;c++) matTimeStamp[r][c]=0.0;
	}

	// read design file
	printf("Read Design File\n");
	design.ReadTable(DESIGN_FILE);

	// Initialize data storage
	int NumberOfSensors=getNumSensor();
	int nbCol = 8*NumberOfSensors + 1; // Data counter Fx Fy Fz Mx My Mz Trigger state, op timer Unix timestamp time stamp
	bufferAMTI.MakeTable(nbRow, nbCol);
	// dataAMTI.MakeTable(numSamples, nbCol);
	//int nChannels=6;    // Fx Fy Fz Mx My Mz
	//AMTIdataToYarp.init(nbRow, NumberOfSensors, nChannels);

	// Initialize GLUT
	glutInit(&argc, argv);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION); // freeglut EXTENSION
	// Main Graphical Window
	glutInitWindowPosition(400, 0);
	glutInitWindowSize(400, 400);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	_MainWindow=glutCreateWindow( "Analog Input Plot" );
	glutHideWindow();

	// Register Other Callbaacks
	glutDisplayFunc(WindowDisplay);
	glutReshapeFunc(WindowReshape);
	glutKeyboardFunc(WindowKeyboard);
	glutMotionFunc(WindowMouseMotionCB);
	GLUI_Master.set_glutMouseFunc(WindowMouseCB);
	// Register Idle Function
	GLUI_Master.set_glutIdleFunc(Idle);
	//GLUI_Master.set_glutReshapeFunc(WindowReshape);
	
	/*
	// second window
	_MainWindowSubject=glutCreateWindow( "Partecipant feedback" );
	glutHideWindow();
	// Register Other Callbaacks
	glutDisplayFunc(WindowDisplay);
	glutReshapeFunc(WindowReshapeAMTI);
	// Register Idle Function
	GLUI_Master.set_glutIdleFunc(Idle);
	//GLUI_Master.set_glutReshapeFunc(WindowReshape);
	*/
	
	// Set up YARP
	// Network::init();
	Network yarp;

	// PortablePair<Bottle,Vector> pp;
	platform1.open("/sensorAMTI1");     // Give it a name on the network.
	platform2.open("/sensorAMTI2"); 
		
	// Initialize golbal variables
	graphType=0;
	indxB=0;
    // trial=0;
	// Initialize glut
	if(graphType==0){
		InitPlotWave();
		InitPlotBarsAMTI();
	}
	else
		InitPlotBars();

	glutSetWindow(_MainWindow);
	glutShowWindow();
	glClear( GL_COLOR_BUFFER_BIT );

	/*
	glutSetWindow(_MainWindowSubject);
	glutShowWindow();
	glClear( GL_COLOR_BUFFER_BIT );
	*/
	printf("Start Experiment\n");

	Sleep(100);

	// design
	//printf("Type the desired max number of trials \n");
	//cin >> maxNumberTrials;
	maxNumberTrials=design.GetRow();

	// float samplingRate = 1000.0;
	float samplingRate = getSamplingRate();
	int period= (int) (1000.0f/samplingRate); // in ms 
    // record timer event
    timerID=timeSetEvent(period, 0, callback,NULL, TIME_PERIODIC|TIME_KILL_SYNCHRONOUS);
    if(timerID==NULL){
		printf("Unable to set callback function\n");
		return 1;
    }
				
	// start event processing engine
	eventOptions();
	KeyBoard(' ') ;


	glutMainLoop();
	printf("Acquisition done\n");
	printf("Press any key to exit....\n");
	_getch();

}