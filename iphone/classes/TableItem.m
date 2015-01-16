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

#import "TableItem.h"

@implementation TableItem

@synthesize user;
@synthesize channel;
@synthesize level;
@synthesize typeFlag;

- (void)dealloc {
	if (self.user != nil) {
		[self.user release];
	}
	if (self.channel != nil) {
		[self.channel release];
	}
    [super dealloc];
}

-(id)initWithUser:(User *)aUser Level:(NSInteger)aLevel
{
	if (self = [super init]) {
		self.user = aUser;
		typeFlag = 0;
		level = aLevel;
	}
	return self;
}

-(id)initWithChannel:(Channel *)aChannel Level:(NSInteger)aLevel
{
	if (self = [super init]) {
		self.channel = aChannel;
		typeFlag = 1;
		level = aLevel;
	}
	return self;
}

@end
