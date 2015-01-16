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


@interface Channel : NSObject {
	NSInteger channelId;
	NSString *channelName;
	NSInteger parentId;
}

@property (nonatomic, assign) NSInteger channelId;
@property (nonatomic, retain) NSString *channelName;
@property (nonatomic, assign) NSInteger parentId;
-(id)initWithChannelId:(NSInteger)cid name:(NSString *)name parentId:(NSInteger)pid;

@end
