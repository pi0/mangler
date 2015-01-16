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

#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AudioUnit.h>
#import "ventrilo3.h"

@interface AudioController : NSObject
{
	AudioStreamBasicDescription audioFormat;
	AudioUnit audioUnit;
	AURenderCallbackStruct inputCallbackStruct;
	AURenderCallbackStruct outputCallbackStruct;
	AudioBufferList *silenceBuffer;
	AudioBufferList *tempBuffer;					// Used when cyclic buffer loops
	
	// State information
	BOOL isRecording;
	BOOL isRunning;
	BOOL isOutput;
	BOOL queueOutputStop;
	BOOL callInterrupt;
	
	// Cyclic buffers
	uint8_t *outputBuffer;		// Header of buffer alloc
	uint8_t *outputHead;		// Current head of buffer
	uint8_t *outputTail;		// Current tail of buffer
	uint32_t outputCount;		// Count of how many bytes currently in buffer
	uint8_t *inputBuffer;
	uint8_t *inputHead;
	uint8_t *inputTail;
	uint32_t inputCount;
	uint8_t *tempInput;
	
	// Locks
	NSLock *outputLock;
	NSLock *inputLock;
	
	uint32_t pcmLength;
}

@property (nonatomic, assign) AudioUnit audioUnit;
@property (nonatomic, assign) AudioBufferList *silenceBuffer;
@property (nonatomic, assign) AudioBufferList *tempBuffer;
@property (nonatomic, assign) BOOL isRecording;
@property (nonatomic, assign) BOOL isRunning;
@property (nonatomic, assign) BOOL isOutput;
@property (nonatomic, assign) BOOL queueOutputStop;
@property (nonatomic, assign) BOOL callInterrupt;
@property (nonatomic, assign) uint8_t *outputBuffer;
@property (nonatomic, assign) uint8_t *outputHead;
@property (nonatomic, assign) uint8_t *outputTail;
@property (nonatomic, assign) uint32_t outputCount;
@property (nonatomic, assign) uint8_t *inputBuffer;
@property (nonatomic, assign) uint8_t *inputHead;
@property (nonatomic, assign) uint8_t *inputTail;
@property (nonatomic, assign) uint32_t inputCount;
@property (nonatomic, retain) NSLock *outputLock;
@property (nonatomic, retain) NSLock *inputLock;
@property (nonatomic, assign) uint32_t pcmLength;
- (id)initWithRate: (uint32_t) rate;
- (Float64)rate;
- (void)startAudioSession;
- (void)stopAudioSession;
- (void)killAudio;
- (void)startOutput;
- (void)stopOutput;
- (void)startRecording;
- (void)stopRecording;
- (void)queueOutput:(uint32_t)length Sample:(uint8_t *)sample;
- (void)queueInput:(uint32_t)length Sample:(uint8_t *) sample;

@end
