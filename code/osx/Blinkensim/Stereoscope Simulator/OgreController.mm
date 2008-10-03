#import "OgreController.h"

using namespace Ogre;

// Evil Globals ;)
Ogre::SceneManager *mSceneMgr;
AnimationState* mAnimState;
Camera *mCamera;
BOOL shouldAnimate;

#define FRAMERATE 1.0/60.0

@implementation OgreController

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    shouldAnimate = YES;
    [[ogreView window] setDelegate:self];
	std::string mResourcePath = [[[NSBundle mainBundle] resourcePath] UTF8String];
	
	// Create a new root object with the correct paths
	Root *mRoot = new Root(mResourcePath + "/plugins.cfg", mResourcePath + "/ogre.cfg", mResourcePath + "/Ogre.log");
	mRoot->setRenderSystem(mRoot->getAvailableRenderers()->front());

	// use pbuffers not frame buffers because of driver problems
	mRoot->getRenderSystem()->setConfigOption("RTT Preferred Mode", "Copy");

	// Initialise, we do not want an auto created window, as that will create a carbon window
	mRoot->initialise(false);
	
	// Build the param list for a embeded cocoa window...
	NameValuePairList misc;
	misc["macAPI"] = "cocoa";
	misc["externalWindowHandle"] = StringConverter::toString((size_t)ogreView);
	
	// Create the window and load the params
	mRoot->createRenderWindow("Stereoscope Simulator", 0, 0, false, &misc);
	RenderWindow *mWindow = [ogreView ogreWindow];
	
	mSceneMgr = mRoot->createSceneManager(ST_GENERIC, "OgreNode");
	
	// Add resource locations -- looking at folders recursively
	 ResourceGroupManager::getSingleton().addResourceLocation(mResourcePath, std::string("FileSystem"), ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, false);
	 ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    
    // Set ambient light
    //mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));
    
    // Create a skydome
    //mSceneMgr->setSkyDome(true, "SkyDome", 5, 8);
    mSceneMgr->setSkyBox(true, "Toronto", 10);
    // Create a light
    Light* l = mSceneMgr->createLight("MainLight");
    // Accept default settings: point light, white diffuse, just set position
    // NB I could attach the light to a SceneNode if I wanted it to move automatically with
    //  other objects, but I don't
    l->setPosition(20,80,50);
    
    Entity *ent;
    
    // Define a floor plane mesh
    Plane p;
    p.normal = Vector3::UNIT_Y;
    p.d = 0;
    MeshManager::getSingleton().createPlane(
                                            "FloorPlane", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
                                            p, 40000, 40000, 20, 20, true, 1, 50, 50, Vector3::UNIT_Z);
    
    // Create an entity (the floor)
    ent = mSceneMgr->createEntity("floor", "FloorPlane");
    ent->setMaterialName("Ground");
    // Attach to child of root node, better for culling (otherwise bounds are the combination of the 2)
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
    
    mCamera = mSceneMgr->createCamera("PlayerCam");
    mCamera->setNearClipDistance(5);
    SceneNode* camNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    camNode->attachObject(mCamera);
    Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(ColourValue(0,0,0));
    
    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(Real(vp->getActualWidth()) / Real(vp->getActualHeight()));

    
    // Create a light
    mSceneMgr->setAmbientLight(ColourValue(0, 0, 0));
    Light *mainLight = mSceneMgr->createLight("AmbientLight");
    
    SceneNode* cityhallNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    ent = mSceneMgr->createEntity("cityhall", "CityHall.mesh");
    cityhallNode->attachObject(ent);
    cityhallNode->pitch(Degree(90)); 
    //cityhallNode->translate(Vector3(0,0,-150));

    SceneNode* rampNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    ent = mSceneMgr->createEntity("ramp", "Ramp.mesh");
    rampNode->attachObject(ent);
    rampNode->pitch(Degree(90)); 
    //rampNode->translate(Vector3(0,0,-150));
    
    SceneNode* platzNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    ent = mSceneMgr->createEntity("platz", "Platz.mesh");
    platzNode->attachObject(ent);
    platzNode->pitch(Degree(90)); 
    
    SceneNode* pavilionNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    ent = mSceneMgr->createEntity("pavilion", "Pavillon.mesh");
    pavilionNode->attachObject(ent);
    pavilionNode->pitch(Degree(90)); 
    
    SceneNode* windowNodeA1 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    ent = mSceneMgr->createEntity("windowsA1", "WindowsA1.mesh");
    windowNodeA1->attachObject(ent);
    windowNodeA1->pitch(Degree(90)); 
    
    SceneNode* windowNodeA2 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    ent = mSceneMgr->createEntity("windowsA2", "WindowsA2.mesh");
    windowNodeA2->attachObject(ent);
    windowNodeA2->pitch(Degree(90)); 
    
    SceneNode* windowNodeB1 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    ent = mSceneMgr->createEntity("windowsB1", "WindowsB1.mesh");
    windowNodeB1->attachObject(ent);
    windowNodeB1->pitch(Degree(90)); 
    
    SceneNode* windowNodeB2 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    ent = mSceneMgr->createEntity("windowsB2", "WindowsB2.mesh");
    windowNodeB2->attachObject(ent);
    windowNodeB2->pitch(Degree(90));
    
    Vector3 lookTo = Vector3(0,10,0);
    
    mCamera->setPosition(Vector3(0,5,50));
    mCamera->lookAt(lookTo);
    
    // Make sure the camera track this node
    mCamera->setAutoTracking(true, cityhallNode, lookTo);
        
    // set up spline animation of node
    Animation* anim = mSceneMgr->createAnimation("CameraTrack", 10);
    // Spline it for nice curves
    anim->setInterpolationMode(Animation::IM_SPLINE);
    // Create a track to animate the camera's node
    NodeAnimationTrack* track = anim->createNodeTrack(0, camNode);
    // Setup keyframes
    TransformKeyFrame* key = track->createNodeKeyFrame(0); // startposition
    key = track->createNodeKeyFrame(2.5);
    key->setTranslate(Vector3(50,50,-10));
    key = track->createNodeKeyFrame(5);
    key->setTranslate(Vector3(-150,100,-6));
    key = track->createNodeKeyFrame(7.5);
    key->setTranslate(Vector3(0,10,0));
    key = track->createNodeKeyFrame(10);
    key->setTranslate(Vector3(0,0,0));
    // Create a new animation state to track this
    mAnimState = mSceneMgr->createAnimationState("CameraTrack");
    mAnimState->setEnabled(true);
    
    // Put in a bit of fog for the hell of it
    //mSceneMgr->setFog(FOG_EXP, ColourValue::White, 0.0002);
    
    
	// create a timer that causes OGRE to render
	NSTimer *renderTimer = [NSTimer scheduledTimerWithTimeInterval:FRAMERATE target:self selector:@selector(renderFrame) userInfo:NULL repeats:YES];
	[[NSRunLoop currentRunLoop] addTimer:renderTimer forMode:NSEventTrackingRunLoopMode];
}

