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

@class VentafoneAppDelegate;

@interface AddServerViewController : UIViewController <UITextFieldDelegate, UITableViewDelegate, UITableViewDataSource>
{
	BOOL movedUP;
	
	UIBarButtonItem *cancelButton;
	UIBarButtonItem *saveButton;
	UITableView	*myTableView;
	NSMutableArray *optionsData;
	NSInteger serverIndex;
	VentafoneAppDelegate *appDelegate;
}

@property (nonatomic, retain) IBOutlet UIBarButtonItem *cancelButton;
@property (nonatomic, retain) IBOutlet UIBarButtonItem *saveButton;
@property (nonatomic, retain) NSMutableArray *optionsData;
@property (nonatomic, assign) NSInteger serverIndex;
@property (nonatomic, retain) IBOutlet UITableView *myTableView;
- (IBAction)cancelAction:(id)sender;
- (IBAction)saveAction:(id)sender;

@end
