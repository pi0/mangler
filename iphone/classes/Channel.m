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

#import "Channel.h"


@implementation Channel

@synthesize channelId;
@synthesize channelName;
@synthesize parentId;

-(void)dealloc {
	[channelName release];
    [super dealloc];
}

-(id)initWithChannelId:(NSInteger)cid name:(NSString *)name parentId:(NSInteger)pid
{
	if (self = [super init]) {
		channelId = cid;
		self.channelName = name;
		parentId = pid;
	}
	return self;
}

@end
