/*
 * Copyright 2010 Forwardcode, LLC
 *
 * This file is part of Ventafone.
 *
 * Ventafone is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Ventafone is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Ventafone.  If not, see <http://www.gnu.org/licenses/>.
 */

#import "AudioController.h"

#define kOutputBus 0
#define kInputBus 1
#define bufferSize 204800		// Represents bytes
#define silenceBufferSize 20480	// Represents bytes
#define tempBufferSize 20480	// Represents bytes


@implementation AudioController

@synthesize outputBuffer;
@synthesize outputHead;
@synthesize outputTail;
@synthesize outputCount;
@synthesize inputBuffer;
@synthesize inputHead;
@synthesize inputTail;
@synthesize inputCount;
@synthesize silenceBuffer;
@synthesize tempBuffer;
@synthesize audioUnit;
@synthesize isRecording;
@synthesize isRunning;
@synthesize isOutput;
@synthesize callInterrupt;
@synthesize queueOutputStop;
@synthesize outputLock;
@synthesize inputLock;
@synthesize pcmLength;


#pragma mark -
#pragma mark Callback subscribers

// audio render procedure, don't allocate memory, don't take any locks, don't waste time
static OSStatus playbackCallback(void *inRefCon, 
								 AudioUnitRenderActionFlags *ioActionFlags, 
								 const AudioTimeStamp *inTimeStamp, 
								 UInt32 inBusNumber, 
								 UInt32 inNumberFrames, 
								 AudioBufferList *ioData) {  
    // Notes: ioData contains buffers (may be more than one!)
    // Fill them up as much as you can. Remember to set the size value in each buffer to match how
    // much data is in the buffer.
	
	
	// Get a reference to the object that was passed with the callback
	// In this case, the AudioController passed itself so
	// that you can access its data.
	AudioController *THIS = (AudioController*)inRefCon;
	
	// Get a pointer to the dataBuffer of the AudioBufferList
	//	AudioSampleType *outA = (AudioSampleType *)ioData->mBuffers[0].mData;
	
	
	// Lock cyclic buffer
	[[THIS outputLock] lock];
	
//	NSLog(@"BAH: %d, BAT: %d, QUEUE: %d, FRAMES: %d\n",
//		  [THIS outputBuffer]+bufferSize-[THIS outputHead],
//		  [THIS outputBuffer]+bufferSize-[THIS outputTail],
//		  [THIS outputCount],
//		  inNumberFrames);

//	if (  (int)(([THIS buffer]+bufferSize)-[THIS tail]) == 96320 ) {
//		// Let's break, we're about to hit it for real
//		printf("Buffer: %d\n", (uint32_t)[THIS buffer]);
//		printf("Head: %d\n", (uint32_t)[THIS head]);
//		printf("Tail: %d\n", (uint32_t)[THIS tail]);
//		printf("Buffer size: %d\n", bufferSize);
//		printf("Tail - buffer: %d\n", (uint32_t)([THIS tail]-[THIS buffer]));
//		printf("Buffer size - previous: %d\n", (bufferSize - ([THIS tail]-[THIS buffer])));
//		printf("\n");
//	}
	if ([THIS queueOutputStop])
	{
		if ([THIS outputCount] > 0)
		{
			// We have some data
			int size;
			if ((int)[THIS outputCount] > ((int)inNumberFrames*2) ) {
				size = (int)inNumberFrames*2;
			}
			else {
				size = (int)[THIS outputCount];
				inNumberFrames = size/2;
			}
			
			if ( ((int)[THIS outputTail]-(int)[THIS outputHead]) > 0)
			{
				// There is no looping
				ioData->mBuffers[inBusNumber].mDataByteSize = size;
				//uint8_t *thePointer = [THIS outputHead];
				ioData->mBuffers[inBusNumber].mData = [THIS outputHead];
				// Increment head
				uint8_t *theHead = [THIS outputHead];
				theHead = theHead + size;
				[THIS setOutputHead:theHead];
				// Decrement count
				uint32_t theCount = [THIS outputCount];
				theCount = (int)theCount - size;
				[THIS setOutputCount:theCount];
			}
			else
			{
				// Tail <= Head  - Check to see if our frames are not split up
				int lengthToEnd = (int)bufferSize - ((int)[THIS outputHead]-(int)[THIS outputBuffer]);
				if (lengthToEnd < size )
				{
					// They are split.. copy whatever we have until the end of the buffer to the temp buffer
					memcpy([THIS tempBuffer]->mBuffers[0].mData,[THIS outputHead],lengthToEnd);
					// Now copy the rest to the temp buffer
					int lengthRemaining = size - lengthToEnd;
					memcpy([THIS tempBuffer]->mBuffers[0].mData + lengthToEnd,[THIS outputBuffer],lengthRemaining);
					ioData->mBuffers[inBusNumber].mData = [THIS tempBuffer]->mBuffers[0].mData;
					ioData->mBuffers[inBusNumber].mDataByteSize = (size);
					
					// Increment head
					uint8_t *theHead = [THIS outputBuffer] + lengthRemaining;
					[THIS setOutputHead:theHead];
					// Decrement count
					uint32_t theCount = [THIS outputCount];
					theCount = (int)theCount - size;
					[THIS setOutputCount:theCount];
					
				}
				else {
					// Fill up all the way to the request
					uint8_t *thePointer = [THIS outputHead];
					ioData->mBuffers[inBusNumber].mData = thePointer;
					ioData->mBuffers[inBusNumber].mDataByteSize = size;
					// Increment head
					uint8_t *theHead = [THIS outputHead];
					theHead = theHead + size;
					[THIS setOutputHead:theHead];
					// Decrement count
					uint32_t theCount = [THIS outputCount];
					theCount = (int)theCount - size;
					[THIS setOutputCount:theCount];
				}
				
			}
		}
		else
		{
//			// Buffer is clear.. stop output
//			if ([THIS isOutput]) {
//				if ([THIS outputCount] == 0 && [THIS queueOutputStop]) {
//					[THIS setQueueOutputStop:NO];
//				}
//
//			}
			
			ioData->mBuffers[inBusNumber].mData = [THIS silenceBuffer]->mBuffers[0].mData;
			ioData->mBuffers[inBusNumber].mDataByteSize = ((int)inNumberFrames*2);
			
			
		}
	}
	else {
		if (  (int)[THIS outputCount] >= ((int)inNumberFrames*2))
		{
			// We have enough data to give
			if ( ((int)[THIS outputTail]-(int)[THIS outputHead]) > 0)
			{
				// There is no looping
				ioData->mBuffers[inBusNumber].mDataByteSize = ((int)inNumberFrames*2);
				//uint8_t *thePointer = [THIS outputHead];
				ioData->mBuffers[inBusNumber].mData = [THIS outputHead];
				// Increment head
				uint8_t *theHead = [THIS outputHead];
				theHead = theHead + (inNumberFrames*2);
				[THIS setOutputHead:theHead];
				// Decrement count
				uint32_t theCount = [THIS outputCount];
				theCount = (int)theCount - ((int)inNumberFrames*2);
				[THIS setOutputCount:theCount];
			}
			else {
				// Tail <= Head  - Check to see if our frames are not split up
				int lengthToEnd = (int)bufferSize - ((int)[THIS outputHead]-(int)[THIS outputBuffer]);
				if (lengthToEnd < ((int)inNumberFrames*2) )
				{
					// They are split.. copy whatever we have until the end of the buffer to the temp buffer
					memcpy([THIS tempBuffer]->mBuffers[0].mData,[THIS outputHead],lengthToEnd);
					// Now copy the rest to the temp buffer
					int lengthRemaining = ((int)inNumberFrames*2) - lengthToEnd;
					memcpy([THIS tempBuffer]->mBuffers[0].mData + lengthToEnd,[THIS outputBuffer],lengthRemaining);
					ioData->mBuffers[inBusNumber].mData = [THIS tempBuffer]->mBuffers[0].mData;
					ioData->mBuffers[inBusNumber].mDataByteSize = ((int)inNumberFrames*2);
					
					// Increment head
					uint8_t *theHead = [THIS outputBuffer] + lengthRemaining;
					[THIS setOutputHead:theHead];
					// Decrement count
					uint32_t theCount = [THIS outputCount];
					theCount = (int)theCount - ((int)inNumberFrames*2);
					[THIS setOutputCount:theCount];
				}
				else {
					// Fill up all the way to the request
					uint8_t *thePointer = [THIS outputHead];
					ioData->mBuffers[inBusNumber].mData = thePointer;
					ioData->mBuffers[inBusNumber].mDataByteSize = ((int)inNumberFrames*2);
					// Increment head
					uint8_t *theHead = [THIS outputHead];
					theHead = theHead + (inNumberFrames*2);
					[THIS setOutputHead:theHead];
					// Decrement count
					uint32_t theCount = [THIS outputCount];
					theCount = (int)theCount - ((int)inNumberFrames*2);
					[THIS setOutputCount:theCount];
				}
				
			}
		}
		else {
			// Output silence
			ioData->mBuffers[inBusNumber].mData = [THIS silenceBuffer]->mBuffers[0].mData;
			ioData->mBuffers[inBusNumber].mDataByteSize = ((int)inNumberFrames*2);
		}
	}
	
	// Unlock cyclic buffer
	[[THIS outputLock] unlock];

	
    return noErr;
}



