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

#import <UIKit/UIKit.h>
#import "VentafoneAppDelegate.h"
#import "VolumeRecord.h"
#import "ServerViewController.h"

@class VolumeRecord;
@class ServerViewController;

@interface UserViewController : UIViewController {

	NSInteger uid;
	UINavigationItem *navItem;
	UITableView	*myTableView;
	NSMutableArray *optionsData;
	
	VentafoneAppDelegate *appDelegate;
	
}

@property (nonatomic, assign) NSInteger uid;
@property (nonatomic, retain) IBOutlet UINavigationItem *navItem;
@property (nonatomic, retain) NSMutableArray *optionsData;
@property (nonatomic, retain) IBOutlet UITableView *myTableView;
- (IBAction)doneAction:(id)sender;

@end
