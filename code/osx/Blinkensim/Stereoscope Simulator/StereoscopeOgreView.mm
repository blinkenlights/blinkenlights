
#import "StereoscopeOgreView.h"

using namespace Ogre;

//extern SceneNode* objectNode;
extern Camera *mCamera;

@implementation StereoscopeOgreView

- (void)mouseDragged:(NSEvent*)theEvent;
{
//	NSRect bounds = [self bounds];
//	float yAngle = 2*M_PI*([theEvent deltaX])/NSWidth(bounds);
//	float xAngle = 2*M_PI*([theEvent deltaY])/NSWidth(bounds);
//	objectNode->rotate(Vector3(0,1,0), Radian(yAngle));
//	objectNode->rotate(Vector3(1,0,0), Radian(xAngle), Node::TS_WORLD);
}

- (void)scrollWheel:(NSEvent *)theEvent;
{
	float move = 4*[theEvent deltaY];
	mCamera->moveRelative(Vector3(0, 0, move));
}

- (void)drawRect:(NSRect)aRect
{
    if ([self inLiveResize])
        mCamera->setAspectRatio(Real(aRect.size.width) / Real(aRect.size.height));
    [super drawRect:aRect];
}

@end
