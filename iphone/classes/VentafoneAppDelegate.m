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

#import "VentafoneAppDelegate.h"
#import "ServerViewController.h"
#import "MainViewController.h"

@class MainViewController;
@class ServerViewController;

@implementation VentafoneAppDelegate

@synthesize window;
@synthesize navigationController;
@synthesize mainViewController;
@synthesize soundEffects;
@synthesize serverList;
@synthesize volumeRecords;
@synthesize globalSettings;
@synthesize refreshEncoder;

BOOL alertedOnConnection;	// Used so that the user isn't alerted multiple times

#pragma mark -
#pragma mark Audio Session Callbacks

void rioInterruptionListener(void* inUserData, UInt32 interruptionState)
{
	VentafoneAppDelegate *thisDelegate = (VentafoneAppDelegate *) inUserData;
	ServerViewController *theServerViewController = [[thisDelegate mainViewController] myServerViewController];
	
	if (theServerViewController != nil) {
		EventService *theEventService = [theServerViewController eventService];
		if (theEventService != nil) {
			
			AudioController *theAudioController = [theEventService audioController];
			
			if (theAudioController != nil) {
				if (interruptionState == kAudioSessionBeginInterruption)
				{
					[theAudioController stopAudioSession];
				}
				else if (interruptionState == kAudioSessionEndInterruption) {
					[theAudioController startAudioSession];
				}
			}
			
			
			UInt32 audioInputIsAvailable;
			UInt32 propertySize = sizeof (audioInputIsAvailable);
			AudioSessionGetProperty (
									 kAudioSessionProperty_AudioInputAvailable,
									 &propertySize,
									 &audioInputIsAvailable // A nonzero value on output means that
									 // audio input is available
									 );
			if (!audioInputIsAvailable) {
				// Check to see if user was connected
				if ([theServerViewController connected]) {
					// User is connected and microphone was removed.. disconnect
					UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:@"Microphone not available" message:@"Please connect a microphone and try again." delegate:thisDelegate cancelButtonTitle:nil otherButtonTitles:@"OK", nil];
					[myAlertView show];
					[myAlertView release];
					[theServerViewController setConnected:NO];
					while (![theServerViewController pushFinished]) {
						[NSThread sleepForTimeInterval:0.1];
					}
					[thisDelegate.navigationController popToRootViewControllerAnimated:YES];
					//[thisDelegate.mainViewController viewWillAppear:YES];
					[thisDelegate disconnectWithHUDDisplay];
				}
			}
		}
	}
}

