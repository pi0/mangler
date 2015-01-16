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
#import "EventService.h"
#import "ServerSettings.h"
#import "TableOrder.h"
#import "MBProgressHUD.h"
#import "UserViewController.h"
#import "User.h"

@class EventService;
@class ServerSettings;
@class TableOrder;
@class UserViewController;
@class User;

@interface ServerViewController : UIViewController <UITableViewDelegate, UITableViewDataSource,
													MBProgressHUDDelegate, UIAlertViewDelegate>
{
	BOOL isRecording;
	BOOL recordingToggle;
	BOOL connected;
	BOOL pushFinished;
	
	NSInteger serverIndex;
	NSInteger rowHeight;
	UITableView	*myTableView;
	NSMutableDictionary *channelList;
	NSMutableDictionary *userList;
	
	EventService *eventService;
	TableOrder *tableOrder;
	UserViewController *myUserViewController;
	
	MBProgressHUD *HUD;
	UITextField *passwordField;
	VentafoneAppDelegate *appDelegate;
	IBOutlet UIImageView *recordingBackground;
	IBOutlet UIButton *recordingButton;
}

@property (nonatomic, assign) BOOL isRecording;
@property (nonatomic, assign) BOOL connected;
@property (nonatomic, assign) BOOL pushFinished;
@property (nonatomic, assign) BOOL smallRowHeight;
@property (nonatomic, retain) EventService *eventService;
@property (nonatomic, assign) NSInteger serverIndex;
@property (nonatomic, assign) NSInteger rowHeight;
@property (nonatomic, retain) IBOutlet UITableView *myTableView;
@property (nonatomic, retain) NSMutableDictionary *channelList;
@property (nonatomic, retain) NSMutableDictionary *userList;
@property (nonatomic, retain) TableOrder *tableOrder;
@property (nonatomic, retain) IBOutlet UIImageView *recordingBackground;
@property (nonatomic, retain) IBOutlet UIButton *recordingButton;
@property (nonatomic, retain) UserViewController *myUserViewController;
- (id)initWithServerSettingsIndex:(NSInteger)aServerIndex;
- (void)addUserWithId:(NSInteger)uid alias:(NSString *)alias channelId:(NSInteger)cid;
- (void)addChannelWithId:(NSInteger)cid name:(NSString *)name parentId:(NSInteger)pid;
- (void)removeUserWithId:(NSInteger)uid;
- (void)removeChannelWithId:(NSInteger)cid;
- (void)moveUser:(NSInteger)uid toChannel:(NSInteger)cid;
- (uint16_t)channelIdForUser:(uint16_t)uid;
- (User *)userForUserId:(NSInteger)uid;
- (void)userTransmitting:(NSInteger)uid Status:(BOOL)isTransmit;
- (IBAction)startRecording:(id)sender;
- (IBAction)stopRecording:(id)sender;
- (IBAction)showInfo;
- (void)connectTask;
- (void)disconnectTask;
- (void)connectToServer;
- (void)disconnectFromServer;
- (IBAction)disconnectAction;
- (void)reloadData;

@end
