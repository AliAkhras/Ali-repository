/*
	PlotBar.h
*/

#ifndef PLOT_BAR_H
#define PLOT_BAR_H

#include <GL/glui.h>

#define PB_MAX_MENU_ENTRY		10

//Display one bar
class OneBarPlot {
public:
	OneBarPlot ();
	void setSize(float height, float width);
	void SetLimit(float min, float max, float targetMin, float targetMax);
	void SetLimit(float min, float max, float step = 0);
	void SetTarget(float targetMin, float targetMax);
	int MakeWindow(int x, int y, int w, int h, const char *title);
	void Reshape(int w, int h);
//	void Display(float level);
	void drawConditions(int mode, int answered=0, bool negotiated=false, int pos=0,int rResp=0, int lResp=0);
	void drawBar(void);
	void drawTargets(void);
	void EmptyDisplay(int mode);
	void Display(float level1, float level2);
	void Display2(float level1, float level2);
	void Display3(float level1, float level2, int mode, int answered=0,bool negotiated=false,int pos=0,int rResp=0, int lResp=0);
	void DisplayHoming(int ok);
	//void DisplayWaiting(int mode, bool negotiated);
	void DisplayStarting(float level1, float level2, int mode, bool negotiated);
	void DisplayEnding(float level1, float level2, int mode);
	//void DisplayRespond(float level1, float level2, int mode);
	void DisplayBreak(int trial);
	void DisplayFinished();
	void Redisplay();

private:
	void drawString(char *s);
	void drawStringBig (char *s);
	void drawArrow(float y1, float y2);
	void DrawCircle(float cx, float cy, float r, int num_segments) ;
	int mWindow;

	float mHeight;
	float mWidth;
	float mMin;
	float mMax;
	float mTargetMin; 
	float mTargetMax; 
	float mScaledTargetMin; 
	float mScaledTargetMax; 

	char numTrialsText[50];

};  
  

// display several bars
class BarPlot {
public:
	BarPlot();
	BarPlot(int numBars);
	~BarPlot();
	void SetSize(float height, float width, float spacing = 1);
	int MakeWindow(int x, int y, int w, int h, const char *title);
	void SetWindow(int Window);
	void Reshape(int w, int h);
	void SetLevels(float level[]);
	void Display();
	void DrawBars();
	void SetLimits(float min[], float max[], float tickStep[] = NULL);
	void SetTargets(float targetMin[], float targetMax[] = 0);
	void SetLabels(const char** label, int length);
	void DisplayText(char *Text,float X, float Y);
	void addText(char *Text,float X, float Y, float colorArray[]);
	void DisplayTrialInstruction(int trial,char *mode);
	void Redisplay();
	// Glut Menu
	void CreateMenu();
	void AddMenuEntry(char *text, void (*callback)());
	void MenuCallBack(int value);
	friend void BarPlotMenuCallback(int entry);
	// Option Windows
	void CreateGUI();
	friend void CloseBarPlotGUI(void);
	
	static BarPlot *ObjectPtr; // used by (non-member) callback functions
private:
	void drawString(char *s);
	void drawStringBig (char *s);
	int mWindow;
	int mNumBars;
	float mHeight;
	float mWidth;
	float mSpacing;
	float *mCenterPos;
	float *mMin;
	float *mMax;
	float *mLevel;
	bool mTicks;
	float *mTickStep;
	bool mLabels;
	char **mLabel;
	bool mTargets;
	float *mTargetMin; 
	float *mTargetMax; 

	// User interface
	GLUI *_glui;
	GLUI_StaticText *_status;
	int _gluiWindowId;
	void Message(char *text);
	// Menu
	void (*_menuCallbacks[PB_MAX_MENU_ENTRY])();
	int _numEntry;
	// Mouse 
	double _mouseWx;
	double _mouseWy;
	char trialsText[100];
};

#endif