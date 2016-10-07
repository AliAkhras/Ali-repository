#include <GL/freeglut.h>

#include <stdio.h>
#include <math.h> //sqrt

#include "PlotBar.h"

/*
	OneBarPlot
*/

// object to screen

OneBarPlot::OneBarPlot(){
	mHeight=1.5;
	mWidth=0.5;
	mMin=0.0;
	mMax=1.0;
	mTargetMin=0.5;
	mTargetMax=0.75;
}

void OneBarPlot::SetLimit(float min, float max, float targetMin, float targetMax){
	mMin = min;
	mMax = max;
	mTargetMin = targetMin;
	mTargetMax = targetMax;
	mScaledTargetMin = mTargetMin*mHeight/(mMax-mMin)-mHeight/2;
	mScaledTargetMax = mTargetMax*mHeight/(mMax-mMin)-mHeight/2;
}

void OneBarPlot::setSize(float height, float width){
	mHeight = height;
	mWidth = width;
	mScaledTargetMin = mTargetMin*mHeight/(mMax-mMin)-mHeight/2;
	mScaledTargetMax = mTargetMax*mHeight/(mMax-mMin)-mHeight/2;
}

int OneBarPlot::MakeWindow(int x, int y, int w, int h, const char *title){
	glutInitWindowPosition(x,y);
	glutInitWindowSize(w,h);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	mWindow = glutCreateWindow(title);
	return mWindow;
}

/*

  Callbacks

*/

// set world coordinate to [-1,-1,1,1]
void OneBarPlot::Reshape(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    //gluOrtho2D(-1, 1, -1, 1);
	if(w>h){
		double asp=(double) w/(double) h;
		gluOrtho2D(-asp, asp, -1, 1);
	}
	else{
		double asp=(double) h/(double) w;
		gluOrtho2D(-1, 1,-asp, asp);
	}
	glMatrixMode(GL_MODELVIEW);
}

void OneBarPlot::Redisplay(){
	glutPostWindowRedisplay(mWindow);
}

void OneBarPlot::drawArrow(float y1, float y2) {
	// transform in viewport coordinate system
	float yy1 = (y1-mMin)/(mMax-mMin)*mHeight-mHeight/2.0f;
	float yy2 = (y2-mMin)/(mMax-mMin)*mHeight-mHeight/2.0f;
	// maximum arrow head size
	float len = 0.05f; 
	// scale arrow head
	if(fabs(yy2-yy1)<len) len = fabs(yy2-yy1);
	// arrow body
	glBegin(GL_LINES);
	glVertex2f(0.0, yy1);
	glVertex2f(0.0, yy2);
	glEnd();
	// arrow head
	if(yy2>yy1) {
		glBegin(GL_LINES);
		glVertex2f(0.0, yy2);
		glVertex2f(len, yy2-len);
		glVertex2f(len, yy2-len);
		glVertex2f(-len, yy2-len);
		glVertex2f(-len, yy2-len);
		glVertex2f(0.0, yy2);
		glEnd();
	} else {
		glBegin(GL_LINES);
		glVertex2f(0.0, yy2);
		glVertex2f(len, yy2+len);
		glVertex2f(len, yy2+len);
		glVertex2f(-len, yy2+len);
		glVertex2f(-len, yy2+len);
		glVertex2f(0.0, yy2);
		glEnd();
	}
}
/* This function renders a character string onto the current position */
void OneBarPlot::drawString (char *s){
	unsigned int i;
	for (i = 0; i < strlen (s); i++)
		glutBitmapCharacter (GLUT_BITMAP_HELVETICA_10, s[i]);	
};

/* This function renders a character string using bigger fonts onto the current position */
void OneBarPlot::drawStringBig (char *s){	
	unsigned int i;
	for (i = 0; i < strlen (s); i++)
		//glutBitmapCharacter (GLUT_BITMAP_HELVETICA_18, s[i]);
		glutBitmapCharacter (GLUT_BITMAP_TIMES_ROMAN_24, s[i]); 
		//glutBitmapCharacter (GLUT_BITMAP_9_BY_15 , s[i]);
}

void OneBarPlot::drawBar(void){
	glColor3f(0.5F,0.5F, 0.5F);
	glBegin(GL_POLYGON);
	glVertex2f(-mWidth/2.0f,-mHeight/2.0f);
	glVertex2f( mWidth/2.0f,-mHeight/2.0f);
	glVertex2f( mWidth/2.0f, mHeight/2.0f);
	glVertex2f(-mWidth/2.0f, mHeight/2.0f);
	glEnd();
}

