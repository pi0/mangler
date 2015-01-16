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
#import "SettingsViewController.h"
#import "AddServerViewController.h"
#import "ServerViewController.h"
#import "AboutViewController.h"
#import "Constants.h"
#import "VentafoneAppDelegate.h"
#import "MBProgressHUD.h"

@class VentafoneAppDelegate;
@class SettingsViewController;
@class AddServerViewController;
@class ServerViewController;
@class AboutViewController;

@interface MainViewController : UIViewController <UINavigationBarDelegate, UITableViewDelegate,
												  UITableViewDataSource, MBProgressHUDDelegate>
{
	UITableView	*myTableView;
	MBProgressHUD *HUD;
	
	SettingsViewController *mySettingsViewController;
	AddServerViewController *myAddServerViewController;
	ServerViewController *myServerViewController;
	AboutViewController *myAboutViewController;
	
	VentafoneAppDelegate *appDelegate;
}

@property (nonatomic, retain) IBOutlet UITableView *myTableView;
@property (nonatomic, retain) IBOutlet SettingsViewController *mySettingsViewController;
@property (nonatomic, retain) IBOutlet AddServerViewController *myAddServerViewController;
@property (nonatomic, retain) IBOutlet ServerViewController *myServerViewController;
@property (nonatomic, retain) IBOutlet AboutViewController *myAboutViewController;
- (void) presentAddServerViewControllerWithServerIndex:(NSInteger)aServerIndex;
- (void) disconnectTask;
- (void) disconnectWithHUDDisplay;
- (IBAction)settingsViewAction;
- (IBAction)addServerViewAction;
- (IBAction)editingAction;
- (IBAction)showInfo;
- (void)reloadTable;


@end