static OSStatus recordingCallback(void *inRefCon, 
                                  AudioUnitRenderActionFlags *ioActionFlags, 
                                  const AudioTimeStamp *inTimeStamp, 
                                  UInt32 inBusNumber, 
                                  UInt32 inNumberFrames, 
                                  AudioBufferList *ioData)
{
	AudioController *THIS = (AudioController *)inRefCon;
	
	if (THIS->isRecording)
	{
		[[THIS inputLock] lock];
		
//		NSLog(@"BAH: %d, BAT: %d, QUEUE: %d, FRAMES: %d\n",
//			  [THIS inputBuffer]+bufferSize-[THIS inputHead],
//			  [THIS inputBuffer]+bufferSize-[THIS inputTail],
//			  [THIS inputCount],
//			  inNumberFrames);
		
//		NSLog(@"Frames before: %d\n", inNumberFrames);
//		NSLog(@"Action flags: %d\n", ioActionFlags);
//		NSLog(@"Time stamp: %d\n", inTimeStamp);
//		NSLog(@"Bus number: %d\n", inBusNumber);
		
		AudioBufferList *bufferList;
		
		bufferList = (AudioBufferList*) malloc(sizeof(AudioBufferList));
		bufferList->mNumberBuffers = 1; //audioFormat.mChannelsPerFrame
		for(UInt32 i=0;i<bufferList->mNumberBuffers;i++)
		{
			bufferList->mBuffers[i].mNumberChannels = 1;
			bufferList->mBuffers[i].mDataByteSize = inNumberFrames * 2;
			bufferList->mBuffers[i].mData = malloc(bufferList->mBuffers[i].mDataByteSize);
		}
		
		OSStatus result = AudioUnitRender(THIS.audioUnit,ioActionFlags,inTimeStamp,inBusNumber,inNumberFrames,bufferList);

		switch(result)
		{
			case noErr:
			{
				// Now, we have the samples we just read sitting in buffers in bufferList
//				NSLog(@"Frames after: %d\n", inNumberFrames);
				[THIS queueInput:inNumberFrames*2 Sample:bufferList->mBuffers[0].mData];
				break;
			}
			case kAudioUnitErr_InvalidProperty: NSLog(@"AudioUnitRender Failed: Invalid Property"); break;
			case -50: NSLog(@"AudioUnitRender Failed: Invalid Parameter(s)"); break;
			default: NSLog(@"AudioUnitRender Failed: Unknown (%d)",result); break;
		}
		
		free(bufferList->mBuffers[0].mData);
		free(bufferList);
		

		[[THIS inputLock] unlock];
	}
	
	return noErr;
	

	
}

