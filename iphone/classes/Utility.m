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

#import "Utility.h"


@implementation Utility

+ (UIImage *)scale:(UIImage *)image toSize:(CGSize)size
{
	/*
	 * For some reason, just weakly linking against UIKit and checking for NULL on 
	 * UIGraphicsBeginImageContextWithOptions does not work. This is the workaround:
	 */
	char majorVersion = [[[UIDevice currentDevice] systemVersion] characterAtIndex: 0];
	if (majorVersion == '2' || majorVersion == '3') {
		UIGraphicsBeginImageContext(size);
	}
	else {
		UIGraphicsBeginImageContextWithOptions(size, NO, 0.0);
	}
	
    [image drawInRect:CGRectMake(0, 0, size.width, size.height)];
    UIImage *scaledImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return scaledImage;
}

@end