#pragma mark -
#pragma mark Application lifecycle

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {    

	/**
	 *	Step 1. Initialize sound effects
	 */
	NSBundle *mainBundle = [NSBundle mainBundle];
	self.soundEffects = [[NSMutableDictionary alloc] initWithCapacity:0];
	SoundEffect *connectSound = [[SoundEffect alloc] initWithContentsOfFile:[mainBundle pathForResource:@"login" ofType:@"caf"]];
	SoundEffect *disconnectSound = [[SoundEffect alloc] initWithContentsOfFile:[mainBundle pathForResource:@"logout" ofType:@"caf"]];
	/*SoundEffect *channelSound = [[SoundEffect alloc] initWithContentsOfFile:[mainBundle pathForResource:@"Channel" ofType:@"caf"]];*/
	SoundEffect *channelJoinSound = [[SoundEffect alloc] initWithContentsOfFile:[mainBundle pathForResource:@"channelenter" ofType:@"caf"]];
	SoundEffect *channelLeaveSound = [[SoundEffect alloc] initWithContentsOfFile:[mainBundle pathForResource:@"channelleave" ofType:@"caf"]];
	/*SoundEffect *userConnectSound = [[SoundEffect alloc] initWithContentsOfFile:[mainBundle pathForResource:@"UserConnect" ofType:@"caf"]];
	SoundEffect *userDisconnectSound = [[SoundEffect alloc] initWithContentsOfFile:[mainBundle pathForResource:@"UserDisconnect" ofType:@"caf"]];*/
	SoundEffect *micKeyDownSound = [[SoundEffect alloc] initWithContentsOfFile:[mainBundle pathForResource:@"talkstart" ofType:@"caf"]];
	SoundEffect *micKeyUpSound = [[SoundEffect alloc] initWithContentsOfFile:[mainBundle pathForResource:@"talkend" ofType:@"caf"]];
	
	/*
		All of our sound effects are added to a dictionary for easy code re-use on playing back specific sounds.
	 */
	[self.soundEffects setObject:connectSound forKey:@"connect"];
	[self.soundEffects setObject:disconnectSound forKey:@"disconnect"];
	/*[self.soundEffects setObject:channelSound forKey:@"Channel"];*/
	[self.soundEffects setObject:channelJoinSound forKey:@"ChannelJoin"];
	[self.soundEffects setObject:channelLeaveSound forKey:@"ChannelLeave"];
	/*[self.soundEffects setObject:userConnectSound forKey:@"UserConnect"];
	[self.soundEffects setObject:userDisconnectSound forKey:@"UserDisconnect"];*/
	[self.soundEffects setObject:micKeyDownSound forKey:@"MicKeyDown"];
	[self.soundEffects setObject:micKeyUpSound forKey:@"MicKeyUp"];
	
	[connectSound release];
	[disconnectSound release];
	/*[channelSound release];*/
	[channelJoinSound release];
	[channelLeaveSound release];
	/*[userConnectSound release];
	[userDisconnectSound release];*/
	[micKeyDownSound release];
	[micKeyUpSound release];
	
	/**
	 *	Step 2. Initialize main view controller
	 */
	MainViewController *aMainViewController = [[MainViewController alloc] initWithNibName:
																				   NSStringFromClass([MainViewController class])
																				   bundle:mainBundle];
	[self setMainViewController:aMainViewController];
	[aMainViewController release];
	

	/**
	 *	Step 3. Initialize navigation bar controller
	 */
	UINavigationController *aNavController = [[UINavigationController alloc] initWithRootViewController:mainViewController];
	[self setNavigationController:aNavController];
	[aNavController release];
	
	/**
	 *	Step 4. Retrieve server list from disk if exists
	 */
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	NSString *serverListFile = [NSString stringWithFormat:@"%@/serverListFile", documentsDirectory];
	NSMutableArray *serversFromFile = [NSKeyedUnarchiver unarchiveObjectWithFile:serverListFile];
	if ([serversFromFile count] > 0)
	{
		// Server list is already created, load it from the disk
		[self setServerList:serversFromFile];
	}
	else
	{
		// First time being used, lets create an empty one
		self.serverList = [NSMutableArray array];
	}
	//[serversFromFile release];
	
//	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
//	NSMutableData *data = [defaults objectForKey:@"servers"];
//	if (data != nil) {
//		NSMutableArray *serversFromFile = [NSKeyedUnarchiver unarchiveObjectWithData:data];
//		if (serversFromFile != nil) {
//			NSLog(@"Retrieved servers from file.\n");
//			self.serverList = serversFromFile;
//		}
//		else {
//			NSLog(@"Creating new server array, unarchive failed.\n");
//			self.serverList = [[NSMutableArray alloc] init];
//		}
//
//	}
//	else
//	{
//		NSLog(@"Creating new server array, nothing existed.\n");
//		self.serverList = [[NSMutableArray alloc] init];
//	}

	
//	[navigationController pushViewController:self.mainViewController animated:NO];
//	[navigationController initWithRootViewController:self.mainViewController];
	
	
	/**
	 *  Step 5. Retrieve global settings from disk if exists
	 */
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	NSMutableData *data = [defaults objectForKey:@"globalSettings"];
	if (data != nil) {
		GlobalSettings *globalSettingsFromFile = [NSKeyedUnarchiver unarchiveObjectWithData:data];
		if (globalSettingsFromFile != nil) {
			self.globalSettings = globalSettingsFromFile;
		}
		else {
			self.globalSettings = [[GlobalSettings alloc] init];
			[self saveGlobalSettingsToDisk];
		}

	}
	else
	{
		self.globalSettings = [[GlobalSettings alloc] init];
		[self saveGlobalSettingsToDisk];
	}
	
	/**
	 *  Step 6. Retrieve volume record dictionary from disk if exists
	 */
	data = [defaults objectForKey:@"volumeRecords"];
	if (data != nil) {
		// Volume records found
		NSMutableDictionary *volumeRecordsFromFile = [NSKeyedUnarchiver unarchiveObjectWithData:data];
		if (volumeRecordsFromFile != nil) {
			self.volumeRecords = volumeRecordsFromFile;
		}
		else {
			// Error retrieving volume records, make new dictionary and save
			self.volumeRecords = [[NSMutableDictionary alloc] initWithCapacity:0];
			data = [NSKeyedArchiver archivedDataWithRootObject:self.volumeRecords];
			[defaults setObject:data forKey:@"volumeRecords"];
			[defaults synchronize];
		}
	}
	else {
		// None found, start an empty dictionary
		self.volumeRecords = [[NSMutableDictionary alloc] initWithCapacity:0];
		// And save them to disk
		data = [NSKeyedArchiver archivedDataWithRootObject:self.volumeRecords];
		[defaults setObject:data forKey:@"volumeRecords"];
		[defaults synchronize];
	}

	
	/**
	 *	Step 7. Setup reachability observer
	 */
	
	[[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(reachabilityChanged:) name: kReachabilityChangedNotification object: nil];
	alertedOnConnection = NO;
	hostReach = [[Reachability reachabilityWithHostName: @"www.apple.com"] retain];
	[hostReach startNotifier];
	//[self alertOnReachability: hostReach];
	
//    internetReach = [[Reachability reachabilityForInternetConnection] retain];
//	[internetReach startNotifier];
//	[self alertOnReachability: internetReach];
	
	
	/**
	 *	Step 8. Setup navigation controller and present
	 */
	
	//NSArray* controllers = [NSArray arrayWithObjects:self.mainViewController, nil];
	//[navigationController pushViewController:mainViewController animated:YES];
	//navigationController.viewControllers = controllers;
	
	// Add the navigation controller's view to the window
	[window addSubview:[navigationController view]];
	[window makeKeyAndVisible];
	
	UIApplication *thisApp = [UIApplication sharedApplication];
	thisApp.idleTimerDisabled = YES;
	
	interrupted = NO;
	refreshEncoder = NO;
	
	AudioSessionInitialize(NULL,NULL,rioInterruptionListener,self);
	AudioSessionSetActive(NO);
	
	return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
//    /*
//     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
//     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
//     */
	
	/*
	 Dear Apple,
	 
	 We will stop all threads and return to the main screen on de-activation.  We just disconnect the user.
	 Perhaps in a future release we can allow our users to continue communication while the screen is locked.
	 */
	//NSLog(@"Application will resign active");
	ServerViewController *theServerViewController = [mainViewController myServerViewController];
	if (theServerViewController != nil) {
		if ([theServerViewController connected])
		{
			if ([theServerViewController isRecording]) {
				[theServerViewController recordingButton].selected = NO;
				[theServerViewController setIsRecording:NO];
				[[theServerViewController recordingBackground] setHidden:YES];
				[[theServerViewController eventService] stopRecording];
			}
			AudioController *theAudioController = [[theServerViewController eventService] audioController];
			if (theAudioController != nil) {
				interrupted = YES;
				[theAudioController stopAudioSession];
			}
			
			//NSLog(@"We will disconnect for them.");
			// If user locked their phone, we will automatically disconnect for them to save resources
			//[self.navigationController popViewControllerAnimated:NO];
			
			//		ServerViewController *theServerViewController = [mainViewController myServerViewController];
			//		[[theServerViewController eventService] killService];
			//		[theServerViewController setConnected:NO];
		}
	}
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
//    /*
//     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
//     If your application supports background execution, called instead of applicationWillTerminate: when the user quits.
//     */
	
	/*
	 Dear Apple,
	 
	 We have set the application property UIApplicationExitsOnSuspend to YES for this application.
	 We are disconnecting and finishing all processing threads and therefor
	 do not need any background processing at this time.
	 */
	
	//NSLog(@"Application did enter background");
	ServerViewController *theServerViewController = [mainViewController myServerViewController];
	if (theServerViewController != nil) {
		if ([theServerViewController connected])
		{
			// If user moved this app to the background, we will automatically disconnect for them before termination of the application
			//NSLog(@"We will disconnect for them.");
			[[theServerViewController eventService] killService];
			[theServerViewController setConnected:NO];
		}
	}
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
	//NSLog(@"Application will enter foreground");
    /*
     Called as part of  transition from the background to the inactive state: here you can undo many of the changes made on entering the background.
     */
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
	//NSLog(@"Application did become active");
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
     */
	
	if (interrupted)
	{
		ServerViewController *theServerViewController = [mainViewController myServerViewController];
		if (theServerViewController != nil) {
			if ([theServerViewController connected])
			{
				AudioController *theAudioController = [[theServerViewController eventService] audioController];
				if (theAudioController != nil) {
					interrupted = NO;
					[theAudioController startAudioSession];
				}
				
				//		NSLog(@"We do have server window up still..\n");
				//		// User is coming back to the server window after having been disconnected by us,
				//		// so we will bring them back to server selection window
				//		ServerViewController *theServerViewController = [mainViewController myServerViewController];
				//		EventService *eventService = [theServerViewController eventService];
				//		[self.navigationController popViewControllerAnimated:NO];
				//		while (![eventService finished]) {
				//			NSLog(@"VentafoneAppDelegate >>> (applicationDidBecomeActive) >>> waiting for eventservice to finish");
				//			[NSThread sleepForTimeInterval:0.1];
				//		}
				//		[theServerViewController release];
				//		[mainViewController setMyServerViewController:nil];
			}
		}
	}
}

- (void)applicationWillTerminate:(UIApplication *)application {
    /*
     Called when the application is about to terminate.
     See also applicationDidEnterBackground:.
     */
	//NSLog(@">>> Application will terminate!");
}

#pragma mark -
#pragma mark Reachability

//Called by Reachability whenever status changes.
- (void) reachabilityChanged: (NSNotification* )note
{
	// Check to see if user was connected
	ServerViewController *theServerViewController = [mainViewController myServerViewController];
	if ([theServerViewController connected]) {
		while (![theServerViewController pushFinished]) {
			[NSThread sleepForTimeInterval:0.1];
		}
		[self.navigationController popToRootViewControllerAnimated:YES];
		[theServerViewController setConnected:NO];
		[self disconnectWithHUDDisplay];
	}
//	if ([[self.navigationController viewControllers] count] > 1)
//	{
//		// User is connected and network changed.. disconnect
//		[self.navigationController popViewControllerAnimated:NO];	
//	}

	Reachability* curReach = [note object];
	NSParameterAssert([curReach isKindOfClass: [Reachability class]]);
	[self alertOnReachability: curReach];
}

- (BOOL) alertOnReachability: (Reachability*) curReach
{
    NetworkStatus netStatus = [curReach currentReachabilityStatus];
	BOOL connectionRequired = [curReach connectionRequired];
	
	if ( netStatus == ReachableViaWiFi )
	{
		if (connectionRequired) {
			// User intervention required to access network
			if (!alertedOnConnection) {
				// The user does not have an alert in view
				alertedOnConnection = YES;
				UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:@"Could not connect to the internet" message:@"Waiting for connection to be acquired, please verify Wi-Fi settings are correct and try again." delegate:self cancelButtonTitle:nil otherButtonTitles:@"OK", nil];
				[myAlertView show];
				[myAlertView release];
			}
			return NO;
		}
		// else { // User is connected fine }
	}
	else if ( netStatus == ReachableViaWWAN )
	{
		if (connectionRequired) {
			// Possible user intervention required to access network
			if (!alertedOnConnection) {
				// The user does not have an alert in view
				alertedOnConnection = YES;
				UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:@"Could not connect to the internet" message:@"Waiting for connection to be acquired, check that your carrier service is available or try again." delegate:self cancelButtonTitle:nil otherButtonTitles:@"OK", nil];
				[myAlertView show];
				[myAlertView release];
			}
			return NO;
		}
		// else { // User is connected fine }
	}
	else {
		// User needs to connect turn on cellular data or Wifi
		if (!alertedOnConnection) {
			// The user does not have an alert in view
			alertedOnConnection = YES;
			UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:@"Could not connect to the internet" message:@"Access not available, please turn on Wi-Fi or enable cellular data." delegate:self cancelButtonTitle:nil otherButtonTitles:@"OK", nil];
			[myAlertView show];
			[myAlertView release];
		}
		return NO;
	}
	return YES;
}

