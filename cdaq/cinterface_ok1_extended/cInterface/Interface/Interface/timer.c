#include <memory.h> //memset
#include <windows.h> // QueryPerformanceFrequency, QueryPerformanceCounter
#include <stdio.h> // QueryPerformanceFrequency, QueryPerformanceCounter
#include "timer.h"

void TimerInit(TIMER *timer)								// Initialize Our Timer (Get It Ready)
{
	memset(timer, 0, sizeof(timer));						// Clear Our Timer Structure

	// Check To See If A Performance Counter Is Available
	// If One Is Available The Timer Frequency Will Be Updated
	if (!QueryPerformanceFrequency((LARGE_INTEGER *) &timer->frequency))
	{
		// No Performace Counter Available
		timer->performance_timer	= FALSE;				// Set Performance Timer To FALSE
		timer->mm_timer_start	= timeGetTime();			// Use timeGetTime() To Get Current Time
		timer->resolution		= 1.0f/1000.0f;				// Set Our Timer Resolution To .001f
		timer->frequency		= 1000;						// Set Our Timer Frequency To 1000
		timer->mm_timer_elapsed	= timer->mm_timer_start;	// Set The Elapsed Time To The Current Time
	}
	else
	{
		// Performance Counter Is Available, Use It Instead Of The Multimedia Timer
		// Get The Current Time And Store It In performance_timer_start
		QueryPerformanceCounter((LARGE_INTEGER *) &timer->performance_timer_start);
		timer->performance_timer			= TRUE;				// Set Performance Timer To TRUE
		// Calculate The Timer Resolution Using The Timer Frequency
		timer->resolution				= (float) (((double)1.0f)/((double)timer->frequency));
		// Set The Elapsed Time To The Current Time
		timer->performance_timer_elapsed	= timer->performance_timer_start;
	}
}

// Reset counter
void TimerReset(TIMER *timer)								
{
	if (timer->performance_timer)							
	{
		QueryPerformanceCounter((LARGE_INTEGER *) &timer->performance_timer_start);
		timer->performance_timer_elapsed	= timer->performance_timer_start;
	} else {
		timer->performance_timer_start= timeGetTime();
		timer->mm_timer_elapsed	= timer->mm_timer_start;	
	}
}

// returns elapsed time since initialization
float TimerGetTime(TIMER *timer)							// Get Time In Milliseconds
{
	__int64 time;											// time Will Hold A 64 Bit Integer

	if (timer->performance_timer)							// Are We Using The Performance Timer?
	{
		QueryPerformanceCounter((LARGE_INTEGER *) &time);	// Grab The Current Performance Time
		// Return The Current Time Minus The Start Time Multiplied By The Resolution And 1000 (To Get MS)
		return ( (float) ( time - timer->performance_timer_start) * timer->resolution)*1000.0f;
	}
	else
	{
		// Return The Current Time Minus The Start Time Multiplied By The Resolution And 1000 (To Get MS)
		return( (float) ( timeGetTime() - timer->mm_timer_start) * timer->resolution)*1000.0f;
	}
}



void LoggerReset(LOGGER *log) {
	log->sum=log->sumSq=0.0;
	log->min=1000.0; //1 Hz
	log->max=log->mean=log->freq=log->sd=0.0;
	log->n=0;
}

void LoggerRecord(LOGGER *log, float dt){		
	if(dt<log->min) log->min=dt;
	if(dt>log->max) log->max=dt;
	log->sum+=dt;
	log->sumSq+=dt*dt;
	log->n++;
}
void LoggerTick(LOGGER *log, float t){		
	float dt;
	dt=t-log->told;
	if(log->n>0){
		if(dt<log->min) log->min=dt;
		if(dt>log->max) log->max=dt;
		log->sum+=dt;
		log->sumSq+=dt*dt;
		log->told=t;
	} else {
		log->told=t;
	}
	log->n++;
}

void LoggerPrint(LOGGER *log){
		if(log->sum > 0) {
			log->mean=log->sum/log->n;
			log->freq=1000.0F/log->mean;
			log->sd=log->sumSq/log->n - (log->mean)*(log->mean);
		} else {
			log->mean=log->freq=log->sd=0.0;
		}
		printf("Mean: %6.4f ms, %g Hz (SD=%6.4g, Min=%6.4g, Max=%6.4g, N=%u)\n",
			log->mean,log->freq,log->sd,log->min,log->max,log->n);
}

