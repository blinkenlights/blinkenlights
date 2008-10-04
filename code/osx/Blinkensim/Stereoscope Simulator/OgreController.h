/* OgreController */

#import <Cocoa/Cocoa.h>
#import <Ogre/OgreOSXCocoaView.h>
#import <Ogre/Ogre.h>

#import "TCMHost.h"
#import "BlinkenListener.h"


@interface OgreController : NSObject
{
	IBOutlet OgreView *ogreView;
	NSColor *diffuseLight;
	NSColor *specularLight;
	Ogre::Entity *topLeftWindows;
	Ogre::Entity *bottomLeftWindows;
	Ogre::Entity *topRightWindows;
	Ogre::Entity *bottomRightWindows;
	unsigned char displayState[23][54];

	TCMHost *_hostToResolve;
	NSURLConnection *_proxyListConnection;
	NSMutableData *_responseData;
	NSMutableArray *_blinkenStreamsArray;
	BlinkenListener    *_blinkenListener;
	NSMutableArray *_frameQueue;
	NSTimeInterval _maxTimeDifference;
	int            _timeSamplesTaken;
	NSTimeInterval _lastDrawTime;

	NSDictionary *_currentProxy;
	NSTimeInterval _connectionLostTime;
	NSTimeInterval _hostResolveFailureTime;
	BOOL _fadeOutOnBlinkenframe;
	int _xmlReadFailureCount;
    BOOL _showTime;
    
    NSMutableDictionary *_messageDictionary;
    int _position;
	NSMutableArray *projectTableSections;

}

@property (nonatomic, retain) NSMutableArray *projectTableSections;
@property (nonatomic, retain) NSURLConnection *proxyListConnection;
@property (nonatomic, retain) TCMHost *hostToResolve;
@property (nonatomic, retain) NSDictionary *currentProxy;
@property (nonatomic, retain) NSMutableDictionary *messageDictionary;

- (IBAction) showWebsite:(id)inSender;
- (IBAction) setCameraPosition:(id)inSender;
- (void)animateCamera;
- (void)setOgreView:(OgreView *)inView;

- (BOOL)hasConnection;

- (void)setStatusText:(NSString *)inString;
- (void)fadeoutStatusText;
- (void)connectionDidBecomeAvailable;
- (void)connectToManualProxy;
- (void)connectToProxy:(NSDictionary *)inProxy;
- (void)fetchStreamsXML;
- (void)connectToAutoconnectProxy;

@end