//Taken from http://slabode.exofire.net/circle_draw.shtml
void OneBarPlot::DrawCircle(float cx, float cy, float r, int num_segments) 
{ 
	float theta = 2 * 3.1415926f / float(num_segments); 
	float c = cosf(theta);//precalculate the sine and cosine
	float s = sinf(theta);
	float t;

	float x = r;//we start at angle = 0 
	float y = 0; 
    
	//glBegin(GL_LINE_LOOP); 
	glBegin(GL_POLYGON);
	for(int ii = 0; ii < num_segments; ii++) 
	{ 
		glVertex2f(x + cx, y + cy);//output vertex 
        
		//apply the rotation matrix
		t = x;
		x = c * x - s * y;
		y = s * t + c * y;
	} 
	glEnd(); 
}


void OneBarPlot::drawTargets(){
	// target zone
	glColor3f(0.25F,0.75F, 0.25F);
	if(mScaledTargetMin==mScaledTargetMax){
		glLineWidth(3);
		glBegin(GL_LINES);
		glVertex2f(-mWidth/2,  mHeight/2.0f-mScaledTargetMin);
		glVertex2f( mWidth/2,  mHeight/2.0f-mScaledTargetMin);
		glVertex2f(-mWidth/2, -mHeight/2.0f+mScaledTargetMin);
		glVertex2f( mWidth/2, -mHeight/2.0f+mScaledTargetMin);
		glEnd();
	} else {
		glBegin(GL_POLYGON);
		glVertex2f(-mWidth/2, mHeight/2.0f-mScaledTargetMin);
		glVertex2f( mWidth/2, mHeight/2.0f-mScaledTargetMin);
		glVertex2f( mWidth/2, mHeight/2.0f-mScaledTargetMax);
		glVertex2f(-mWidth/2, mHeight/2.0f-mScaledTargetMax);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(-mWidth/2,-mHeight/2.0f+mScaledTargetMin);
		glVertex2f( mWidth/2,-mHeight/2.0f+mScaledTargetMin);
		glVertex2f( mWidth/2,-mHeight/2.0f+mScaledTargetMax);
		glVertex2f(-mWidth/2,-mHeight/2.0f+mScaledTargetMax);
		glEnd();

	}
	//glColor3f(0.25F,0.75F, 0.25F);
	//if(mScaledTargetMin==mScaledTargetMax){
	//	glLineWidth(3);
	//	glBegin(GL_LINES);
	//	glVertex2f(-mWidth/2, mScaledTargetMin);
	//	glVertex2f( mWidth/2, mScaledTargetMin);
	//	glEnd();
	//} else {
	//	glBegin(GL_POLYGON);
	//	glVertex2f(-mWidth/2, mScaledTargetMin);
	//	glVertex2f( mWidth/2, mScaledTargetMin);
	//	glVertex2f( mWidth/2, mScaledTargetMax);
	//	glVertex2f(-mWidth/2, mScaledTargetMax);
	//	glEnd();
	//}
	// plot two lines
	//glLineWidth(2);
	//glBegin(GL_LINES);
	//glColor3f(1.0F,0.25F, 0.25F);
	//glVertex2f(-mWidth/2,level1);
	//glVertex2f( mWidth/2,level1);
	//glColor3f(.25F,0.25F, 1.F);
	//glVertex2f(-mWidth/2,level2);
	//glVertex2f( mWidth/2,level2);
	//glEnd();
}

