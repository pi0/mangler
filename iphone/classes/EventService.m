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

#import "EventService.h"
#define _GNU_SOURCE

@implementation EventService

@synthesize audioController;
@synthesize finished;
@synthesize connectSuccess;
@synthesize loginComplete;
@synthesize serverIndex;
@synthesize errorMessage;

int debug;
BOOL should_exit;
BOOL feederFinished;
BOOL consumerFinished;
EventService *refToSelf;

- (id)initWithServerSettingsIndex:(NSInteger)aServerIndex
{
	running = NO;
	recording = NO;
	finished = NO;
	debug = 0;
	should_exit = NO;
	feederFinished = NO;
	consumerFinished = NO;
	connectSuccess = NO;
	refToSelf = self;
	appDelegate = (VentafoneAppDelegate *)[[UIApplication sharedApplication] delegate];
	serverIndex = aServerIndex;

	return self;
}

- (void)startService
{
	//NSLog(@"EventService >>> (startService) >>> entering");
	running = YES;
	[appDelegate setRefreshEncoder:YES];
	[NSThread detachNewThreadSelector:@selector(runServiceThread) toTarget:self withObject:nil];
}

- (void)stopService {
	//NSLog(@"EventService >>> (stopService) >>> entering");
	running = NO;
	//should_exit = YES;
	
	if (audioController != nil) {
		if ([audioController isRecording]) {
			if (recording) {
				recording = NO;
				v3_stop_audio();
			}
			//[audioController performSelectorOnMainThread:@selector(stopRecording) withObject:nil waitUntilDone:NO];
			[audioController stopRecording];
		}
		//NSLog(@"Checking for output..\n");
		if ([audioController isOutput]) {
			//NSLog(@"We are outputting.. telling audio controller to stop.\n");
			//[audioController performSelectorOnMainThread:@selector(stopOutput) withObject:nil waitUntilDone:NO];
			[audioController stopOutput];
		}
	}
	
	if (v3_is_loggedin()) {
		//NSLog(@"EventService >>> (stopService) >>> We are logged in, so send logout");
		v3_logout();
	}
	else {
		//NSLog(@"EventService >>> (stopService) >>> We are not logged in.. doing nothing");
	}

}

- (void)killService
{
	//NSLog(@"EventService >>> (killService) >>> entering");
	running = NO;
	//should_exit = YES;
	
	//	if ([audioController isRunning]) {
	//		[audioController stopAudioSession];
	//	}
	
	if (audioController != nil) {
		if ([audioController isRecording]) {
			if (recording) {
				recording = NO;
				v3_stop_audio();
			}
			//[audioController performSelectorOnMainThread:@selector(stopRecording) withObject:nil waitUntilDone:NO];
			[audioController stopRecording];
		}
		//NSLog(@"Checking for output..\n");
		if ([audioController isOutput]) {
			//NSLog(@"We are outputting.. telling audio controller to stop.\n");
			//[audioController performSelectorOnMainThread:@selector(stopOutput) withObject:nil waitUntilDone:NO];
			[audioController stopOutput];
		}
	}
	
	
//	if (audioController != nil) {
//		if ([audioController isRunning]) {
//			[audioController killAudio];
//		}
//	}
//	if (recording) {
//		recording = NO;
//		v3_stop_audio();
//	}
	
	
	if (v3_is_loggedin()) {
		//NSLog(@"EventService >>> (killService) >>> We are logged in, so send logout");
		v3_logout();
	}
	else {
		//NSLog(@"EventService >>> (killService) >>> We are not logged in.. doing nothing");
	}

}

- (void)startRecording
{
	// Event coming in from user via device to record
	if (running) {
		if (recording) {
			return;
		}
		if (audioController != nil) {
			recording = YES;
			uint16_t sendType = 3;
			[appDelegate userTransmitting:v3_get_user_id() Status:YES];
			[audioController performSelectorOnMainThread:@selector(startRecording) withObject:nil waitUntilDone:NO];
			v3_start_audio(sendType);
			if ([[appDelegate globalSettings] keyClicks]) {
				[appDelegate playSoundWithId:@"MicKeyDown"];
			}
		}
	}
	return;
}

- (void)stopRecording
{
	// Event coming in from user via device to stop recording
	if (running) {
		if (!recording) {
			return;
		}
		recording = NO;
		if (audioController != nil) {
			[audioController performSelectorOnMainThread:@selector(stopRecording) withObject:nil waitUntilDone:NO];
		}
		v3_stop_audio();
		if ([[appDelegate globalSettings] keyClicks]) {
			[appDelegate playSoundWithId:@"MicKeyUp"];
		}
		[appDelegate userTransmitting:v3_get_user_id() Status:NO];
	}
	return;
}

