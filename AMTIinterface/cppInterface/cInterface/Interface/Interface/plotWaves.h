#ifndef PLOT_WAVES_H
#define PLOT_WAVES_H

#include <GL/glui.h>

/*
	plotWaves.h
*/

#define PW_MAX_MENU_ENTRY		10

#define DATATYPE float

#define PLOTWAVE_MAXCHANNELS	20
#define PLOTWAVE_MAXSENSORS		3

#define PW_1D					0
#define PW_2D_BYCHANNEL			1
#define PW_2D_BYSAMPLE			2
#define PW_3D_BYGROUPSAMPLE		3
#define PW_3D_BYGROUPCHANNEL	4

#define PW_2D_NC_BYCHANNEL		11
#define PW_2D_NC_BYSAMPLE		12
#define PW_3D_NC_BYGROUPSAMPLE	13
#define PW_3D_NC_BYGROUPCHANNEL	14


class PlotWave {
public:
	// Initialization parameters
	PlotWave();
	PlotWave(void *dataPtr,int memoryLayout,int maxSamples, int maxChannels, int maxGroups);
	void SetData(void *dataPtr,int memoryLayout, int maxSamples, int maxChannels, int maxGroups);
	void SetChannels(float *yMin=NULL, float *yMax=NULL, char **channelNames=NULL);
	void SetSensors();
	// Main Graphical Window
	int CreatePlot(int x, int y, int w, int h, char *plotName);
	int PlotWave::CreatePlot(int mainWindowId);
	int GetMainWindowId(void);
	void Plot(int current);
	void PlotBlank();
	void Redisplay();
	void Mouse(int button, int state, int x, int y);
	void Reshape(int w, int h);
	void MouseMotion(int x, int y);
	friend void PlotWaveReshapeCallback(int w, int h);
	// Glut Menu
	void CreateMenu();
	void AddMenuEntry(char *text, void (*callback)());
	void MenuCallBack(int value);
	friend void PlotWaveMenuCallback(int entry);
	// Option Windows
	void CreateGUI();
	friend void ClosePlotWaveGUI(void);
	friend void PlotWaveChannelCheckboxCB(int channel);
	friend void PlotWaveSensorCheckboxCB(int channel);

	static PlotWave *ObjectPtr; // used by (non-member) callback functions
private:
	// Array dimensions
	int _maxSamples;
	int _maxSensors;
	int _maxChannels;
	// Pointer toward data array
	void *_dataPtr;
	// Access function pointer
	DATATYPE (PlotWave::*GetData)(int,int,int); 
	// Access functions
	DATATYPE GetData1D(int sample, int channel,int group);
	DATATYPE GetData2DBySample(int sample, int channel,int group);
	DATATYPE GetData2DByChannel(int sample, int channel,int group);
	DATATYPE GetData3DByGroupSample(int sample, int channel,int group);
	DATATYPE GetData3DByGroupChannel(int sample, int channel,int group);
	DATATYPE GetData2DNCBySample(int sample, int channel,int group);
	DATATYPE GetData2DNCByChannel(int sample, int channel,int group);
	DATATYPE GetData3DNCByGroupSample(int sample, int channel,int group);
	DATATYPE GetData3DNCByGroupChannel(int sample, int channel,int group);

	// Trace info
	int _numSensors;
	int _sensorList[PLOTWAVE_MAXSENSORS];
	int _sensorColor[PLOTWAVE_MAXSENSORS][3]; // RGB
	// Channel parameters
	int _numChannels;
	char *_channelNames[PLOTWAVE_MAXCHANNELS];
	int _channelList[PLOTWAVE_MAXCHANNELS]; 

	// Main window
	int _mainWindowId;
	// User interface
	GLUI *_glui;
	GLUI_StaticText *_status;
	int _gluiWindowId;
	void Message(char *text);
	// Menu
	void (*_menuCallbacks[PW_MAX_MENU_ENTRY])();
	int _numEntry;
	// Mouse 
	double _mouseWx;
	double _mouseWy;

	// Graphical Parameters
	// in window coordinates
	float _fontHeight;
	// define world coordinate
	int   _yAuto[PLOTWAVE_MAXCHANNELS];
	float _yMin[PLOTWAVE_MAXCHANNELS];
	float _yMax[PLOTWAVE_MAXCHANNELS];
	// Horizontal (Time) axis
	int _sampleRange;
	int _sampleOffset;	
	// Sampling rate (used to label x-axis)
	float _Freq;
	// plotting area (margins) in scren coordinate
	float _pxMin;
	float _pxMax;
	float _pyMin;
	float _pyMax;
};  

// Inline functions
inline int PlotWave::GetMainWindowId(void) { 
	return _mainWindowId; 
};

// Access functions for contiguous memory layouts
inline DATATYPE PlotWave::GetData1D(int sample, int channel,int group){ 
	return(((DATATYPE *)_dataPtr)[sample]);
};
inline DATATYPE PlotWave::GetData2DBySample(int sample, int channel,int group) {
	return(((DATATYPE *)_dataPtr)[sample*_maxChannels+channel]);
};
inline DATATYPE PlotWave::GetData2DByChannel(int sample, int channel,int group) {
	return(((DATATYPE *)_dataPtr)[channel*_maxSamples+sample]);
};
inline DATATYPE PlotWave::GetData3DByGroupSample(int sample, int channel,int group) {
	return(((DATATYPE *)_dataPtr)[(group*_maxSamples+sample)*_maxChannels+channel]);
};
inline DATATYPE PlotWave::GetData3DByGroupChannel(int sample, int channel,int group) {
	return(((DATATYPE *)_dataPtr)[(group*_maxChannels+sample)*_maxSamples+channel]);
};

// Access functions for non-contiguous memory layouts
inline DATATYPE PlotWave::GetData2DNCBySample(int sample, int channel,int group) {
	return(((DATATYPE **)_dataPtr)[sample][channel]);
};
inline DATATYPE PlotWave::GetData2DNCByChannel(int sample, int channel,int group) { 
	return(((DATATYPE **)_dataPtr)[channel][sample]);
};
inline DATATYPE PlotWave::GetData3DNCByGroupSample(int sample, int channel,int group) {
	return(((DATATYPE ***)_dataPtr)[group][sample][channel]);
};
inline DATATYPE PlotWave::GetData3DNCByGroupChannel(int sample, int channel,int group) {
	return(((DATATYPE ***)_dataPtr)[group][channel][sample]);
};

#endif // PLOT_WAVES_H