void OneBarPlot::drawConditions(int mode, int answered, bool negotiated, int pos,int rResp, int lResp){
	if(answered){
		glColor3f(0.5F,0.5F, 0.5F);
		glRasterPos2f( mWidth/2.0f-0.8f, (mHeight/2.0f+1.0F)/2.0F);
		drawStringBig ("Risposta acquisita, attendere...");
	}
	else{
		switch(mode) {
		case 1:
			glColor3f(0.5F,0.5F, 0.5F);
			glRasterPos2f(-mWidth/2.0f, (mHeight/2.0f+1.0F)/2.0F);			
			drawStringBig ("Prova Statica");			
			break;
		case 2:
			//glColor3f(1.0F,0.5F, 0.5F);
			glColor3f(0.5F,0.5F, 0.5F);
			glRasterPos2f( mWidth/2.0f, (mHeight/2.0f+1.0F)/2.0F);			
			drawStringBig ("Prova Dinamica");			
			break;
		case 3:
			glColor3f(0.5F,0.5F, 0.5F);
			glRasterPos2f( mWidth/2.0f-0.8f, (mHeight/2.0f+1.0F)/2.0F);
			//drawStringBig ("Per Favore, Rispondere `Sinistra' o `Destra'");			
			drawStringBig ("Per Favore, Rispondere `Avanti' o `Indietro'");			
			break;
		}		
	}
				
	switch(pos){
		case 1:
			if(rResp==2)
				glColor3f(1.F,0.25F, 0.25F);  // red
			else
				glColor3f(1.F,1.F, 1.F);  // white
			DrawCircle(0.7f, 0.5f, 0.1f, 20);
			if(rResp==1)
				glColor3f(1.F,0.25F, 0.25F);  // red
			else
				glColor3f(1.F,1.F, 1.F);  // white
			DrawCircle(0.7f, -0.5f, 0.1f, 20);
			if(negotiated){
				if(lResp==2)
					glColor3f(1.F,0.25F, 0.25F);  // red
				else
					glColor3f(1.F,1.F, 1.F);  // white
				DrawCircle(-0.7f, 0.5f, 0.1f, 20);
				if(lResp==1)
					glColor3f(1.F,0.25F, 0.25F);  // red
				else
					glColor3f(1.F,1.F, 1.F);  // white
				DrawCircle(-0.7f, -0.5f, 0.1f, 20);
			}
			break;
		case 2:
			if(negotiated){
				if(rResp==1)
					glColor3f(1.F,0.25F, 0.25F);  // red
				else
					glColor3f(1.F,1.F, 1.F);  // white
				DrawCircle(0.7f, 0.5f, 0.1f, 20);
				if(rResp==2)
					glColor3f(1.F,0.25F, 0.25F);  // red
				else
					glColor3f(1.F,1.F, 1.F);  // white
				DrawCircle(0.7f, -0.5f, 0.1f, 20);
			}
			if(lResp==1)
				glColor3f(1.F,0.25F, 0.25F);  // red
			else
				glColor3f(1.F,1.F, 1.F);  // white
			DrawCircle(-0.7f, 0.5f, 0.1f, 20);
			if(lResp==2)
				glColor3f(1.F,0.25F, 0.25F);  // red
			else
				glColor3f(1.F,1.F, 1.F);  // white
			DrawCircle(-0.7f, -0.5f, 0.1f, 20);
			break;
		}				
}

void OneBarPlot::EmptyDisplay(int mode){
		// clean drawing board
	glClear(GL_COLOR_BUFFER_BIT);
	// condition

	glColor3f(1.0F,1.0F, 1.0F);
	glRasterPos2f(-mWidth/2.0f-0.3f, (mHeight/2.0f-0.5F)/2.0F);
	//drawStringBig ("Premi ` ' per iniziare la prova.");
	drawStringBig ("Attendi il segnale d'inizio");


	// drawBar();

	// swap buffer after drawing
	glutSwapBuffers();

}

void OneBarPlot::DisplayBreak(int trial)
{
	// clean drawing board
	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(1.0F,1.0F, 1.0F);
	glRasterPos2f(-mWidth/2.0f-0.1f, (mHeight/2.0f)/2.0F);
	sprintf(numTrialsText,"Hai completato %d prove.",trial);
	drawStringBig(numTrialsText);
	glRasterPos2f(-mWidth/2.0f-0.1f, (mHeight/2.0f-0.5F)/2.0F);
	drawStringBig("Riposa un attimo");
	/*drawStringBig("Riposa un attimo, e poi");
	glRasterPos2f(-mWidth/2.0f-0.1f, (mHeight/2.0f-1.0F)/2.0F);
	drawStringBig ("premi ` ' per continuare");

	// condition
	switch(mode) {
	case 1:
		glRasterPos2f(-mWidth/2.0f-0.1f, (mHeight/2.0f-1.5F)/2.0F);
		drawStringBig("con le prove STATICHE");
		break;
	case 2:
		glRasterPos2f(-mWidth/2.0f-0.1f, (mHeight/2.0f-1.5F)/2.0F);
		drawStringBig ("con le prove DINAMICHE");
		break;
	}*/

	// swap buffer after drawing
	glutSwapBuffers();
}

