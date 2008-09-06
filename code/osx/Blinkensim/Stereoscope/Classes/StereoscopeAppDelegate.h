//
//  StereoscopeAppDelegate.h
//  Stereoscope
//
//  Created by Martin Pittenauer on 06.09.08.
//  Copyright TheCodingMonkeys 2008. All rights reserved.
//

#import <UIKit/UIKit.h>

@class EAGLView;

@interface StereoscopeAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    EAGLView *glView;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet EAGLView *glView;

@end

