#include <Bela.h>
#include <string.h>
#include <Scope.h>
#include <cmath>

#define LOOP_BUFFER_SIZE 882000  //20 seconds worth of looping (sampled at 44100 Hz)

unsigned int gAudioFramesPerAnalogFrame = 0;
float gInverseSampleRate;
float gLoopBuffer[LOOP_BUFFER_SIZE] = {0.0};	//Looper buffer
static unsigned int gLoopBufferPointer = 0;
static unsigned int loopLength = 0;   		//Used to shorten buffer to length of recorded loop
float linGain = 1.0;				//Value read from analog input (linearly increases from 0-1 for linear pots)
float gain = 1.0;				//Will be converted to exponential (dB) increase
float shape = 3.0;				//Controls knee point of dB volume increase

int gRecordPin = 0;
int gVolumeKnob = 0;

int lastControl = 0;
int m = 0;

bool setup(BelaContext *context, void *userData)
{
	pinMode(context, 0, gRecordPin, INPUT);
	gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;
	gInverseSampleRate = 1 / context->audioSampleRate;
	return true;
}

void render(BelaContext *context, void *userData)
{
	
	int control = digitalRead(context, 0, gRecordPin);
	float in = audioRead(context,n,0);
	float out = 0.0;
		
		

	if (control != lastControl)		//Detects pushbutton press
		{				//NOTE: Not debounced, was developed as a proof of concept
			if (control)		//For implementation, a debouncer should be used
			{
				m++;
				rt_printf("m = %i\n", m);
			}
		}
			
	lastControl = control;
	
	
	
	for (unsigned int n = 0; n < context->audioFrames; n++)
	{
		
		if(!(n % gAudioFramesPerAnalogFrame))
		{
			linGain = map(analogRead(context, n/gAudioFramesPerAnalogFrame, gVolumeKnob), 0.0, 1.0, 0.1, 1.0);
			gain = (std::exp(linGain*shape)-1)/(std::exp(shape)-1);
		}
		
		
		switch (m)
		{
			case 0:				//Bypassed state
				out = in;
				break;
				
			case 1:				//Recording first iteration of loop, sets loop length
				if(++loopLength>=LOOP_BUFFER_SIZE)
					loopLength = 0;			//If loop tries to go longer than buffer size, wraps back to 0
				gLoopBuffer[loopLength] = in;
				out = in;
				break;
				
			case 2:				//Overdubbing over first recording
				if(++gLoopBufferPointer>=loopLength)
					gLoopBufferPointer = 0;
				out = in + gLoopBuffer[gLoopBufferPointer]*gain;
				gLoopBuffer[gLoopBufferPointer] += in;
				break; 
				
			case 3:				//Outputting complete loop without recording anymore
				if(++gLoopBufferPointer>=loopLength)
					gLoopBufferPointer = 0;
				out = in + gLoopBuffer[gLoopBufferPointer]*gain;
				break;
				
			case 4:				//Resets buffer length and pointer (doesn't clear buffer)
				loopLength = 0;
				gLoopBufferPointer = 0;
				m = 0;
				break;
				
			default:
				m = 0;
				break;
		}
		
		audioWrite(context, n, 0, out);
		
	}
}

void cleanup(BelaContext *context, void *userData)
{

}