void OneBarPlot::DisplayFinished()
{
	// clean drawing board
	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(1.0F,1.0F, 1.0F);
	glRasterPos2f(-mWidth/2.0f-0.1f, (mHeight/2.0f)/2.0F);
	drawStringBig("Congratulazioni -- le prove sono finite per oggi!");

	// swap buffer after drawing
	glutSwapBuffers();
}

void OneBarPlot::DisplayHoming(int ok){
	// clean drawing board
	glClear(GL_COLOR_BUFFER_BIT);	
	
	if(!ok){
		glColor3f(1.0F,0.25F, 0.25F);
		glRasterPos2f(-mWidth/2.0f, (mHeight/2.0f+1.0F)/2.0F);
		drawStringBig ("Posizionare il dispositivo al centro....");
	}
	else{
		glColor3f(0.25F,1.0F, 0.25F);
		glRasterPos2f(-mWidth/2.0f, (mHeight/2.0f+1.0F)/2.0F);
		drawStringBig ("Posizione corretta, attendere...");
	}	

	//  grey bar
    drawBar();

	// swap buffer after drawing
	glutSwapBuffers();
}
/*
void OneBarPlot::DisplayWaiting(int mode, bool negotiated)
{
	// clean drawing board
	glClear(GL_COLOR_BUFFER_BIT);

	// condition
	drawConditions(mode,0,negotiated);
	
	//  grey bar
    drawBar();

	// swap buffer after drawing
	glutSwapBuffers();
}
*/
void OneBarPlot::DisplayStarting(float level1, float level2, int mode, bool negotiated)
{
	// clean drawing board
	glClear(GL_COLOR_BUFFER_BIT);

	// condition
	drawConditions(mode, 0, negotiated);
	
	//  grey bar
	drawBar();

	drawTargets();

	glLineWidth(3);
	glColor3f(1.0F,0.25F, 0.25F); // red (top)
	drawArrow(mMax, mMax-level1);

	glColor3f(.25F,0.25F, 1.F);  // blue 
	drawArrow(mMin, mMin+level2);
	
	glColor3f(1.0F,1.0F,1.0F);
	glRasterPos2f(-mWidth/2.0f-0.1f, (mHeight/2.0f)/2.0F);
	//sprintf(numTrialsText,"State -- Ramping Up.");

	//drawStringBig(numTrialsText);
	// swap buffer after drawing
	glutSwapBuffers();
}


void OneBarPlot::DisplayEnding(float level1, float level2, int mode)
{
	// clean drawing board
	glClear(GL_COLOR_BUFFER_BIT);

	// condition
	drawConditions(mode);
	
	//  grey bar
	drawBar();

	drawTargets();

	glLineWidth(3);
	glColor3f(1.0F,0.25F, 0.25F); // red (top)
	drawArrow(mMax, mMax-level1);

	glColor3f(.25F,0.25F, 1.F);  // blue 
	drawArrow(mMin, mMin+level2);

	glColor3f(1.0F,1.0F,1.0F);
	glRasterPos2f(-mWidth/2.0f-0.1f, (mHeight/2.0f)/2.0F);
	sprintf(numTrialsText,"State -- Ramping Down.");
	drawStringBig(numTrialsText);
	
	// swap buffer after drawing
	glutSwapBuffers();
}

/*
void OneBarPlot::DisplayRespond(float level1, float level2, int mode)
{
	// clean drawing board
	glClear(GL_COLOR_BUFFER_BIT);

	// condition
	drawConditions(mode);
	
	//  grey bar
	drawBar();

	glColor3f(1.0F,1.0F,1.0F);
	glRasterPos2f( mWidth/2.0f-0.8f, (mHeight/2.0f+1.0F)/2.0F);
	//drawStringBig ("Per Favore, Rispondere `Sinistra' o `Destra'");
	sprintf(numTrialsText,"Per Favore, Rispondere `Avanti' o `Indietro'");	
	drawStringBig(numTrialsText);
	
	// swap buffer after drawing
	glutSwapBuffers();
}
*/

