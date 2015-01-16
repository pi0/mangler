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
#import "User.h"
#import "Channel.h"

@class User;
@class Channel;

@interface TableItem : NSObject {

	User *user;
	Channel *channel;
	NSInteger level;
	NSInteger typeFlag;	// 0 - User, 1 - Channel
}

@property (nonatomic, retain) User *user;
@property (nonatomic, retain) Channel *channel;
@property (nonatomic, assign) NSInteger level;
@property (nonatomic, assign) NSInteger typeFlag;
-(id)initWithUser:(User *)aUser Level:(NSInteger)aLevel;
-(id)initWithChannel:(Channel *)aChannel Level:(NSInteger)aLevel;

@end
