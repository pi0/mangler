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

#import "SoundEffect.h"


@implementation SoundEffect


// Creates a sound effect object from the specified sound file
+ (id)soundEffectWithContentsOfFile:(NSString *)aPath {
    if (aPath) {
        return [[[SoundEffect alloc] initWithContentsOfFile:aPath] autorelease];
    }
    return nil;
}

// Initializes a sound effect object with the contents of the specified sound file
- (id)initWithContentsOfFile:(NSString *)path {
    self = [super init];
    
	// Gets the file located at the specified path.
    if (self != nil) {
        NSURL *aFileURL = [NSURL fileURLWithPath:path isDirectory:NO];
		// If the file exists, calls Core Audio to create a system sound ID.
        if (aFileURL != nil)  {
            SystemSoundID aSoundID;
            OSStatus error = AudioServicesCreateSystemSoundID((CFURLRef)aFileURL, &aSoundID);

            if (error == kAudioServicesNoError) { // success
                _soundID = aSoundID;
            } else {
                //NSLog(@"Error %d loading sound at path: %@", error, path);
                [self release], self = nil;
            }
        } else {
            //NSLog(@"NSURL is nil for path: %@", path);
            [self release], self = nil;
        }
    }
    return self;
}

// Releases resouces when no longer needed.
-(void)dealloc {
    AudioServicesDisposeSystemSoundID(_soundID);
    [super dealloc];
}

// Plays the sound associated with a sound effect object.
-(void)play {
	// Calls Core Audio to play the sound for the specified sound ID.
    AudioServicesPlaySystemSound(_soundID);
}


@end