- (void)renderFrame
{
    //if (shouldAnimate) mAnimState->addTime(FRAMERATE);
	Ogre::Root::getSingleton().renderOneFrame();
	//mSceneMgr->getSceneNode("OgreNode")->rotate(Vector3(0 ,1 ,0 ), Radian(0.01));
}

- (IBAction) showWebsite:(id)inSender
{
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://blinkenlights.net/stereoscope"]];
}

#define NEAR_LEFT Vector3(-16.255816,1.924682,20.444798)
#define NEAR_MIDDLE Vector3(0.003960,0.302409,20.962048)
#define NEAR_RIGHT Vector3(14.228559,2.149735,23.378349)
#define FAR_LEFT Vector3(-12.485603,2.159666,39.822617)
#define FAR_MIDDLE Vector3(-0.632594,1.828550,42.405907)
#define FAR_RIGHT Vector3(11.238027,1.401788,38.238659)

- (IBAction)setCameraPosition:(id)inSender
{
    int camera = [inSender tag];
    Vector3 pos;
    
    if (camera == 0) {
        // Enable Animation
        shouldAnimate = YES;
    } else {
        // Disable Animation
        shouldAnimate = NO;
        switch (camera)
        {
            case 1:
                pos = FAR_RIGHT;
                break;
            case 2:
                pos = FAR_MIDDLE;
                break;
            case 3:
                pos = FAR_LEFT;
                break;
            case 4:
                pos = NEAR_RIGHT;
                break;
            case 5:
                pos = NEAR_LEFT;
                break;
            case 6:
                pos = NEAR_MIDDLE;
                break;
            default:
                break;
        }
        
        // move to pos
        mCamera->setPosition(pos.x, pos.y, pos.z);
    }
    
}


- (void)windowDidResize:(NSNotification *)notification
{
    NSRect aRect = [[ogreView window] frame];
    mCamera->setAspectRatio(Real(aRect.size.width) / Real(aRect.size.height));
}

@end
