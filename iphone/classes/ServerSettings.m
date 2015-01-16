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

#import "ServerSettings.h"


@implementation ServerSettings

@synthesize userAlias;
@synthesize hostAlias;
@synthesize hostAddress;
@synthesize hostPort;
@synthesize hostPassword;
@synthesize toggle;
@synthesize keyClicks;

-(void)printSettings
{
	NSLog(@"User alias: %@\n",userAlias);
	NSLog(@"Host alias: %@\n",hostAlias);
	NSLog(@"Host address: %@\n",hostAddress);
	NSLog(@"Host port: %@\n",hostPort);
	NSLog(@"Host password: %@\n",hostPassword);
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

}

- (void)encodeWithCoder:(NSCoder *)coder;
{
    [coder encodeObject:userAlias forKey:@"userAlias"];
    [coder encodeObject:hostAlias forKey:@"hostAlias"];
	[coder encodeObject:hostAddress forKey:@"hostAddress"];
	[coder encodeObject:hostPort forKey:@"hostPort"];
	[coder encodeObject:hostPassword forKey:@"hostPassword"];
	[coder encodeValueOfObjCType:@encode(BOOL) at:&toggle];
	[coder encodeValueOfObjCType:@encode(BOOL) at:&keyClicks];
}

- (id)initWithCoder:(NSCoder *)coder;
{
    if (self = [super init])
    {
        self.userAlias = [coder decodeObjectForKey:@"userAlias"];
        self.hostAlias = [coder decodeObjectForKey:@"hostAlias"];
		self.hostAddress = [coder decodeObjectForKey:@"hostAddress"];
		self.hostPort = [coder decodeObjectForKey:@"hostPort"];
		self.hostPassword = [coder decodeObjectForKey:@"hostPassword"];
		[coder decodeValueOfObjCType:@encode(BOOL) at:&toggle];
		[coder decodeValueOfObjCType:@encode(BOOL) at:&keyClicks];
    }   
    return self;
}

-(void)dealloc {
    [userAlias release];
	[hostAlias release];
	[hostAddress release];
	[hostPort release];
	[hostPassword release];
    [super dealloc];
}

@end