//- (void)rioInterruptionListener(void* inUserData, UInt32 interruptionState)
//{
//	// TODO: Remove this
//	NSLog(@"Rio Interrupt");
//	AudioController *controller = (AudioController *) inUserData;
//	if (interruptionState == kAudioSessionBeginInterruption)
//	{
//		[controller stopAudioSession];
//	}
//	else if (interruptionState == kAudioSessionEndInterruption) {
//		[controller startAudioSession];
//	}
//	
//	UInt32 audioInputIsAvailable;
//	UInt32 propertySize = sizeof (audioInputIsAvailable);
//	AudioSessionGetProperty (
//							 kAudioSessionProperty_AudioInputAvailable,
//							 &propertySize,
//							 &audioInputIsAvailable // A nonzero value on output means that
//							 // audio input is available
//							 );
//    if (audioInputIsAvailable) {
//		NSLog(@"Mic is connected");
//	}
//	else {
//		NSLog(@"Mic is not connected");
//		[(VentafoneAppDelegate *)[[UIApplication sharedApplication] delegate] micNoLongerAvailable];
//	}
//}
	
- (void)dealloc
{
	AudioOutputUnitStop(audioUnit);
	AudioUnitUninitialize(audioUnit);
	AudioSessionSetActive(NO);
	free(silenceBuffer->mBuffers[0].mData);
	free(silenceBuffer);
	free(tempBuffer->mBuffers[0].mData);
	free(tempBuffer);
	free(outputBuffer);
	free(inputBuffer);
	free(tempInput);
	[outputLock release];
	[inputLock release];
	[super dealloc];
}

