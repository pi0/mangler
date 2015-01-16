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

#import "GlobalSettings.h"

@implementation GlobalSettings

@synthesize toggle;
@synthesize keyClicks;
@synthesize soundEffects;
@synthesize recordOverride;
@synthesize smallRowHeight;

-(id)init
{
    if (self = [super init])
    {
		toggle = NO;
		keyClicks = YES;
		soundEffects = YES;
		recordOverride = NO;
		smallRowHeight = NO;
    }
    return self;
}

-(void)printSettings
{
	if (toggle) {
		NSLog(@"Toggle: ON\n");
	}
	else
	{
		NSLog(@"Toggle: OFF\n");
	}
	if (keyClicks) {
		NSLog(@"Key clicks: ON\n");
	}
	else {
		NSLog(@"Key clicks: OFF\n");
	}
	if (soundEffects) {
		NSLog(@"Sound effects: ON\n");
	}
	else {
		NSLog(@"Sound effects: OFF\n");
	}
	if (recordOverride) {
		NSLog(@"Record override: ON\n");
	}
	else {
		NSLog(@"Record override: OFF\n");
	}
	if (smallRowHeight) {
		NSLog(@"Small row height: ON\n");
	}
	else {
		NSLog(@"Small row height: OFF\n");
	}
	
}

- (void)encodeWithCoder:(NSCoder *)coder;
{
	[coder encodeValueOfObjCType:@encode(BOOL) at:&toggle];
	[coder encodeValueOfObjCType:@encode(BOOL) at:&keyClicks];
	[coder encodeValueOfObjCType:@encode(BOOL) at:&soundEffects];
	[coder encodeValueOfObjCType:@encode(BOOL) at:&recordOverride];
	[coder encodeValueOfObjCType:@encode(BOOL) at:&smallRowHeight];
}

- (id)initWithCoder:(NSCoder *)coder;
{
    if (self = [super init])
    {
		[coder decodeValueOfObjCType:@encode(BOOL) at:&toggle];
		[coder decodeValueOfObjCType:@encode(BOOL) at:&keyClicks];
		[coder decodeValueOfObjCType:@encode(BOOL) at:&soundEffects];
		[coder decodeValueOfObjCType:@encode(BOOL) at:&recordOverride];
		[coder decodeValueOfObjCType:@encode(BOOL) at:&smallRowHeight];
    }   
    return self;
}

-(void)dealloc {
    [super dealloc];
}

@end