// Contact forces on the top and bottom contact surfaces are represented by 
// a downward and upward pointing arrows. Two green rectangles indicate
// the desired force levels. 
void OneBarPlot::Display3(float level1, float level2, int mode, int answered, bool negotiated, int pos,int rResp, int lResp)
{
		// clean drawing board
	glClear(GL_COLOR_BUFFER_BIT);

	// condition
	drawConditions(mode, answered, negotiated, pos,rResp, lResp);
	
	//  grey bar
	drawBar();

	// target zone
	drawTargets();

	glLineWidth(3);
	glColor3f(1.0F,0.25F, 0.25F); // red (top)
	drawArrow(mMax, mMax-level1);

	glColor3f(.25F,0.25F, 1.F);  // blue 
	drawArrow(mMin, mMin+level2);	

	//drawStringBig(numTrialsText);

	// swap buffer after drawing
	glutSwapBuffers();
}

// Contact forces on the top and bottom contact surfaces are represented by 
// a downward and upward pointing arrows. Two green rectangles indicate
// the desired force levels.
void OneBarPlot::Display2(float level1, float level2)
{
	glClear(GL_COLOR_BUFFER_BIT);

	//  grey bar
	drawBar();

	// target zone
	drawTargets();

	glLineWidth(2);
	glColor3f(1.0F,0.25F, 0.25F); // red (top)
	drawArrow(0.0f, -level1);

	glColor3f(.25F,0.25F, 1.F);  // blue 
	drawArrow(0.0f, level2);

	// swap buffer after drawing
	glutSwapBuffers();
}


// display a cursor movements of which are controlled by force transducer
void OneBarPlot::Display(float level1, float level2)
{

	// 
	//float offset =  0.5F*(mTargetMin +  mTargetMax);
	//float tmp = level1;
	//level1 = (level2 - level1) + offset; // net force (centered on offset)
	//level2 = 0.5F*(tmp + level2); // grip force

	//// scale variable
	//if(level1<mMin) level1 = mMin;
	//if(level1>mMax) level1 = mMax;
	//level1 = level1*mHeight/(mMax-mMin)-mHeight/2.0f;
	////
	//if(level2<mMin) level2 = mMin;
	//if(level2>mMax) level2 = mMax;
	//level2 = level2*mHeight/(mMax-mMin)-mHeight/2.0f;

	// clean drawing board
	glClear(GL_COLOR_BUFFER_BIT);

	drawBar();

	// target zone
	drawTargets();

	glLineWidth(2);
	float offset =  0.5f*(mTargetMin +  mTargetMax); // grip force
	glColor3f(1.0F,0.25F, 0.25F); // red (top)
	drawArrow(0.0f, level1);

	glColor3f(.25F,0.25F, 1.F);  // blue 
	drawArrow(0.0f, level2);

	// swap buffer after drawing
	glutSwapBuffers();
};



//
//// display a cursor movements of which are controlled by force transducer
//void OneBarPlot::Display(float level1, float level2)
//{
//	// scale variable
//	if(level1<mMin) level1 = mMin;
//	if(level1>mMax) level1 = mMax;
//	level1 = level1*mHeight/(mMax-mMin)-mHeight/2.0f;
//	//
//	if(level2<mMin) level2 = mMin;
//	if(level2>mMax) level2 = mMax;
//	level2 = level2*mHeight/(mMax-mMin)-mHeight/2.0f;
//
//	// clean drawing board
//	glClear(GL_COLOR_BUFFER_BIT);
//
//	// bar
//	glColor3f(0.5F,0.5F, 0.5F);
//	glBegin(GL_POLYGON);
//	glVertex2f(-mWidth/2.0f,-mHeight/2.0f);
//	glVertex2f( mWidth/2.0f,-mHeight/2.0f);
//	glVertex2f( mWidth/2.0f, mHeight/2.0f);
//	glVertex2f(-mWidth/2.0f, mHeight/2.0f);
//	glEnd();
//	// target zone
//	glColor3f(0.25F,0.75F, 0.25F);
//	if(mScaledTargetMin==mScaledTargetMax){
//		glLineWidth(3);
//		glBegin(GL_LINES);
//		glVertex2f(-mWidth/2, mScaledTargetMin);
//		glVertex2f( mWidth/2, mScaledTargetMin);
//		glEnd();
//	} else {
//		glBegin(GL_POLYGON);
//		glVertex2f(-mWidth/2, mScaledTargetMin);
//		glVertex2f( mWidth/2, mScaledTargetMin);
//		glVertex2f( mWidth/2, mScaledTargetMax);
//		glVertex2f(-mWidth/2, mScaledTargetMax);
//		glEnd();
//	}
//	// level
//	glLineWidth(2);
//	glBegin(GL_LINES);
//	glColor3f(1.0F,0.25F, 0.25F);
//	glVertex2f(-mWidth/2,level1);
//	glVertex2f( mWidth/2,level1);
//	glColor3f(.25F,0.25F, 1.F);
//	glVertex2f(-mWidth/2,level2);
//	glVertex2f( mWidth/2,level2);
//	glEnd();
//
//	// swap buffer after drawing
//	glutSwapBuffers();
//};