- (id)initWithRate: (uint32_t) rate
{
	[super init];
	
	outputLock = [[NSLock alloc] init];
	inputLock = [[NSLock alloc] init];
	
	isRecording = NO;
	isRunning = NO;
	isOutput = NO;
	queueOutputStop = NO;
	callInterrupt = NO;
	
	outputBuffer = (uint8_t *) malloc(bufferSize);
	outputHead = outputBuffer;
	outputTail = outputBuffer;
	outputCount = 0;
	
	inputBuffer = (uint8_t *)malloc(bufferSize);
	inputHead = inputBuffer;
	inputTail = inputBuffer;
	inputCount = 0;
	
	// Describe audio component
	AudioComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_RemoteIO;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	
	// Get component
	AudioComponent inputComponent = AudioComponentFindNext(NULL, &desc);
	
	// Get audio units
	if(AudioComponentInstanceNew(inputComponent, &audioUnit) != noErr)
	{
		NSLog(@"Failed creating new audio component instance.");
		return self;
	}
	
	// Enable IO for recording
	UInt32 flag = 1;
	if(AudioUnitSetProperty(audioUnit, 
							kAudioOutputUnitProperty_EnableIO, 
							kAudioUnitScope_Input, 
							kInputBus,
							&flag, 
							sizeof(flag)) != noErr)
	{
		NSLog(@"Failed enabling IO for recording.");
		return self;
	}
	
	
	// Enable IO for playback
	if (AudioUnitSetProperty(audioUnit, 
							 kAudioOutputUnitProperty_EnableIO, 
							 kAudioUnitScope_Output, 
							 kOutputBus,
							 &flag, 
							 sizeof(flag)) != noErr)
	{
		NSLog(@"Failed enabling IO for playback.");
		return self;
	}
	
	// Describe format
	audioFormat.mSampleRate			= rate;
	audioFormat.mFormatID			= kAudioFormatLinearPCM;
	audioFormat.mFormatFlags		= kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	audioFormat.mFramesPerPacket	= 1;
	audioFormat.mChannelsPerFrame	= 1;
	audioFormat.mBitsPerChannel		= 16;
	audioFormat.mBytesPerPacket		= 2;
	audioFormat.mBytesPerFrame		= 2;
	
	pcmLength = v3_pcmlength_for_rate((uint32_t)audioFormat.mSampleRate);
	tempInput = (uint8_t *)malloc(pcmLength);
	
	// Apply format
	if (AudioUnitSetProperty(audioUnit, 
							 kAudioUnitProperty_StreamFormat, 
							 kAudioUnitScope_Output, 
							 kInputBus, 
							 &audioFormat, 
							 sizeof(audioFormat)) != noErr)
	{
		NSLog(@"Failed apply audio format for recording.");
		return self;
	}
	
	if (AudioUnitSetProperty(audioUnit, 
							 kAudioUnitProperty_StreamFormat, 
							 kAudioUnitScope_Input, 
							 kOutputBus, 
							 &audioFormat, 
							 sizeof(audioFormat)) != noErr)
	{
		NSLog(@"Failed apply audio format for playback.");
		return self;
	}
	
	// Set input callback
	inputCallbackStruct.inputProc = recordingCallback;
	inputCallbackStruct.inputProcRefCon = self;
	if (AudioUnitSetProperty(audioUnit, 
							 kAudioOutputUnitProperty_SetInputCallback, 
							 kAudioUnitScope_Global, 
							 kInputBus, 
							 &inputCallbackStruct, 
							 sizeof(inputCallbackStruct)) != noErr)
	{
		NSLog(@"Failed setting audio input callback.");
		return self;
	}
	
	// Set output callback
	outputCallbackStruct.inputProc = playbackCallback;
	outputCallbackStruct.inputProcRefCon = self;
	if (AudioUnitSetProperty(audioUnit, 
							 kAudioUnitProperty_SetRenderCallback, 
							 kAudioUnitScope_Global, 
							 kOutputBus,
							 &outputCallbackStruct, 
							 sizeof(outputCallbackStruct)) != noErr)
	{
		NSLog(@"Failed setting audio output callback.");
		return self;
	}
	
	// Enable buffer allocation
	UInt32 allocFlag = 0;
	if (AudioUnitSetProperty(audioUnit, 
							 kAudioUnitProperty_ShouldAllocateBuffer,
							 kAudioUnitScope_Output, 
							 kInputBus,
							 &allocFlag, 
							 sizeof(allocFlag)) != noErr)
	{
		NSLog(@"Failed initializing recording buffer allocation flag.");
		return self;
	}
	
	// Initialize
	if (AudioUnitInitialize(audioUnit) != noErr)
	{
		NSLog(@"Failed to initialize audio control unit.");
		return self;
	}
	
	// Initialize silence buffer
	silenceBuffer = (AudioBufferList*) malloc(sizeof(AudioBufferList));
	silenceBuffer->mNumberBuffers = 1; //audioFormat.mChannelsPerFrame
	for(UInt32 i=0;i<silenceBuffer->mNumberBuffers;i++)
	{
		silenceBuffer->mBuffers[i].mNumberChannels = 1;
		silenceBuffer->mBuffers[i].mDataByteSize = silenceBufferSize;
		silenceBuffer->mBuffers[i].mData = malloc(silenceBuffer->mBuffers[i].mDataByteSize);
		
		memset(silenceBuffer->mBuffers[i].mData, 0, silenceBuffer->mBuffers[i].mDataByteSize);
	}
	
	// Initialize temporary buffer
	tempBuffer = (AudioBufferList*) malloc(sizeof(AudioBufferList));
	tempBuffer->mNumberBuffers = 1; //audioFormat.mChannelsPerFrame
	for(UInt32 i=0;i<tempBuffer->mNumberBuffers;i++)
	{
		tempBuffer->mBuffers[i].mNumberChannels = 1;
		tempBuffer->mBuffers[i].mDataByteSize = tempBufferSize;
		tempBuffer->mBuffers[i].mData = malloc(tempBuffer->mBuffers[i].mDataByteSize);
	}
	
	//AudioSessionInitialize(NULL,NULL,rioInterruptionListener,self);
	AudioSessionSetActive(YES);
	UInt32 audioCategory = kAudioSessionCategory_PlayAndRecord;
	AudioSessionSetProperty(kAudioSessionProperty_AudioCategory,sizeof(audioCategory),&audioCategory);
	UInt32 doChangeDefaultRoute = 1;
	AudioSessionSetProperty(kAudioSessionProperty_OverrideCategoryDefaultToSpeaker, sizeof (doChangeDefaultRoute), &doChangeDefaultRoute);
	Float32 preferredBufferSize = 12.00;
	AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareIOBufferDuration, sizeof(preferredBufferSize), &preferredBufferSize);
	
	return self;
}

