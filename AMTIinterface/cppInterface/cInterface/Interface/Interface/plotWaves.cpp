/*
C++ routine plotting time series with FREEGLUT/GLUI/OPENGL

Preprocessor definitions:
	GLUT_BUILDING_LIB;GLUI_FREEGLUT;_CRT_SECURE_NO_WARNINGS
Libraries:
	opengl32.lib freeglut.lib glu32.lib glui32d.lib
*/

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <float.h>  // FLT_MIN, FLT_MAX
#include <math.h>
#include <GL/freeglut.h>
#include "plotWaves.h"

// static members
PlotWave *PlotWave::ObjectPtr;


/*
	The world coordinate system are
		[0,SampleRange,yLimMin,yLimMax]
	The viewport coordinate system are
		[0,1,0,1]
*/

// convert from workd to viewport
#define W2PX(x) ((x)/(DATATYPE)_sampleRange )
#define W2PY(y) (((y)-yMin)/(yMax-yMin))

/*
	The screen coordinate system is [0,1,0,1] (defined in the reshape)
	The plotting area coordinates inside the screen are
		[pxMin,pxMax,pyMin,pyMax].
	The plotting area is divided in numChannels viewports.
	The viewport coordinates inside the screen are
		[pxMin,pXMax,ch*(pxMax-pxMin)/numChannels+pyMin,(ch+1)*(pxMax-pxMin)/_numChannels+pyMin]
	where ch=0,,numChannels-1 is the channel number
*/
// convert coordinates from viewport to screen 
#define P2SX(x) ((_pxMax-_pxMin)*(x)+_pxMin)

#define P2SY(y,ch) ((ch+(y))*((_pyMax-_pyMin)/(DATATYPE)_numChannels) - 2*(y)*_fontHeight + _pyMin )


// global conversion
#define convertX(x)   P2SX(W2PX(x)) 
#define convertY(y,i) P2SY(W2PY(y),i) 

//#define DEBUG 1

/* 
	Misc functions
*/

//This function renders a character string onto the current position 
void drawString (char *s){
  unsigned int i;
  for (i = 0; i < strlen(s); i++)
    glutBitmapCharacter (GLUT_BITMAP_HELVETICA_10, s[i]);
};

int fontHeight(){
	return glutBitmapHeight(GLUT_BITMAP_HELVETICA_10);
}


/*

The PlotWave class does not contain a copy of the data: 
The address of the array containing the data, the dimension(s)
of the array and its memory layout must be passed at creation
time.

*/
PlotWave::PlotWave(){
	PlotWave::ObjectPtr=this;

//	_include0=0; // include 0;

	// define plotting area (in screen coordinate system)
	_pxMin=.15f;
	_pxMax=.9f;
	_pyMin=.05f;
	_pyMax=1.0f;

	_sampleRange = _maxSamples;
	_sampleOffset= 0;

	_mouseWx=-999;
	_mouseWy=-999;

	// 
	_gluiWindowId=0;
}

PlotWave::PlotWave(void *dataPtr, int memoryLayout, 
				   int maxSamples, int maxChannels, int maxGroups){

	SetData(dataPtr, memoryLayout, maxSamples,maxChannels,maxGroups);

	SetChannels();

	PlotWave::ObjectPtr=this;

//	_include0=0; // include 0;

	// define plotting area (in screen coordinate system)
	_pxMin=.15f;
	_pxMax=.9f;
	_pyMin=.05f;
	_pyMax=1.0f;

	_sampleRange = _maxSamples;
	_sampleOffset= 0;

	_mouseWx=-999;
	_mouseWy=-999;

	// 
	_gluiWindowId=0;
}