- (void)runServiceThread
{   
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	//NSLog(@"EventService >>> (runServiceThread) >>> entering");
	loginComplete = NO;
	
//	conninfo.server = "kalne.ath.cx:384";
//	conninfo.username = "kalnePhone";
//	conninfo.channelid = "";
//	conninfo.password = "";
//	
//    if (! conninfo.server)  {
//        fprintf(stderr, "error: server hostname was not specified\n");
//		return;
//    }
//    if (! conninfo.username)  {
//        fprintf(stderr, "error: username was not specified\n");
//		return;
//    }
//    if (! conninfo.channelid) {
//        fprintf(stderr, "error: channel id was not specified\n");
//		return;
//    }
//    if (! conninfo.password) {
//        conninfo.password = "";
//    }
//    printf("server: %s\nusername: %s\n", conninfo.server, conninfo.username);
	
	//NSLog(@"EventService >>> (runServiceThread) >>> detaching feeder thread");
	[NSThread detachNewThreadSelector:@selector(feeder) toTarget:self withObject:nil];
	//NSLog(@"EventService >>> (runServiceThread) >>> detaching consumer thread");
	[NSThread detachNewThreadSelector:@selector(consumer) toTarget:self withObject:nil];
	while (!should_exit)
	{
		//NSLog(@"EventService >>> (runServiceThread) >>> waiting for should exit...");
		[NSThread sleepForTimeInterval:0.1];
	}

	if (running)
	{
		if (connectSuccess) {
			//NSLog(@"EventService >>> (runServiceThread) >>> perform server disconnected on app delegate");
			[appDelegate performSelectorOnMainThread:@selector(serverDisconnected) withObject:nil waitUntilDone:NO];
		}
		running = NO;
		if (v3_is_loggedin()) {
			v3_logout();
		}
	}
	
	while (!feederFinished || !consumerFinished) {
		//NSLog(@"EventService >>> (runServiceThread) >>> waiting for feeder and consumer to finish...");
		[NSThread sleepForTimeInterval:0.1];
	}
	
	if (!connectSuccess) {
		//NSLog(@"EventService >>> (runServiceThread) >>> perform connect failed on app delegate");
		[appDelegate performSelectorOnMainThread:@selector(connectFailed:) withObject:self.errorMessage waitUntilDone:NO];
	}

	finished = YES;
	//NSLog(@"EventService >>> (runServiceThread) >>> exiting thread");
	[pool release];
    return;

}

-(void)feeder
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	//NSLog(@"EventService >>> (feeder) >>> entering");
    _v3_net_message *msg;

    if (debug >= 2) {
        v3_debuglevel(V3_DEBUG_PACKET | V3_DEBUG_PACKET_PARSE | V3_DEBUG_INFO);
    }

	NSString *connectionString = [NSString stringWithFormat:@"%@:%@",[[[appDelegate serverList] objectAtIndex:serverIndex] hostAddress],[[[appDelegate serverList] objectAtIndex:serverIndex] hostPort]];
	char *server = [connectionString cStringUsingEncoding:NSUTF8StringEncoding];
	char *user = [[[[appDelegate serverList] objectAtIndex:serverIndex] userAlias] cStringUsingEncoding:NSUTF8StringEncoding];
	char *password = [[[[appDelegate serverList] objectAtIndex:serverIndex] hostPassword] cStringUsingEncoding:NSUTF8StringEncoding];
    if (! v3_login(server, user, password, "")) {
//		UIAlertView *myAlertView = [[UIAlertView alloc] initWithTitle:@"Could not connect to server" message:@"Please verify that you are connected to a network, verify the connection settings, and try again" delegate:self cancelButtonTitle:nil otherButtonTitles:@"OK", nil];
//		[myAlertView show];
//		[myAlertView release];
		//_v3_logout();
		NSString *theError = [[NSString alloc] initWithFormat:@"%s",_v3_error(NULL)];
		[self setErrorMessage:theError];
		[theError release];
		//NSLog(@"EventService >>> (feeder) >>> failed login, setting should exit to YES");
		should_exit = YES;
    }
	else {
		//NSLog(@"EventService >>> (feeder) >>> passed login, setting should exit to NO");
		connectSuccess = YES;
	}


