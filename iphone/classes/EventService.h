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
#import "ventrilo3.h"
#import <stdio.h>
#import <stdlib.h>
#import <unistd.h>
#import <signal.h>
#import <strings.h>
#import <sys/types.h>
#import <string.h>
#import <getopt.h>
#import "AudioController.h"
#import "VentafoneAppDelegate.h"
#import "ServerSettings.h"

@class VentafoneAppDelegate;

@interface EventService : NSObject {
	BOOL running;
	BOOL recording;
	BOOL finished;
	BOOL connectSuccess;
	BOOL loginComplete;
	AudioController *audioController;
	VentafoneAppDelegate *appDelegate;
	NSInteger serverIndex;
	NSString *errorMessage;
}

@property (nonatomic, retain) AudioController *audioController;
@property (nonatomic, assign) BOOL finished;
@property (nonatomic, assign) BOOL connectSuccess;
@property (nonatomic, assign) BOOL loginComplete;
@property (nonatomic, assign) NSInteger serverIndex;
@property (nonatomic, retain) NSString *errorMessage;
- (id)initWithServerSettingsIndex:(NSInteger)aServerIndex;
- (void)startService;
- (void)stopService;
- (void)killService;
- (void)startRecording;
- (void)stopRecording;

@end