/*


The MemoryLayout variable indicates either the number of dimension
of the data array and how it is organized. Note that multi-dimensional 
arrays with contiguous memory layout are typically declared in the main
program as

DATATYPE data[MAXSAMPLES][MAXCHANNELS];

while data array with non-contiguous memory layout are

DATATYPE data**=calloc(MAXSAMPLES,sizeof(DATATYPE *));
for(i=0;i<MAXSAMPLES;i++)data[i]=calloc(MAXCHANNELS,sizeof(DATATYPE));

One dimensional array
PW_1D						data[sample]
Two-dimensiona array, contiguous memory layout
PW_2D_BYCHANNEL				data[channel*maxSamples+sample];
PW_2D_BYSAMPLE				data[sample*maxChannels+channel];
Two-dimensiona array, non-contiguous memory layout
PW_2D_NC_BYCHANNEL			data[channel][sample]
PW_2D_NC_BYSAMPLE			data[sample][channel]
Three-dimensiona array, contiguous memory layout
PW_2D_BYGROUP				data[(group*maxChannels+channel)*maxSamples+sample]
Two-dimensiona array, non-contiguous memory layout
PW_2D_NC_BYGROUP			data[group][sample][channel]

Implementation details: 

The SetData method uses the MemoryLayout information to set a pointer (GetData) 
toward the correct access function (see PlotWave.h). The pointer is used in the 
Plot method to access to the data. Note that access method for contiguous memory 
layout uses the syntax
	data[sample*maxChanels+channel] 
rather than 
	data[sample][channel] 
because dimensions are not known at compile time.

*/
void PlotWave::SetData(void *dataPtr, int memoryLayout, int maxSamples, int maxChannels=0, int maxGroups=0){
	//
	_maxSamples=maxSamples;
	// save data pointer
	_dataPtr = dataPtr;
	// select access function
	switch(memoryLayout) {
	case PW_1D:
		GetData = &PlotWave::GetData1D;
		_maxChannels=1;
		_maxSensors=1;
		break;
	case PW_2D_BYSAMPLE:
		GetData = &PlotWave::GetData2DBySample;
		_maxChannels=maxChannels;
		_maxSensors=1;
		break;
	case PW_2D_BYCHANNEL:
		GetData = &PlotWave::GetData2DByChannel;
		_maxChannels=maxChannels;
		_maxSensors=1;
		break;
	case PW_3D_BYGROUPSAMPLE:
		GetData = &PlotWave::GetData3DByGroupSample;
		_maxChannels=maxChannels;
		_maxSensors=maxGroups;
		break;
	case PW_2D_NC_BYSAMPLE:
		GetData = &PlotWave::GetData2DNCBySample;
		_maxChannels=maxChannels;
		_maxSensors=1;
		break;
	case PW_2D_NC_BYCHANNEL:
		GetData = &PlotWave::GetData2DNCByChannel;
		_maxChannels=maxChannels;
		_maxSensors=1;
		break;
	case PW_3D_NC_BYGROUPSAMPLE:
		GetData = &PlotWave::GetData3DNCByGroupSample;
		_maxChannels=maxChannels;
		_maxSensors=maxGroups;
		break;
	}
	_sampleRange = _maxSamples;
	_sampleOffset= 0;
	_mouseWx=-999;
	_mouseWy=-999;
}

//Set channels. 
// By default, all channels are plotted and y axis limits are 
// set automatically.
void PlotWave::SetChannels(float *yMin, float *yMax, char **channelNames){
	for(int ch=0;ch<_maxChannels;ch++) {
		// set all channels visible
		_channelList[ch]=true;
		// set limits
		if(yMin==NULL || yMax==NULL) {
			_yMin[ch]=0.0;
			_yMax[ch]=1.0;
			_yAuto[ch]=true;
		} else {
			_yMin[ch]=yMin[ch];
			_yMax[ch]=yMax[ch];
			_yAuto[ch]=false;
		}
		// initialize _channelNames
		if(channelNames==NULL){
			_channelNames[ch]=(char *)calloc(10,sizeof(char));
			sprintf(_channelNames[ch],"Channel %d",ch);
		} else {
			_channelNames[ch]=(char *)calloc(strlen(channelNames[ch]),sizeof(char));
			strcpy(_channelNames[ch],channelNames[ch]);
		}
	}
	_numChannels=_maxChannels;
}

void PlotWave::SetSensors(){
	for(int sensor=0;sensor<_maxSensors;sensor++) {
		// set all channels visible
		_sensorList[sensor]=true;
	}
	_numSensors=_maxSensors;
}


// set world coordinate to [0,0,1,1]
void PlotWave::Reshape(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluOrtho2D(0, 1, 0, 1);
	glMatrixMode(GL_MODELVIEW);

	_fontHeight = (float) fontHeight()/h;

}



