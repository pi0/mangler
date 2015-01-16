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

#import "VolumeRecord.h"


@implementation VolumeRecord

@synthesize userAlias;
@synthesize volume;
@synthesize muted;

-(id)init
{
    if (self = [super init])
    {
		muted = NO;
    }
    return self;
}

-(void)printSettings
{
	NSLog(@"User alias: %@\n",userAlias);
	NSLog(@"User volume: %d\n",volume);
	if (muted) {
		NSLog(@"Muted: ON\n");
	}
	else
	{
		NSLog(@"Muted: OFF\n");
	}
}

- (void)encodeWithCoder:(NSCoder *)coder;
{
	[coder encodeObject:userAlias forKey:@"userAlias"];
	[coder encodeValueOfObjCType:@encode(NSInteger) at:&volume];
	[coder encodeValueOfObjCType:@encode(BOOL) at:&muted];
}

- (id)initWithCoder:(NSCoder *)coder;
{
    if (self = [super init])
    {
		self.userAlias = [coder decodeObjectForKey:@"userAlias"];
		[coder decodeValueOfObjCType:@encode(NSInteger) at:&volume];
		[coder decodeValueOfObjCType:@encode(BOOL) at:&muted];
    }   
    return self;
}

-(void)dealloc {
    [super dealloc];
}

@end