#pragma mark -
#pragma mark UIAlertViewDelegate methods

- (void)alertView:(UIAlertView *)alertView willDismissWithButtonIndex:(NSInteger)buttonIndex
{
	// Clear for future alerts
	alertedOnConnection = NO;
}

#pragma mark -
#pragma mark Global methods

-(void) unsupportedCodec:(NSString *)codecName
{
	[codecName retain];
	NSString *errorMessage = [NSString stringWithFormat:@"This server uses an unsupported codec: %@",codecName];
	[codecName release];
	UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:@"Unsupported codec" message:errorMessage delegate:self cancelButtonTitle:nil otherButtonTitles:@"OK", nil];
	[myAlertView show];
	[myAlertView release];
	ServerViewController *theServerViewController = [mainViewController myServerViewController];
	while (![theServerViewController pushFinished]) {
		[NSThread sleepForTimeInterval:0.1];
	}
	[self.navigationController popToRootViewControllerAnimated:YES];
	//[self.mainViewController viewWillAppear:YES];
	if ([theServerViewController connected]) {
		[theServerViewController setConnected:NO];
		[self disconnectWithHUDDisplay];
	}
}

-(void) channelUnsupportedCodec:(NSString *)codecName
{
	[codecName retain];
	NSString *errorMessage = [NSString stringWithFormat:@"This channel uses an unsupported codec: %@",codecName];
	[codecName release];
	UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:@"Unsupported codec" message:errorMessage delegate:self cancelButtonTitle:nil otherButtonTitles:@"OK", nil];
	[myAlertView show];
	[myAlertView release];
}