int PlotWave::CreatePlot(int x, int y, int w, int h,char *plotName){
	glutInitWindowPosition(x,y);
	glutInitWindowSize(w,h);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	_mainWindowId = glutCreateWindow(plotName);
	_fontHeight = (float) fontHeight()/h;

	//
	if(_channelNames==NULL){
		for(int ch=0;ch<_maxChannels;ch++) {
			_channelNames[ch]=(char *)calloc(10,sizeof(char));
			sprintf(_channelNames[ch],"Channel %d",ch);
		}
	}
	// Create Menu
	CreateMenu();
	return _mainWindowId;
}

int PlotWave::CreatePlot(int mainWindowId){
	_mainWindowId = mainWindowId;
	_fontHeight = (float) fontHeight()/400.0f;

	//
	if(_channelNames==NULL){
		for(int ch=0;ch<_maxChannels;ch++) {
			_channelNames[ch]=(char *)calloc(10,sizeof(char));
			sprintf(_channelNames[ch],"Channel %d",ch);
		}
	}
	// Create Menu
	CreateMenu();
	return _mainWindowId;
}

void PlotWave::Redisplay(){
	glutPostWindowRedisplay(_mainWindowId);
}
	
void PlotWave::PlotBlank(){
	printf("PlotWave::PlotBlank - mainWindowId %d\n",_mainWindowId);
	glutSetWindow(_mainWindowId);
	// clean drawing board
	glClear(GL_COLOR_BUFFER_BIT);
	// swap buffer after drawing
	glutSwapBuffers();
}
	
void PlotWave::Plot(int current)
{
	int sensor;	// select sensor

	DATATYPE yMin, yMax; // minimum and maximum value of the time serie 

	DATATYPE y;
	char string[100];

	int sample, i,ch,k, chCount;
	
	glutSetWindow(_mainWindowId);

	// clean drawing board
	glClear(GL_COLOR_BUFFER_BIT);

	// compute number of division for vertical axis
	int tickNumber = (int) ((_pxMax-_pxMin) / _fontHeight / (float)_numChannels  / 2.0f);
	if(tickNumber>6) tickNumber = 6;
	if(tickNumber<1) tickNumber = 1;



	// loop over each component
	for(ch=0, chCount=0; ch<_maxChannels; ch++) {

		if(!_channelList[ch]) continue; // skip channel if not included in the list
		
		if(_yAuto[ch]){
			// fing minimum and maximum values of the channel
			yMin= FLT_MAX;
			yMax= FLT_MIN;
			for(i=0;i < _sampleRange;i++) {
				sample=current+i-_sampleRange+_sampleOffset;
				if(sample<0) sample=sample+_maxSamples;
				if(sample<0) sample=sample+_maxSamples; 
				if(sample>=_maxSamples) sample=sample-_maxSamples;
				for(sensor=0;sensor<_maxSensors;sensor++) {
					if(!_sensorList[sensor]) continue;
					y = (this->*GetData)(sample,ch,sensor);
					if(y<yMin) yMin=y;
					if(y>yMax) yMax=y;
				}
			}
		} else {
			// manual settings
			yMin=_yMin[ch];
			yMax=_yMax[ch];
		}


		// adjust range of value to include zero
		//if(_include0) {
		//	if(0<yMin) yMin=0;
		//	if(0>yMax) yMax=0;
		//}

		// select color for axes and labels
		glLineWidth(1);
		glColor3f(0.5F,0.5F, 0.5F);
	
		float scale;
		if(fabs(yMax)>fabs(yMin)) scale = fabs(yMax);
		else scale = fabs(yMin);
		scale =  3.0f*(int)(log10(scale)/3.0f); 

		
		// plot y labels
		for(k=0; k<=tickNumber; k++) {
			float y = k*(yMax-yMin)/tickNumber+yMin;
			if(y<1) sprintf_s(string,"%+5.3f",y/pow(10.0f,scale));
			else sprintf_s(string,"%+5.1f",y/pow(10.0f,scale));
			y = convertY(y,chCount);
			glRasterPos2f(.01f,y-0.25f*_fontHeight);
			drawString(string);
			glBegin(GL_LINES);
			glVertex2f(_pxMin-0.01f,y);
			glVertex2f(_pxMin+0.01f,y);
			glEnd();
		}

		// plot channel names
		glRasterPos2f(0.01f,convertY(yMax,chCount)+0.75f*_fontHeight);
		if(scale == 0.0) sprintf_s(string,"%s",_channelNames[ch]);
		else sprintf_s(string,"%s [x10%+.0f]",_channelNames[ch],scale);
		drawString(string);

		// plot horizontal line at the zero value
		if(yMin<= 0 && yMax>=0) {
			glBegin(GL_LINES);
			glVertex2f(convertX(0),convertY(0,chCount));
			glVertex2f(convertX(_sampleRange),convertY(0,chCount));
			glEnd();
		} 

		// plot four vertical bars
		for(k = 0; k<=4; k++) {
			sample = (int) (_sampleRange*k/4.0f);
			glBegin(GL_LINES);
			glVertex2f(convertX(sample),convertY(yMin,chCount));
			glVertex2f(convertX(sample),convertY(yMax,chCount));
			glEnd();
			// plot sample on the x axis of the first plotted channel
			if(chCount==0) {
				sprintf_s(string,"%d",_sampleRange-sample-_sampleOffset);
				glRasterPos2f(convertX(sample), _pyMin-1.0f*_fontHeight);
				drawString(string);
			}
		}

		// plot wave

		glLineWidth (1);
		for(sensor=0;sensor<_maxSensors;sensor++){
			// skip non active sensors
			if(!_sensorList[sensor]) continue;
			// set a different color for each sensor
			switch(sensor){
			case 0:
				glColor3f(0.2F,0.6F, 0.6F);
				break;
			case 1:
				glColor3f(0.6F,0.2F, 0.6F);
				break;
			case 2:
				glColor3f(0.6F,0.6F, 0.2F);
				break;
			default:
				glColor3f(0.5F,0.5F, 0.5F);
				break;
			}
			// plot samples in interval [current-_sampleRange,current-1]
			glBegin(GL_LINE_STRIP);
			for(i = 0; i<_sampleRange; i++) {
				// compute sample
				sample=current+i-_sampleRange+_sampleOffset;
				if(sample<0) sample=sample+_maxSamples;
				if(sample<0) sample=sample+_maxSamples;
				if(sample>=_maxSamples) sample=sample-_maxSamples;
				y= (this->*GetData)(sample,ch,sensor);
// printf("y[%d][%d][%d]=%f (%4.2f,%4.2f)\n",sensor,sample,ch,y,convertX(i),convertY(y,chCount));
				glVertex2f(convertX(i),convertY(y,chCount));
			}
			glEnd();

		}

		chCount++;

	} 
	// swap buffer after drawing
	glutSwapBuffers();
}