#pragma mark -
#pragma mark Callback implementations

-(void) queueOutput: (uint32_t) length Sample: (uint8_t *) sample
{
//	// Apply any gain needed
//	int32_t pcmval;
//	for (int i = 0; i < (length/2); i++) {
//		int16_t *currentFrame = sample + (i*2);
//		pcmval = *currentFrame * 10000;
//		if (pcmval < 32767 && pcmval > -32768) {
//			*currentFrame = pcmval;
//		} else if (pcmval > 32767) {
//			*currentFrame = 32767;
//		} else if (pcmval < -32768) {
//			*currentFrame = -32768;
//		}
//	}
	
	if (callInterrupt) {
		return;
	}
	// Lock the cyclic buffer
	[outputLock lock];
//	NSLog(@"Queue is now: %d\n",count);
//	NSLog(@"Output Diff before: %d\n",(outputBuffer.buffer+bufferSize)-outputBuffer.tail);
//	if ( ((outputBuffer.buffer+bufferSize)-outputBuffer.tail) < 4000) {
//		// Let's break, we're about to hit it for real
//		printf("We're about to go below 0.\n");
//		printf("Buffer: %d\n", (uint32_t)outputBuffer.buffer);
//		printf("Head: %d\n", (uint32_t)outputBuffer.head);
//		printf("Tail: %d\n", (uint32_t)outputBuffer.tail);
//		printf("Buffer size: %d\n", bufferSize);
//		printf("Length: %d\n", length);
//		printf("Tail - buffer: %d\n", (uint32_t)(outputBuffer.tail-outputBuffer.buffer));
//		printf("Buffer size - previous: %d\n", (bufferSize - (outputBuffer.tail-outputBuffer.buffer)));
//	}
	
	if ( ((int)outputCount + (int)length) > (int)bufferSize)
	{
		// Too many packets.. ignore
		//NSLog(@"CRITICAL ERROR:  INCOMING AUDIO PACKETS HAVE EXCEEDED ALLOCATED BUFFER SPACE (%d BYTES) AND ARE BEING IGNORED\n", bufferSize);
	}
	else
	{
		if ( ((int)bufferSize - ((int)outputTail-(int)outputBuffer)) >= (int)length)
		{
			// We can fit all without looping
			memcpy(outputTail, sample, length);
			outputCount = outputCount + length;
			outputTail = outputTail + length;
		}
		else {
			// We are going to loop the tail
			int lengthToEnd = (int)bufferSize - ((int)outputTail-(int)outputBuffer);
			if (lengthToEnd > 0) {
				// Put some at the end
				memcpy(outputTail,sample,lengthToEnd);
				int lengthRemaining = (int)length - lengthToEnd;
				memcpy(outputBuffer,sample+lengthToEnd,lengthRemaining);
				outputTail = outputBuffer + lengthRemaining;
				outputCount = outputCount + length;
			}
			else {
				// We're at the end, start from beginning
				memcpy(outputBuffer,sample,length);
				outputCount = outputCount + length;
				outputTail = outputBuffer + length;
			}
		}
	}
	
	//NSLog(@"Output Diff after: %d\n",(outputBuffer.buffer+bufferSize)-outputBuffer.tail);
	
	// Unlock the cyclic buffer
	[outputLock unlock];

}