//    while ((msg = _v3_recv(V3_BLOCK)) != NULL) {
//        switch (_v3_process_message(msg)) {
//            case V3_MALFORMED:
//                _v3_debug(V3_DEBUG_INFO, "received malformed packet");
//                break;
//            case V3_NOTIMPL:
//                _v3_debug(V3_DEBUG_INFO, "packet type not implemented");
//                break;
//            case V3_OK:
//                _v3_debug(V3_DEBUG_INFO, "packet processed");
//                break;
//        }
//		if (should_exit) {
//			break;
//		}
//    }
	
	while (v3_is_loggedin() || !should_exit) {
		while ((msg = _v3_recv(V3_BLOCK)) != NULL) {
			switch (_v3_process_message(msg)) {
				case V3_MALFORMED:
					_v3_debug(V3_DEBUG_INFO, "received malformed packet");
					break;
				case V3_NOTIMPL:
					_v3_debug(V3_DEBUG_INFO, "packet type not implemented");
					break;
				case V3_OK:
					_v3_debug(V3_DEBUG_INFO, "packet processed");
					break;
			}
			if (should_exit) {
				break;
			}
		}
	}
	

//	while (v3_is_loggedin() && !should_exit)
//	{
//        if ((msg = _v3_recv(V3_NONBLOCK)) == NULL) {
//            //printf("recv() failed: %s\n", _v3_error(NULL));
//        }
//		else {
//			switch (_v3_process_message(msg)) {
//				case V3_MALFORMED:
//					_v3_debug(V3_DEBUG_INFO, "received malformed packet");
//					break;
//				case V3_NOTIMPL:
//					_v3_debug(V3_DEBUG_INFO, "packet type not implemented");
//					break;
//				case V3_OK:
//					_v3_debug(V3_DEBUG_INFO, "packet processed");
//					/*
//					 if (v3_queue_size() > 0) {
//					 fprintf("stderr", "there's something to do\n");
//					 }
//					 */
//					break;
//			}
//		}
//    } 
	
	
	should_exit = YES;
	feederFinished = YES;
	//NSLog(@"EventService >>> (feeder) >>> exiting");
	[pool release]; 
	return;
}