// ------------------------------------------------------------------
/* 
	BarPlot
	Display several bars
*/

// object to screen
BarPlot *BarPlot::ObjectPtr;

BarPlot::BarPlot(){
	BarPlot::ObjectPtr=this;

	mHeight=1.5;
	mWidth=0.5;
	mMin=NULL;
	mMax=NULL;
	mNumBars = 0;
}

/**
	\param n Number of bars desired
*/
BarPlot::BarPlot(int n){

	BarPlot::ObjectPtr=this;

	mNumBars = n;
	mCenterPos = new float[n];

	mMin = new float[n];
	mMax = new float[n];
	mTargets = false;
	mTargetMin = new float[n]; 
	mTargetMax = new float[n]; 
	mTicks = false;
	mTickStep = new float[n]; 
	mLevel = new float[n];
	for(int i=0; i<mNumBars;i++) mLevel[i] = 0.0f;
	mLabels = false;
	mLabel = new char*[n];
	for(int i=0; i<mNumBars;i++)  mLabel[i] = NULL;
}

BarPlot::~BarPlot(){
	delete[] mMin;
	delete[] mMax;
	delete[] mTargetMin; 
	delete[] mTargetMax; 
	delete[] mCenterPos;
	if(mLabels) for(int i=0; i<mNumBars;i++) delete[] mLabel[i];
	delete[] mLabel;
}

void BarPlot::SetSize(float height, float width, float spacing){
	mHeight = height;
	mWidth = width;
	mSpacing = spacing;
	float offset = -(mNumBars-1)*mWidth*(1+mSpacing)/2.0F;
	for(int i=0; i<mNumBars;i++) {
		mCenterPos[i] = i*mWidth*(1+mSpacing) + offset;
	}
}


void BarPlot::SetLevels(float level[]){
	for(int i=0; i<mNumBars;i++) mLevel[i]=level[i];
}

void BarPlot::SetLimits(float min[], float max[], float tickStep[]){
	for(int i=0; i<mNumBars;i++){
		mMin[i] = min[i];
		mMax[i] = max[i];
	}
	if(tickStep!=NULL) {
		mTicks = true;
		for(int i=0; i<mNumBars;i++) mTickStep[i] = tickStep[i];
	} else {
		mTicks = false;
	}
}

void BarPlot::SetTargets(float targetMin[], float targetMax[]){
	/*if(targetMin = 0) {
		mTargets = false;
		return;
	}*/
	mTargets=true;
	for(int i=0; i<mNumBars;i++){
		mTargetMin[i] = targetMin[i];
		if(targetMax==0) mTargetMax[i] = mTargetMin[i];
		else mTargetMax[i] =targetMax[i];
	}
}

void BarPlot::SetLabels(const char** label,int length){
	mLabels=true;
	for(int i=0; i<mNumBars;i++){
		delete[] mLabel[i];
		mLabel[i] = new char[length];
		strcpy_s(mLabel[i],length, label[i]);
	}
}
void BarPlot::SetWindow(int Window){
	mWindow=Window;
}

int BarPlot::MakeWindow(int x, int y, int w, int h, const char *title){

	glutInitWindowPosition(x,y);
	glutInitWindowSize(w,h);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	mWindow = glutCreateWindow(title);

	return mWindow;
}


/*
 *
 * Callbacks
 *
 */

// set world coordinate to [-1,-1,1,1]
void BarPlot::Reshape(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
}

void BarPlot::Redisplay(){
	glutPostWindowRedisplay(mWindow);
}

//Forward declaration: this function is defined in plotWaves.cpp
void drawString (char *s);

// display a cursor movements of which are controlled by force transducer
void BarPlot::Display(){

	glutSetWindow(mWindow);

	// clean drawing board
	glClear(GL_COLOR_BUFFER_BIT);


};