-(void) queueInput: (uint32_t) length Sample: (uint8_t *) sample
{
	/**
	 *
	 * Add newest recording data to the queue
	 *
	 */
	
	if ( (inputCount + length) > bufferSize)
	{
		// Too many packets.. ignore
		//NSLog(@"CRITICAL ERROR:  OUTGOING AUDIO PACKETS HAVE EXCEEDED ALLOCATED BUFFER SPACE (%d BYTES) AND ARE BEING IGNORED\n", bufferSize);
	}
	else
	{
		if ( ((int)bufferSize - ((int)inputTail-(int)inputBuffer)) >= (int)length)
		{
			// We can fit all without looping
			memcpy(inputTail, sample, length);
			inputCount = inputCount + length;
			inputTail = inputTail + length;
		}
		else {
			// We need to loop tail around
			int lengthToEnd = (int)bufferSize - ((int)inputTail-(int)inputBuffer);
			if (lengthToEnd > 0) {
				// Put some at the end
				memcpy(inputTail,sample,lengthToEnd);
				int lengthRemaining = (int)length - lengthToEnd;
				memcpy(inputBuffer,sample+lengthToEnd,lengthRemaining);
				inputTail = inputBuffer + lengthRemaining;
				inputCount = inputCount + length;
			}
			else {
				// We're at the end, start from beginning
				memcpy(inputBuffer,sample,length);
				inputCount = inputCount + length;
				inputTail = inputBuffer + length;
			}

		}
		
	}
	
	//NSLog(@"Queue is now: %d\n",inputCount);
	//NSLog(@"Input Diff: %d\n",(inputBuffer+bufferSize)-inputTail);
	
	
	/**
	 *
	 * Then send off whatever we can
	 *
	 */
	//printf("PCM Length: %d\n", [self pcmLength]);
	
	if (inputCount >= [self pcmLength]) {
		// We have data to send
		if ( ((int)inputTail-(int)inputHead) > 0)
		{
			// There is no looping
			v3_send_audio(V3_AUDIO_SENDTYPE_U2CCUR, (uint32_t)audioFormat.mSampleRate, inputHead, pcmLength, 0);
			inputHead = inputHead + pcmLength;
			inputCount = inputCount - pcmLength;
			
		}
		else {
			// Tail <= Head  - Check to see if our data is split up
			int lengthToEnd = (int)bufferSize - ((int)inputHead-(int)inputBuffer);
			if (lengthToEnd < pcmLength )
			{
				// They are split.. copy to temp buffer
				memcpy(tempInput,inputHead,lengthToEnd);
				int lengthRemaining = (int)pcmLength - lengthToEnd;
				memcpy(tempInput+lengthToEnd,inputBuffer,lengthRemaining);
				v3_send_audio(V3_AUDIO_SENDTYPE_U2CCUR, (uint32_t)audioFormat.mSampleRate, tempInput, pcmLength, 0);
				inputHead = inputBuffer + lengthRemaining;
				inputCount = inputCount - pcmLength;
				
			}
			else {
				// Fill up all the way up
				v3_send_audio(V3_AUDIO_SENDTYPE_U2CCUR, (uint32_t)audioFormat.mSampleRate, inputHead, pcmLength, 0);
				inputHead = inputHead + pcmLength;
				inputCount = inputCount - pcmLength;
			}
			
		}
	}
	// else { we don't have enough for a packet }
	
}