-(void) disconnectWithHUDDisplay
{
	//NSLog(@"VentafoneAppDelegate >>> (disconnectWithHUDDisplay) >>> entering");
	[mainViewController disconnectWithHUDDisplay];
}

-(void) connectFailed:(NSString *)errorMessage
{
	//@"Please verify that you are connected to a network, verify the connection settings, and try again"
	[errorMessage retain];
	UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:@"Could not connect to server" message:errorMessage delegate:self cancelButtonTitle:nil otherButtonTitles:@"OK", nil];
	[myAlertView show];
	[myAlertView release];
	[errorMessage release];
	ServerViewController *theServerViewController = [mainViewController myServerViewController];
	NSInteger serverIndex = [theServerViewController serverIndex];
	while (![theServerViewController pushFinished]) {
		[NSThread sleepForTimeInterval:0.1];
	}
	[self.navigationController popToRootViewControllerAnimated:YES];
	//[self.mainViewController viewWillAppear:YES];
	if ([theServerViewController connected]) {
		[theServerViewController setConnected:NO];
		[self disconnectWithHUDDisplay];
	}
	[mainViewController presentAddServerViewControllerWithServerIndex:serverIndex];
}

-(void) joinChannelFailed:(NSString *)errorMessage
{
	[errorMessage retain];
	UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:@"Could not join channel" message:errorMessage delegate:self cancelButtonTitle:nil otherButtonTitles:@"OK", nil];
	[myAlertView show];
	[myAlertView release];
	[errorMessage release];
}