void BarPlot::DrawBars(){
	glClear(GL_COLOR_BUFFER_BIT);
	for(int i=0; i<mNumBars; i++){
		// bar
		glColor3f(0.5F,0.5F, 0.5F);
		glBegin(GL_POLYGON);
		glVertex2f(-mWidth/2.0f+mCenterPos[i],-mHeight/2.0f);
		glVertex2f( mWidth/2.0f+mCenterPos[i],-mHeight/2.0f);
		glVertex2f( mWidth/2.0f+mCenterPos[i], mHeight/2.0f);
		glVertex2f(-mWidth/2.0f+mCenterPos[i], mHeight/2.0f);
		glEnd();
		// labels
		glColor3f(1.0F,1.0F, 1.0F);
		if(mLabels){
			glRasterPos2f(mCenterPos[i]-0.02F, mHeight/2.0f+0.04F);
			drawString(mLabel[i]);
		}
		if(mTargets) {
			// target zone
			glColor3f(0.25F,0.75F, 0.25F);
			float scaledTargetMin = mTargetMin[i]*mHeight/(mMax[i]-mMin[i])-mHeight/2;
			if(mTargetMax==0){
				glLineWidth(3);
				glBegin(GL_LINES);
				glVertex2f(-mWidth/2+mCenterPos[i], scaledTargetMin);
				glVertex2f( mWidth/2+mCenterPos[i], scaledTargetMin);
				glEnd();
			} else {
				float scaledTargetMax = mTargetMax[i]*mHeight/(mMax[i]-mMin[i])-mHeight/2;
				glBegin(GL_POLYGON);
				glVertex2f(-mWidth/2+mCenterPos[i], scaledTargetMin);
				glVertex2f( mWidth/2+mCenterPos[i], scaledTargetMin);
				glVertex2f( mWidth/2+mCenterPos[i], scaledTargetMax);
				glVertex2f(-mWidth/2+mCenterPos[i], scaledTargetMax);
				glEnd();
			}
		}
		// rescale level variable
		float level = mLevel[i]; 
		
		if(level>0.0)
			glColor3f(1.0F,0.25F, 0.25F);
		else
			glColor3f(0.25F, 1.0F, 0.25F);

		level=sqrt(level*level);

		if(level<mMin[i]) level = mMin[i];
		if(level>mMax[i]) level = mMax[i];
		level = level*mHeight/(mMax[i]-mMin[i])-mHeight/2.0f;
	
		// level
		//glColor3f(1.0F,0.25F, 0.25F);
		//glLineWidth(2);
		//glBegin(GL_LINES);
		//glVertex2f(-mWidth/2+mCenterPos[i],level);
		//glVertex2f( mWidth/2+mCenterPos[i],level);
		//glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(-mWidth/2.0f+mCenterPos[i],-mHeight/2.0f);
		glVertex2f( mWidth/2.0f+mCenterPos[i],-mHeight/2.0f);
		glVertex2f( mWidth/2.0f+mCenterPos[i],level);
		glVertex2f(-mWidth/2.0f+mCenterPos[i],level);
		glEnd();

		// ticks
		if(mTicks) {
			glColor3f(0.5F,0.5F, 0.5F);
			for(float tick = mMin[i]; tick<=mMax[i]; tick+=mTickStep[i]){
				float rescaledTick = tick*mHeight/(mMax[i]-mMin[i])-mHeight/2.0f;
				glLineWidth(2);
				glBegin(GL_LINES);
				glVertex2f(-mWidth/2+mCenterPos[i]-0.005F,rescaledTick);
				glVertex2f(-mWidth/2+mCenterPos[i],rescaledTick);
				glEnd();
				// tick label
				char buf[100];
				sprintf_s(buf,100,"%1.1f",tick);
				glRasterPos2f(mCenterPos[i]-mWidth/2.0f-0.065f,rescaledTick-0.01F);
				drawString(buf);
			}
		}
	}

};
/*
	GLUT MENU
*/

// callbacks for default menu entries
void BarPlotCreateGUI(){
	BarPlot::ObjectPtr->CreateGUI();
}

// menu callback
void BarPlotMenuCallback(int entry){
	BarPlot::ObjectPtr->_menuCallbacks[entry]();
}


// create menu and attach default entries
void BarPlot::CreateMenu(){
	_numEntry=0;
	glutCreateMenu(BarPlotMenuCallback);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	AddMenuEntry("Plot Options...",&BarPlotCreateGUI);
}

void BarPlot::AddMenuEntry(char *text, void (*callback)(void)){
	_menuCallbacks[_numEntry]=callback;
	glutAddMenuEntry(text, _numEntry++);
}


