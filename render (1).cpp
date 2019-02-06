#include <Bela.h>
#include <string.h>
#include <Scope.h>
#include <cmath>

#define LOOP_BUFFER_SIZE 882000  //20 seconds worth of looping

float gLoopBuffer[LOOP_BUFFER_SIZE] = {0};	 //Looper buffer
int gLoopBufferPointer = 0;
int gAudioFramesPerAnalogFrame = 0;
float gInverseSampleRate;
int loopLength = 0;   //Used to shorten buffer to length of recorded loop
float linGain = 1;
float gain = 1;
float shape = 3;

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
		
		

	if (control != lastControl)
		{
			if (control)
			{
				m++;
				rt_printf("m = %i\n", m);
			}
		}
			
	lastControl = control;
	
	
	
	for (unsigned int n = 0; n < context->audioFrames; n++)
	{
		
		float in = audioRead(context,n,0);
		float out = 0;
		
		
		if(!(n % gAudioFramesPerAnalogFrame))
		{
			linGain = map(analogRead(context, n/gAudioFramesPerAnalogFrame, gVolumeKnob), 0.0, 1.0, 0.1, 1.0);
		}
		
		gain = (std::exp(linGain*shape)-1)/(std::exp(shape)-1);
		
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
				gLoopBufferPointer += in;
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
		}
		
		audioWrite(context, n, 0, out);
		
	}
}

void cleanup(BelaContext *context, void *userData)
{

}