/*
	GLUT MENU
*/

// callbacks for default menu entries
void PlotWaveCreateGUI(){
	PlotWave::ObjectPtr->CreateGUI();
}

// menu callback
void PlotWaveMenuCallback(int entry){
	PlotWave::ObjectPtr->_menuCallbacks[entry]();
}


// create menu and attach default entries
void PlotWave::CreateMenu(){
	_numEntry=0;
	glutCreateMenu(PlotWaveMenuCallback);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	AddMenuEntry("Plot Options...",&PlotWaveCreateGUI);
}


void PlotWave::AddMenuEntry(char *text, void (*callback)(void)){
	_menuCallbacks[_numEntry]=callback;
	glutAddMenuEntry(text, _numEntry++);
}


/*
  
  GRAPHICAL USER INTERFACE

  Note: there is a problem when closing GLUI windows containing GLUI checkboxes. It looks
  like a glutSetWindow() toward the destroyed GLUI windows is called when a new one is
  created (iff checkboxes values have been changed!).

*/

void PlotWave::Message(char *text){
//	printf("Message\n");
	if(_gluiWindowId==0){
		printf("%s\n",text); // console output
	} else {
		_status->erase_text();
		_status->set_text(text);
		_status->draw_text();
	}
}



void PlotWaveChannelCheckboxCB(int channel){
//	printf("ChannelCheckboxCB\n");
	if(PlotWave::ObjectPtr->_channelList[channel]) PlotWave::ObjectPtr->_numChannels++;
	else PlotWave::ObjectPtr->_numChannels--;
}

void PlotWaveSensorCheckboxCB(int sensor){
//	printf("SensorCheckboxCB\n");
	if(PlotWave::ObjectPtr->_sensorList[sensor]) PlotWave::ObjectPtr->_numSensors++;
	else PlotWave::ObjectPtr->_numSensors--;
}

