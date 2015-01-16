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


@interface ServerSettings : NSObject < NSCoding >
{
	NSString *userAlias;
	NSString *hostAlias;
	NSString *hostAddress;
	NSString *hostPort;
	NSString *hostPassword;
	BOOL toggle;
	BOOL keyClicks;
}

@property (nonatomic, retain) NSString *userAlias;
@property (nonatomic, retain) NSString *hostAlias;
@property (nonatomic, retain) NSString *hostAddress;
@property (nonatomic, retain) NSString *hostPort;
@property (nonatomic, retain) NSString *hostPassword;
@property (nonatomic, assign) BOOL toggle;
@property (nonatomic, assign) BOOL keyClicks;
- (void)printSettings;

@end