-(void) serverDisconnected
{
	UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:@"Server disconnected" message:@"Remote host closed the connection" delegate:self cancelButtonTitle:nil otherButtonTitles:@"OK", nil];
	[myAlertView show];
	[myAlertView release];
	ServerViewController *theServerViewController = [mainViewController myServerViewController];
	while (![theServerViewController pushFinished]) {
		[NSThread sleepForTimeInterval:0.1];
	}
	[self.navigationController popToRootViewControllerAnimated:YES];
	//[self.mainViewController viewWillAppear:YES];
	if ([theServerViewController connected]) {
		[theServerViewController setConnected:NO];
		[self disconnectWithHUDDisplay];
	}
}

-(void) serverDisconnectedOnConnect
{
	UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:@"Server disconnected" message:@"Remote host closed the connection" delegate:self cancelButtonTitle:nil otherButtonTitles:@"OK", nil];
	[myAlertView show];
	[myAlertView release];
	ServerViewController *theServerViewController = [mainViewController myServerViewController];
//	[theServerViewController setConnected:NO];
	while (![theServerViewController pushFinished]) {
		[NSThread sleepForTimeInterval:0.1];
	}
	[self.navigationController popToRootViewControllerAnimated:YES];
	//[self.mainViewController viewWillAppear:YES];
}

-(void) addServer:(ServerSettings *)serverSettings
{	
	[self.serverList addObject:serverSettings];
	[mainViewController reloadTable];
}

- (void)addUserWithId:(NSInteger)uid alias:(NSString *)alias channelId:(NSInteger)cid
{
	[alias retain];
	ServerViewController *theServerViewController = [mainViewController myServerViewController];
	[theServerViewController addUserWithId:uid alias:alias channelId:cid];
	[alias release];
}

- (void)addChannelWithId:(NSInteger)cid name:(NSString *)name parentId:(NSInteger)pid
{
	[name retain];
	ServerViewController *theServerViewController = [mainViewController myServerViewController];
	[theServerViewController addChannelWithId:cid name:name parentId:pid];
	[name release];
}

- (void)removeUserWithId:(NSInteger)uid
{
	ServerViewController *theServerViewController = [mainViewController myServerViewController];
	[theServerViewController removeUserWithId:uid];
}

