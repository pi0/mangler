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


@interface User : NSObject
{
	NSInteger userId;
	NSString *userAlias;
	NSInteger channelId;
	BOOL transmitting;
	BOOL muted;
}

@property (nonatomic, assign) NSInteger userId;
@property (nonatomic, retain) NSString *userAlias;
@property (nonatomic, assign) NSInteger channelId;
@property (nonatomic, assign) BOOL transmitting;
@property (nonatomic, assign) BOOL muted;
-(id)initWithUserId:(NSInteger)uid alias:(NSString *)alias channelId:(NSInteger)cid;

@end