#pragma mark -
#pragma mark API

-(Float64) rate
{
	return audioFormat.mSampleRate;
}

-(void) startAudioSession
{
	callInterrupt = NO;
	AudioSessionSetActive(YES);
}

-(void) stopAudioSession
{
	callInterrupt = YES;
	AudioOutputUnitStop(audioUnit);
	AudioSessionSetActive(NO);
	isOutput = NO;
	isRunning = NO;
	isRecording = NO;
}

- (void)killAudio
{
	AudioOutputUnitStop(audioUnit);
	AudioSessionSetActive(NO);
	isOutput = NO;
	isRunning = NO;
	isRecording = NO;
	
}

-(void) startOutput
{
	if (callInterrupt) {
		return;
	}
	if (queueOutputStop) {
		queueOutputStop = NO;
	}
	if (!isOutput) {
		isOutput = YES;
		if (!isRunning) {
			
			isRunning = YES;
			if(AudioOutputUnitStart(audioUnit) != noErr)
			{
				NSLog(@"Error starting output audio unit.\n");
			}
		}
	}
}

-(void) stopOutput
{
	if (isOutput) {
		if (outputCount > 0) {
			queueOutputStop = YES;
		}
		while (outputCount > 0 && queueOutputStop) {
			// Wait for audio unit to process what's left in the queue
			[NSThread sleepForTimeInterval:0.1];
		}
		if (queueOutputStop) {
			
			isOutput = NO;
			if (!isRecording) {
				if (isRunning) {
					
					isRunning = NO;
					if(AudioOutputUnitStop(audioUnit) != noErr)
					{
						NSLog(@"Error stopping output audio unit.\n");
					}
				}
				
			}
			
		}
	}
}

-(void) startRecording
{
	if (callInterrupt) {
		return;
	}
	if (!isRecording) {
		
		isRecording = YES;
		if (!isRunning) {
			isRunning = YES;
			if(AudioOutputUnitStart(audioUnit) != noErr)
			{
				NSLog(@"Error starting output audio unit.\n");
			}
		}
	}
}

-(void) stopRecording
{
	if (isRecording) {
		
		isRecording = NO;
		[inputLock lock];
		inputHead = inputTail;
		inputCount = 0;
		[inputLock unlock];
		if (!isOutput) {
			if (isRunning) {
				isRunning = NO;
				if(AudioOutputUnitStop(audioUnit) != noErr)
				{
					NSLog(@"Error stopping output audio unit.\n");
				}
			}
		}
	}
}

@end
