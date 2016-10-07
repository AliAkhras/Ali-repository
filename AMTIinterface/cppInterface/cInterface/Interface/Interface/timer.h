
typedef struct
{
  float told;
  float sum;
  float sumSq;
  unsigned n;
  float min;
  float	max;
  float mean;
  float sd;
  float freq;

} LOGGER;

typedef struct
{
	int				performance_timer; //boolean
	__int64			frequency;
	float			resolution;
	__int64			performance_timer_start;
	__int64			performance_timer_elapsed;
	unsigned long	mm_timer_start;
	unsigned long	mm_timer_elapsed;
	// 
} TIMER;


void TimerInit(TIMER *timer);		// Initialize timer (get it ready)
void TimerReset(TIMER *timer);		// Reset timer 
float TimerGetTime(TIMER *timer);	// Get Elapsed Time In Milliseconds

void LoggerReset(LOGGER *log);		
void LoggerTick(LOGGER *log, float t);		
void LoggerRecord(LOGGER *log, float dt);	
void LoggerPrint(LOGGER *log);		// Initialize timer (get it ready)
