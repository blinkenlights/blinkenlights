
#import "StereoscopeOgreView.h"

using namespace Ogre;

extern BOOL shouldAnimate;
//extern SceneNode* objectNode;
extern Camera *mCamera;

@implementation StereoscopeOgreView

- (void)mouseDragged:(NSEvent*)theEvent;
{
    shouldAnimate = NO;
	NSRect bounds = [self bounds];
    
    Vector3 old = mCamera->getPosition();

    float deltaX = -100*([theEvent deltaX])/NSWidth(bounds);
    float deltaY = 100*([theEvent deltaY])/NSHeight(bounds);
    
    if ((old.x+deltaX>-100) && (old.x+deltaX<100)) mCamera->moveRelative(Vector3(deltaX,0,0));
    if ((old.y+deltaY>0.2) && (old.y+deltaY<100)) mCamera->moveRelative(Vector3(0,deltaY,0));

    Vector3 pos = mCamera->getPosition();
    if (pos.z<2) mCamera->setPosition(old.x, old.y, old.z);

    pos = mCamera->getPosition();
    //NSLog(@"Camera at %f,%f,%f",pos.x,pos.y,pos.z);
}

- (void)scrollWheel:(NSEvent *)theEvent;
{
    shouldAnimate = NO;
	float move = [theEvent deltaY];
    Vector3 old = mCamera->getPosition();
	mCamera->moveRelative(Vector3(0, 0, move));    
    Vector3 pos = mCamera->getPosition();
    if (pos.z>75) mCamera->setPosition(old.x, old.y, 75);
    if (pos.z<0.2) mCamera->setPosition(old.x, old.y, 0.2);
    if (pos.y<1) mCamera->setPosition(pos.x, 1, pos.z);
}

- (void)drawRect:(NSRect)aRect
{
    if ([self inLiveResize])
        mCamera->setAspectRatio(Real(aRect.size.width) / Real(aRect.size.height));
    [super drawRect:aRect];
}

@end