/*
  
  GRAPHICAL USER INTERFACE

  Note: there is a problem when closing GLUI windows containing GLUI checkboxes. It looks
  like a glutSetWindow() toward the destroyed GLUI windows is called when a new one is
  created (iff checkboxes values have been changed!).

*/

void BarPlot::Message(char *text){
		printf("%s\n",text); // console output
}

void CloseBarPlotGUI(void){
//	printf("CloseBarPlotGUI\n");
//	free(BarPlot::ObjectPtr->_status);
	//
	BarPlot::ObjectPtr->_glui=NULL;
	BarPlot::ObjectPtr->_gluiWindowId=0;
};


void BarPlot::CreateGUI(){
//	printf("CreateGUI\n");
	char text[100];

	/*
	if(_gluiWindowId!=0) {
		Message("UI already exists\n");
		return;
	} */

	// create UI
	_glui = GLUI_Master.create_glui("BarPlot Settings", 0 );
	_glui->set_main_gfx_window(mWindow);
	_gluiWindowId = _glui->get_glut_window_id();

	for(int channel=0;channel<mNumBars;channel++) {
		sprintf_s(text,"Channel %2d",channel);
		GLUI_Panel *ChannelPanel = _glui->add_panel(text);

		_glui->add_column_to_panel(ChannelPanel,false);
		_glui->add_spinner_to_panel(ChannelPanel, "Min",GLUI_SPINNER_FLOAT,&mMin[channel]);
		_glui->add_column_to_panel(ChannelPanel,false);
		_glui->add_spinner_to_panel(ChannelPanel, "Max",GLUI_SPINNER_FLOAT,&mMax[channel]);
		_glui->add_column_to_panel(ChannelPanel,false);
		_glui->add_spinner_to_panel(ChannelPanel, "Step",GLUI_SPINNER_FLOAT,&mTickStep[channel]);
	}

	// sync GUI
	_glui->sync_live();

	// retrieve glut window Id and set callabck 
	int w = glutGetWindow();
//	printf("BarPlots::CreateGui - current window %d\n",_gluiWindowId);
	glutSetWindow(_gluiWindowId);
	glutCloseFunc(CloseBarPlotGUI);			// freeglut extension
//	printf("BarPlots::CreateGui - current window %d\n",w);
	glutSetWindow(w);
}

/* This function renders a character string onto the current position */
void BarPlot::drawString (char *s){
	unsigned int i;
	for (i = 0; i < strlen (s); i++)
		glutBitmapCharacter (GLUT_BITMAP_HELVETICA_18, s[i]);	
};

/* This function renders a character string using bigger fonts onto the current position */
void BarPlot::drawStringBig (char *s){	
	unsigned int i;
	for (i = 0; i < strlen (s); i++)
		//glutBitmapCharacter (GLUT_BITMAP_HELVETICA_18, s[i]);
		glutBitmapCharacter (GLUT_BITMAP_TIMES_ROMAN_24, s[i]); 
		//glutBitmapCharacter (GLUT_BITMAP_9_BY_15 , s[i]);
}

void BarPlot::DisplayText(char *Text,float X, float Y)
{
	// glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0F,1.0F, 1.0F);
	glRasterPos2f(X, Y); // 0.5 W, 1.5 H
	drawStringBig(Text);

	// swap buffer after drawing
	glutSwapBuffers();
}
void BarPlot::addText(char *Text,float X, float Y, float colorArray[])
{
	// glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(colorArray[0],colorArray[1], colorArray[2]); // 1.0 0.25 0.25 // 0.25 1.0 0.25
	glRasterPos2f(X, Y); // 0.5 W, 1.5 H
	drawString(Text);

	// swap buffer after drawing
	// glutSwapBuffers();
}

void BarPlot::DisplayTrialInstruction(int trial, char *mode){// trial task as arg
	sprintf(trialsText,"Trial # %d.",trial);
	// clean drawing board
	glClear(GL_COLOR_BUFFER_BIT);
	// condition

	glColor3f(0.25F, 1.0F, 0.25F);
	glRasterPos2f(-mWidth/2.0f-0.1f, (mHeight/2.0f-0.5F)/2.0F);
	drawStringBig (trialsText);

	glColor3f(1.0F,1.0F, 1.0F);
	glRasterPos2f(-mWidth/2.0f-0.7f, (mHeight/2.0f-0.9F)/2.0F);
	drawStringBig (mode);
	// swap buffer after drawing
	glutSwapBuffers();

}