- (void)removeChannelWithId:(NSInteger)cid
{
	ServerViewController *theServerViewController = [mainViewController myServerViewController];
	[theServerViewController removeChannelWithId:cid];
}

- (void)moveUser:(NSInteger)uid toChannel:(NSInteger)cid
{
	ServerViewController *theServerViewController = [mainViewController myServerViewController];
	[theServerViewController moveUser:uid toChannel:cid];
}

- (uint16_t)channelIdForUser:(uint16_t)uid
{
	ServerViewController *theServerViewController = [mainViewController myServerViewController];
	return [theServerViewController channelIdForUser:uid];
}

- (User *)userForUserId:(NSInteger)uid
{
	ServerViewController *theServerViewController = [mainViewController myServerViewController];
	return [theServerViewController userForUserId:uid];
}

- (void)userTransmitting:(NSInteger)uid Status:(BOOL)isTransmit
{
	ServerViewController *theServerViewController = [mainViewController myServerViewController];
	[theServerViewController userTransmitting:uid Status:isTransmit];
}

- (void)playSoundWithId:(NSString *)soundId
{
	[[self.soundEffects objectForKey:soundId] performSelectorOnMainThread:@selector(play) withObject:nil waitUntilDone:NO];
}

- (void)saveServersToDisk
{
	// Save list to disk
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	NSString *serverListFile = [NSString stringWithFormat:@"%@/serverListFile", documentsDirectory];
	[NSKeyedArchiver archiveRootObject:self.serverList toFile:serverListFile];
	
//	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
//	NSMutableData *data = [NSKeyedArchiver archivedDataWithRootObject:self.serverList];
//	[defaults setObject:data forKey:@"servers"];
//	[defaults synchronize];
}

- (void)saveGlobalSettingsToDisk
{
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	NSMutableData *data = [NSKeyedArchiver archivedDataWithRootObject:self.globalSettings];
	[defaults setObject:data forKey:@"globalSettings"];
	[defaults synchronize];
}

- (void)saveVolumeRecordsToDisk
{
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	NSMutableData *data = [NSKeyedArchiver archivedDataWithRootObject:self.volumeRecords];
	[defaults setObject:data forKey:@"volumeRecords"];
	[defaults synchronize];
}

- (BOOL)connectionAvailable
{
	return [self alertOnReachability:hostReach];
}

- (BOOL)micAvailable
{	
	UInt32 audioInputIsAvailable;
	UInt32 propertySize = sizeof (audioInputIsAvailable);
	AudioSessionGetProperty (
							 kAudioSessionProperty_AudioInputAvailable,
							 &propertySize,
							 &audioInputIsAvailable // A nonzero value on output means that
							 // audio input is available
							 );
    if (audioInputIsAvailable) {
		return YES;
	}
	else {
		UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:@"Microphone not available" message:@"Please connect a microphone and try again." delegate:self cancelButtonTitle:nil otherButtonTitles:@"OK", nil];
		[myAlertView show];
		[myAlertView release];
		return NO;
	}
}

- (void)updateVolumeRecordForUser:(NSInteger)uid
{
	ServerViewController *theServerViewController = [mainViewController myServerViewController];
	NSString *userId = [NSString stringWithFormat:@"%d",uid];
	if (theServerViewController != nil) {
		User *user = [[theServerViewController userList] objectForKey:userId];
		if (user != nil) {
			// User exists
			VolumeRecord *aRecord = [volumeRecords objectForKey:[user userAlias]];
			if (aRecord != nil) {
				// Volume record found.. update the volume
				uint16_t theId = (uint16_t)uid;
				int theLevel = (int)[aRecord volume];
				v3_set_volume_user(theId, theLevel);
				[user setMuted:[aRecord muted]];
			}
		}
	}
	
}

#pragma mark -
#pragma mark Memory management

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
    /*
     Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
     */
}

- (void)dealloc {
	if (self.soundEffects != nil) {
		[self.soundEffects release];
	}
	if (self.mainViewController != nil) {
		[self.mainViewController release];
	}
	if (self.navigationController != nil) {
		[self.navigationController release];
	}
	if (self.serverList != nil) {
		[self.serverList release];
	}
	if (self.globalSettings != nil) {
		[self.globalSettings release];
	}
	if (self.volumeRecords != nil) {
		[volumeRecords release];
	}
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[hostReach release];
    [window release];
	
    [super dealloc];
}


@end
