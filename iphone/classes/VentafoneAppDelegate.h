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
#import "SoundEffect.h"
#import "ServerSettings.h"
#import "Constants.h"
#import "MainViewController.h"
#import "Reachability.h"
#import "GlobalSettings.h"
#import "User.h"

@class MainViewController;
@class ServerSettings;
@class SoundEffect;
@class Reachability;
@class GlobalSettings;
@class User;

@interface VentafoneAppDelegate : NSObject <UIApplicationDelegate, UIAlertViewDelegate> {
    UIWindow *window;
	UINavigationController *navigationController;	// Added programmatically
	MainViewController *mainViewController;			// Added programmatically
	
	NSMutableDictionary *soundEffects;
	NSMutableArray *serverList;
	NSMutableDictionary *volumeRecords;
	GlobalSettings *globalSettings;
	
	Reachability* hostReach;
    Reachability* internetReach;
    //Reachability* wifiReach;
	
	BOOL interrupted;
	BOOL refreshEncoder;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet UINavigationController *navigationController;
@property (nonatomic, retain) IBOutlet MainViewController *mainViewController;
@property (nonatomic, retain) NSMutableDictionary *soundEffects;
@property (nonatomic, retain) NSMutableArray *serverList;
@property (nonatomic, retain) NSMutableDictionary *volumeRecords;
@property (nonatomic, retain) GlobalSettings *globalSettings;
@property (nonatomic, assign) BOOL refreshEncoder;
-(void) unsupportedCodec:(NSString *)codecName;
-(void) channelUnsupportedCodec:(NSString *)codecName;
-(void) disconnectWithHUDDisplay;
-(void) connectFailed:(NSString *)errorMessage;
-(void) joinChannelFailed:(NSString *)errorMessage;
-(void) serverDisconnected;
-(void) serverDisconnectedOnConnect;
-(void) addServer:(ServerSettings *)serverSettings;
- (void)addUserWithId:(NSInteger)uid alias:(NSString *)alias channelId:(NSInteger)cid;
- (void)addChannelWithId:(NSInteger)cid name:(NSString *)name parentId:(NSInteger)pid;
- (void)removeUserWithId:(NSInteger)uid;
- (void)removeChannelWithId:(NSInteger)cid;
- (void)moveUser:(NSInteger)uid toChannel:(NSInteger)cid;
- (uint16_t)channelIdForUser:(uint16_t)uid;
- (User *)userForUserId:(NSInteger)uid;
- (void)userTransmitting:(NSInteger)uid Status:(BOOL)isTransmit;
- (void)playSoundWithId:(NSString *)soundId;
- (void)saveServersToDisk;
- (void)saveGlobalSettingsToDisk;
- (void)saveVolumeRecordsToDisk;
- (BOOL)connectionAvailable;
- (BOOL)micAvailable;
- (void)updateVolumeRecordForUser:(NSInteger)uid;

@end