-(void)consumer
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	//NSLog(@"EventService >>> (consumer) >>> entering");
	v3_event *ev;
	//while ((ev = v3_get_event(V3_BLOCK)) && !should_exit)
	//while ((ev = v3_get_event(V3_NONBLOCK)) != NULL)
	while (!should_exit) {
		while ((ev = v3_get_event(V3_NONBLOCK)) != NULL)
		{
			if (debug) {
				printf("lv3_test: got event type %d\n", ev->type);
			}
			switch (ev->type) {
				case V3_EVENT_DISCONNECT:
					//NSLog(@"EventService >>> (consumer) >>> Received disconnect event");
					if (audioController != nil) {
						[audioController release];
						audioController = nil;
					}
					should_exit = YES;
					free(ev);
					if (connectSuccess) {
						//NSLog(@"EventService >>> (consumer) >>> Connect was successful, so we will play the disconnect sound");
						if ([[appDelegate globalSettings] soundEffects]) {
							[appDelegate playSoundWithId:@"disconnect"];
						}
					}
					else if (running) {
						// Disconnect on start
						//NSLog(@"######################################################");
						//NSLog(@"##### CRITICAL ERROR ---- DISCONNECT ON START!!! #####");
						//NSLog(@"######################################################");
						if (v3_is_loggedin()) {
							v3_logout();
						}
						[appDelegate performSelectorOnMainThread:@selector(serverDisconnectedOnConnect) withObject:nil waitUntilDone:NO];
					}

					consumerFinished = YES;
					//NSLog(@"EventService >>> (consumer) >>> exiting");
					[pool release]; 
					return;
				case V3_EVENT_LOGIN_COMPLETE:

					loginComplete = YES;
					v3_change_channel(atoi(""), "");
					v3_serverprop_open();
					if (v3_is_loggedin()) {
						const v3_codec *codec_info;
						
						if ((codec_info = v3_get_channel_codec(0)))
						{
							if ( (codec_info->codec != 0) && (codec_info->codec != 3) )
							{
								NSString *codecName = [NSString stringWithFormat:@"%s",codec_info->name];
								[appDelegate performSelectorOnMainThread:@selector(unsupportedCodec:) withObject:codecName waitUntilDone:NO];
							}
							//EventService *eventService = 
							AudioController *anAudioController = [[AudioController alloc] initWithRate:codec_info->rate];
							[self setAudioController:anAudioController];
							[anAudioController release];
							
//							printf("############ CODEC INFO ############\n");
//							printf("Codec: %d\n",codec_info->codec);
//							printf("Format: %d\n",codec_info->format);
//							printf("PCM Framesize: %d\n",codec_info->pcmframesize);
//							printf("Rate: %d\n",codec_info->rate);
//							printf("Quality: %d\n",codec_info->quality);
//							printf("Name: %s",codec_info->name);
						}
	//                    //channelTree->expand_all();
	//                    if (connectedServerName.length()) {
	//                        iniSection &server(config.servers[connectedServerName]);
	//                        if (server["PersistentComments"].toBool()) {
	//                            comment = server["Comment"].toUString();
	//                            url = server["URL"].toUString();
	//                            v3_set_text(
	//										(char *)ustring_to_c(comment).c_str(),
	//										(char *)ustring_to_c(url).c_str(),
	//										(char *)ustring_to_c(integration_text).c_str(),
	//										true);
	//                        }
	//                        uint32_t channel_id = v3_get_channel_id(ustring_to_c(server["DefaultChannel"].toUString()).c_str());
	//                        if (channel_id && (c = v3_get_channel(channel_id))) {
	//                            Glib::ustring password = channelTree->getChannelSavedPassword(channel_id);
	//                            uint16_t pw_channel = 0;
	//                            if ((pw_channel = v3_channel_requires_password(channel_id)) &&
	//                                (password = channelTree->getChannelSavedPassword(pw_channel)).empty() &&
	//                                (password = getPasswordEntry("Channel Password")).length()) {
	//                                channelTree->setChannelSavedPassword(pw_channel, password);
	//                            }
	//                            if (!pw_channel || (pw_channel && password.length())) {
	//                                v3_change_channel(channel_id, (char *)((pw_channel) ? password.c_str() : ""));
	//                            }
	//                            v3_free_channel(c);
	//                        }
	//                    }
					}
					if ([[appDelegate globalSettings] soundEffects]) {
						[appDelegate playSoundWithId:@"connect"];
					}
					break;
				case V3_EVENT_SRV_PROP_RECV:

					// Server settings
					//admin->serverSettingsUpdated(ev->data->srvprop);
					break;
					
				case V3_EVENT_ERROR_MSG:
					//NSLog(@"Error message: %s",ev->error.message);
					break;
					
				case V3_EVENT_CHAN_BADPASS:
					[appDelegate performSelectorOnMainThread:@selector(joinChannelFailed:) withObject:[NSString stringWithFormat:@"%s",ev->error.message] waitUntilDone:NO];
					break;
					
					
				/**********************************************************************
				 *
				 *
				 *
				 *		AUDIO EVENTS
				 *
				 *
				 *
				 **********************************************************************/
					
				case V3_EVENT_USER_TALK_START:
					// Update UI
					if (ev->user.id != 0) {
						if (audioController != nil) {
							[audioController performSelectorOnMainThread:@selector(startOutput) withObject:nil waitUntilDone:NO];
						}
						NSInteger uid = ev->user.id;
						[appDelegate userTransmitting:uid Status:YES];
					}
					break;
				case V3_EVENT_USER_TALK_END:
				case V3_EVENT_USER_TALK_MUTE:
					// Update UI
					if (ev->user.id != 0) {
						if (audioController != nil) {
							[audioController performSelectorOnMainThread:@selector(stopOutput) withObject:nil waitUntilDone:NO];
						}
						NSInteger uid = ev->user.id;
						[appDelegate userTransmitting:uid Status:NO];
					}
					break;
				case V3_EVENT_PLAY_AUDIO:
					// Add pcm to queue
					//printf("This packet needs to be played!!\n");
					//printf("Received play audio event rate: %d, channels: %d, length: %d\n", ev->pcm.rate, ev->pcm.channels, ev->pcm.length);
					//										16000				1   \\also - 16 bit
					//outputAudio[ev->user.id]->queue(ev->pcm.length, (uint8_t *)ev->data->sample);
					if (audioController != nil) {
						if (running) {
							if (![audioController isOutput]) {
								[audioController performSelectorOnMainThread:@selector(startOutput) withObject:nil waitUntilDone:NO];
							}
							[audioController queueOutput:ev->pcm.length Sample:(uint8_t *)ev->data->sample];
						}
					}
					break;
				case V3_EVENT_RECORD_UPDATE:
					//NSLog(@"EVENTSERVICE: Record update type received.\n");
					break;
					
					
				/**********************************************************************
				 *
				 *
				 *
				 *		USER CONNECTED/LEFT SERVER -OR- USER/CHANNEL CHANGED/ADDED CHANNELS
				 *
				 *
				 *
				 **********************************************************************/
					
				case V3_EVENT_USER_CHAN_MOVE:
					if (ev->user.id != 0) {
						if (ev->user.id == v3_get_user_id()) {
							// Our user is changing channel.. check the audio rate for the new channel
							const v3_codec *codec_info;
							if ((codec_info = v3_get_channel_codec(ev->channel.id)))
							{
								// Check to make sure the new channel uses a supported codec first
								if ( (codec_info->codec != 0) && (codec_info->codec != 3) )
								{
									NSString *codecName = [NSString stringWithFormat:@"%s",codec_info->name];
									[appDelegate performSelectorOnMainThread:@selector(channelUnsupportedCodec:) withObject:codecName waitUntilDone:NO];
									break;
								}
								if (audioController != nil) {
									if (codec_info->rate != (int)[audioController rate] ) {
										// We need to re-initialize new audio controller for this channel
										//[audioController release];
										AudioController *anAudioController = [[AudioController alloc] initWithRate:codec_info->rate];
										[self setAudioController:anAudioController];
										[anAudioController release];
									}
								}
							}
							if ([[appDelegate globalSettings] soundEffects]) {
								[appDelegate playSoundWithId:@"Channel"];
							}
						}
						else {
							// Someone else is changing channels
							if ([[appDelegate globalSettings] soundEffects]) {
								if (ev->channel.id == v3_get_user_channel(v3_get_user_id())) {
									// They are joining our channel
									[appDelegate playSoundWithId:@"ChannelJoin"];
								}
								else if ([appDelegate channelIdForUser:ev->user.id] == v3_get_user_channel(v3_get_user_id()))
								{
									// They are leaving our channel
									[appDelegate playSoundWithId:@"ChannelLeave"];
								}
							}
							
						}

						v3_user *user = v3_get_user(ev->user.id);
						if (user) {
							NSInteger uid = ev->user.id;
							NSInteger cid = ev->channel.id;
							[appDelegate moveUser:uid toChannel:cid];
							v3_free_user(user);
						}
					}
					break;
				case V3_EVENT_USER_LOGIN:
					if (ev->user.id != 0) {
						v3_user *user = v3_get_user(ev->user.id);
						if (user) {
							NSInteger uid = ev->user.id;
							NSString *name = [NSString stringWithFormat:@"%s",user->name];
							NSInteger cid = ev->channel.id;
							[appDelegate addUserWithId:uid alias:name channelId:cid];
							if (loginComplete) {
								if (ev->user.id != v3_get_user_id()) {
									if ([[appDelegate globalSettings] soundEffects]) {
										[appDelegate playSoundWithId:@"UserConnect"];
									}
								}
							}
							[appDelegate updateVolumeRecordForUser:uid];
							v3_free_user(user);
						}
					}
					break;
				case V3_EVENT_USER_LOGOUT:
					if (ev->user.id != 0) {
						NSInteger uid = ev->user.id;
						[appDelegate removeUserWithId:uid];
						if ([[appDelegate globalSettings] soundEffects]) {
							[appDelegate playSoundWithId:@"UserDisconnect"];
						}
						
					}
					break;
				case V3_EVENT_CHAN_ADD:
				{
					v3_channel *channel = v3_get_channel(ev->channel.id);
					if (channel) {
						NSInteger cid = ev->channel.id;
						NSInteger pid = channel->parent;
						NSString *name = [NSString stringWithFormat:@"%s",channel->name];
						[appDelegate addChannelWithId:cid name:name parentId:pid];
						v3_free_channel(channel);
					}
					break;
				}
				case V3_EVENT_CHAN_REMOVE:
				{
					NSInteger cid = ev->channel.id;
					[appDelegate removeChannelWithId:cid];
					break;
				}
			}
			free(ev);
		}
	}
	consumerFinished = YES;
	//NSLog(@"EventService >>> (consumer) >>> exiting");
	[pool release];
	return;
}

- (void)dealloc {
	if (audioController != nil) {
		[audioController release];
		//audioController = nil;
	}
	if (errorMessage != nil) {
		[errorMessage release];
	}
	[super dealloc];
}

@end

