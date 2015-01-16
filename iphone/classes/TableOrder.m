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

#import "TableOrder.h"
#import "User.h"
#import "Channel.h"
#import "TableItem.h"

@class User;
@class Channel;
@class TableItem;

@implementation TableOrder

@synthesize tableLayout;
@synthesize userList;
@synthesize channelList;

- (void)dealloc {
	if (self.tableLayout != nil) {
		[self.tableLayout release];
	}
    [super dealloc];
}

-(id)init
{
	if (self = [super init]) {
		
	}
	return self;
}

-(void)printLayout
{
	for (int i = 0; i<[self.tableLayout count]; i++) {
		TableItem *anItem = [self.tableLayout objectAtIndex:i];
		if ([anItem typeFlag] == 0) {
			// User
			NSLog(@"User: %@ with indent: %d\n",[[anItem user] userAlias],[anItem level] );
		}
		else {
			// Channel
			NSLog(@"Channel: %@ with indent: %d\n",[[anItem channel] channelName],[anItem level] );
		}

	}
}

-(void)createLayoutWithUsers:(NSMutableDictionary *)aUserList Channels:(NSMutableDictionary *)aChannelList
{
	// We need to refresh the current layout, so we will release it if it exists
	if (self.tableLayout != nil) {
		[self.tableLayout release];
		self.tableLayout = nil;
	}
	
	// Now we create our fresh layout
	self.tableLayout = [[NSMutableArray alloc] initWithCapacity:0];
	
	self.userList = aUserList;
	self.channelList = aChannelList;
	
	NSInteger indentLevel	= 0;
	NSInteger channelId		= 0;	// Root channel id is 0
	
	[self addForLevel:indentLevel ChannelId:channelId];
	
	//[self.userList release];
	//[self.channelList release];
}

-(void)addForLevel:(NSInteger)indentLevel ChannelId:(NSInteger)channelId
{
	if (self.userList != nil && self.channelList != nil) {
	
		NSArray *userKeys = [[self.userList allKeys] sortedArrayUsingSelector:@selector(compare:)];
		NSArray *channelKeys = [[self.channelList allKeys] sortedArrayUsingSelector:@selector(compare:)];
		
		// Check for users
		for (int i=0; i < [userKeys count]; i++) {
			User *currentUser = [self.userList objectForKey:[userKeys objectAtIndex:i]];
			if ([currentUser channelId] == channelId) {
				// Add user to table order
				TableItem *anItem = [[TableItem alloc] initWithUser:currentUser Level:indentLevel];
				[self.tableLayout addObject:anItem];
				//[anItem release];
				//NSLog(@">>> User %@ is indented: %d\n",[currentUser userAlias],indentLevel);
			}
			
		}
		for (int i =0; i < [channelKeys count]; i++) {
			Channel *currentChannel = [channelList objectForKey:[channelKeys objectAtIndex:i]];
			if ([currentChannel parentId] == channelId) {
				//NSLog(@">>> Channel %@ is indented: %d\n",[currentChannel channelName],indentLevel);
				TableItem *anItem = [[TableItem alloc] initWithChannel:currentChannel Level:indentLevel];
				[self.tableLayout addObject:anItem];
				//[anItem release];
				indentLevel++;
				[self addForLevel:indentLevel ChannelId:[currentChannel channelId]];
				indentLevel--;
			}
		}
		
	}
}

@end