void ClosePlotWaveGUI(void){
//	printf("ClosePlotWaveGUI\n");
	free(PlotWave::ObjectPtr->_status);
	//
	PlotWave::ObjectPtr->_glui=NULL;
	PlotWave::ObjectPtr->_gluiWindowId=0;
};


void PlotWave::CreateGUI(){
//	printf("CreateGUI\n");
	char text[100];

	if(_gluiWindowId!=0) {
		Message("UI already exists\n");
		return;
	} 

	// create UI
	_glui = GLUI_Master.create_glui("PlotWave Settings", 0 );
	_glui->set_main_gfx_window(_mainWindowId);
	_gluiWindowId = _glui->get_glut_window_id();

	for(int channel=0;channel<_maxChannels;channel++) {
		sprintf_s(text,"Channel %2d",channel);
		GLUI_Panel *ChannelPanel = _glui->add_panel(text);
//		GLUI_Panel *ChannelPanel = new	GLUI_Panel(_glui,text);
		_glui->add_checkbox_to_panel(ChannelPanel,"",&_channelList[channel],channel,PlotWaveChannelCheckboxCB);
//		new GLUI_Checkbox(ChannelPanel,"",&_channelList[channel],channel,ChannelCheckboxCB );

		_glui->add_column_to_panel(ChannelPanel,false);
		_glui->add_checkbox_to_panel(ChannelPanel,"Auto",&_yAuto[channel]);
		_glui->add_column_to_panel(ChannelPanel,false);
		_glui->add_spinner_to_panel(ChannelPanel, "Min",GLUI_SPINNER_FLOAT,&_yMin[channel]);
		_glui->add_column_to_panel(ChannelPanel,false);
		_glui->add_spinner_to_panel(ChannelPanel, "Max",GLUI_SPINNER_FLOAT,&_yMax[channel]);
	}

	GLUI_Panel *SensorPanel = _glui->add_panel("Sensors");
	for(int sensor=0;sensor<_maxSensors;sensor++){
		sprintf_s(text,"%d",sensor);
		_glui->add_checkbox_to_panel(SensorPanel,text, &_sensorList[sensor],sensor,PlotWaveSensorCheckboxCB);
		_glui->add_column_to_panel(SensorPanel,false);
	}

	// Status Line
	GLUI_Panel *MessagePanel = _glui->add_panel("Status");
	_status = _glui->add_statictext_to_panel(MessagePanel,""); 

	// sync GUI
	_glui->sync_live();

	// retrieve glut window Id and set callabck 
	int w = glutGetWindow();
//	printf("plotWaves::CreateGui - current window %d\n",_gluiWindowId);
	glutSetWindow(_gluiWindowId);
	glutCloseFunc(ClosePlotWaveGUI);			// freeglut extension
//	printf("plotWaves::CreateGui - current window %d\n",w);
	glutSetWindow(w);
}

/*
	 MOUSE
*/

void PlotWave::MouseMotion(int x, int y) {

	GLdouble modelMatrix[16];
	glGetDoublev(GL_MODELVIEW_MATRIX,modelMatrix);
	GLdouble projMatrix[16];
	glGetDoublev(GL_PROJECTION_MATRIX,projMatrix);
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
	double wx,wy,wz;
	gluUnProject(x,y,0,
		 modelMatrix,projMatrix,viewport,
		 &wx,&wy,&wz);
	// vertical Mouse movement correspond to a zoom in or out
	if(_mouseWy!=-999) {
		if(_maxSamples>40) {
			_sampleRange=_sampleRange-(int)floor((wy-_mouseWy)*_sampleRange);
			if(_sampleRange<40) _sampleRange=40;
			if(_sampleRange>_maxSamples) _sampleRange=_maxSamples;
		}
	}
	// adjust _sampleOffset
	if(_mouseWx!=-999) {
			_sampleOffset=_sampleOffset-(int)floor((wx-_mouseWx)*_sampleRange);
			if(_sampleOffset< _sampleRange-_maxSamples) _sampleOffset=_sampleRange-_maxSamples;
			if(_sampleOffset > 0) _sampleOffset = 0;
	}
	_mouseWx=wx;
	_mouseWy=wy;
}

void PlotWave::Mouse(int button, int state, int x, int y) {
	if(button==GLUT_LEFT_BUTTON && state==GLUT_UP) {
		_mouseWx=-999;
		_mouseWy=-999;
	}
}


