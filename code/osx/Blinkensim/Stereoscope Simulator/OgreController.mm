#import "OgreController.h"

using namespace Ogre;

// Evil Globals ;)
Ogre::SceneManager *mSceneMgr;
AnimationState* mAnimState;
Camera *mCamera;
BOOL shouldAnimate;
BOOL autoCam;

double progress;
Vector3 from, to;
BOOL animating;
Vector3 cameras[6];


#define FRAMERATE 1.0/60.0

#define NEAR_LEFT Vector3(-16.255816,1.924682,20.444798)
#define NEAR_MIDDLE Vector3(0.003960,0.302409,20.962048)
#define NEAR_RIGHT Vector3(14.228559,2.149735,23.378349)
#define FAR_LEFT Vector3(-12.485603,2.159666,39.822617)
#define FAR_MIDDLE Vector3(-0.632594,1.828550,42.405907)
#define FAR_RIGHT Vector3(11.238027,1.401788,38.238659)


#define LEFT_BOTTOM_MESH_NO  0
#define LEFT_TOP_MESH_NO     1
#define RIGHT_BOTTOM_MESH_NO 2
#define RIGHT_TOP_MESH_NO    3


GLfloat *windowMeshTextureCoords[] = {NULL,NULL,NULL,NULL};
static GLfloat windowtextureCoords[16][12];
GLfloat windowMeshTextureValues[16][2][2];

@implementation OgreController

- (void)setOgreView:(OgreView *)inView
{
    ogreView = [inView retain];
}


- (void)prepareValues
{
	// init textureCoordinates
	for (int i = 0; i<16;i++) {
		int row = i / 4;
		int column = i % 4;
		CGPoint topRight   = CGPointMake(0.0 + (column+1) * 0.25, 0.0 +  row    * 0.25);
		CGPoint bottomLeft = CGPointMake(0.0 +  column    * 0.25, 0.0 + (row+1) * 0.25);
	
//		NSLog(@"%s %d:%@ %@",__FUNCTION__,i,NSStringFromCGPoint(bottomLeft), NSStringFromCGPoint(topRight));
		windowMeshTextureValues[i][0][0] = (GLfloat)bottomLeft.x;
		windowMeshTextureValues[i][1][0] = (GLfloat)bottomLeft.y;
		windowMeshTextureValues[i][0][1] = (GLfloat)topRight.x;
		windowMeshTextureValues[i][1][1] = (GLfloat)topRight.y;
		
		
		windowtextureCoords[i][0] = topRight.x;
		windowtextureCoords[i][1] = bottomLeft.y;
		windowtextureCoords[i][2] = topRight.x;
		windowtextureCoords[i][3] = topRight.y;
		windowtextureCoords[i][4] = bottomLeft.x;
		windowtextureCoords[i][5] = topRight.y;
		
		windowtextureCoords[i][6] = topRight.x;
		windowtextureCoords[i][7] = bottomLeft.y;
		windowtextureCoords[i][8] = bottomLeft.x;
		windowtextureCoords[i][9] = topRight.y;
		windowtextureCoords[i][10] = bottomLeft.x;
		windowtextureCoords[i][11] = bottomLeft.y;
	}
}

- (void)initializeTextureCordsForMeshNo:(int)meshNo entity:(Entity *)inEntity
{
    printf("------------------> window vertex data: <----------------------\n");
    SubMesh *submesh = inEntity->getMesh()->getSubMesh(0);
    VertexData *vertexData = submesh->vertexData;
    VertexDeclaration *declaration = vertexData->vertexDeclaration;
    for (int i=0;i<declaration->getElementCount();i++) {
    	const VertexElement *element = declaration->getElement(i);
    	printf("element %d has type of %d\n",i, element->getType());
    }
    
	// get our copy of the initial mesh values
//	windowMeshTextureCoords[1] = (GLfloat *)calloc(2 * g_sScene.pMesh[3].nNumVertex,sizeof(GLfloat));
//	windowMeshTextureCoords[2] = (GLfloat *)calloc(2 * g_sScene.pMesh[4].nNumVertex,sizeof(GLfloat));
//	windowMeshTextureCoords[3] = (GLfloat *)calloc(2 * g_sScene.pMesh[5].nNumVertex,sizeof(GLfloat));


	const VertexElement *normVE = vertexData->
		vertexDeclaration->findElementBySemantic(VES_TEXTURE_COORDINATES);
	HardwareVertexBufferSharedPtr normHVB = vertexData->
		vertexBufferBinding->getBuffer(normVE->getSource());
	GLfloat* texcoords = (GLfloat*) normHVB->lock(0, normHVB->getSizeInBytes(), 
			HardwareBuffer::HBL_NORMAL);
	
	windowMeshTextureCoords[meshNo] = (GLfloat *)calloc(2 * vertexData->vertexCount,sizeof(GLfloat));

    IndexData *indexData = submesh->indexData;
	HardwareIndexBufferSharedPtr indexHB = indexData->indexBuffer ;
	unsigned short * vertexIndices = (unsigned short*) indexHB->lock(
		0, indexHB->getSizeInBytes(), HardwareBuffer::HBL_READ_ONLY);
	size_t numFaces = indexData->indexCount / 3 ;
	for(int i=0 ; i<numFaces ; i++, vertexIndices+=3) {
		//~ int p0 = 0;
		//~ int p1 = 1;
		//~ int p2 = 2;
		int p0 = vertexIndices[0] ;
		int p1 = vertexIndices[1] ;
		int p2 = vertexIndices[2] ;
//		printf("window half 1 at %d %d %d\n",p0,p1,p2);
//		printf("vertex coords: %f, %f",texcoords[p0 * 8 + 6],texcoords[p0 * 8 + 7]);
//		printf("vertex coords: %f, %f",texcoords[p1 * 8 + 6],texcoords[p1 * 8 + 7]);
//		printf("vertex coords: %f, %f",texcoords[p2 * 8 + 6],texcoords[p2 * 8 + 7]);
		if (texcoords[p0 * 8 + 6] - 0.95 > 0.05)  texcoords[p0 * 8 + 6]*= 0.25;
		if (texcoords[p0 * 8 + 7] - 0.95 > 0.05)  texcoords[p0 * 8 + 7]*= 0.25;
		if (texcoords[p1 * 8 + 6] - 0.95 > 0.05)  texcoords[p1 * 8 + 6]*= 0.25;
		if (texcoords[p1 * 8 + 7] - 0.95 > 0.05)  texcoords[p1 * 8 + 7]*= 0.25;
		if (texcoords[p2 * 8 + 6] - 0.95 > 0.05)  texcoords[p2 * 8 + 6]*= 0.25;
		if (texcoords[p2 * 8 + 7] - 0.95 > 0.05)  texcoords[p2 * 8 + 7]*= 0.25;
		printf("after vertex coords: %f, %f",texcoords[p0 * 8 + 6],texcoords[p0 * 8 + 7]);
		printf("after vertex coords: %f, %f",texcoords[p1 * 8 + 6],texcoords[p1 * 8 + 7]);
		printf("after vertex coords: %f, %f",texcoords[p2 * 8 + 6],texcoords[p2 * 8 + 7]);
	}
	indexHB->unlock();

	normHVB->unlock();

}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{   
    cameras[0] = FAR_LEFT;
    cameras[1] = FAR_MIDDLE;
    cameras[2] = FAR_RIGHT;
    cameras[3] = NEAR_LEFT;
    cameras[4] = NEAR_RIGHT;
    cameras[5] = NEAR_MIDDLE;
    
    srand( (unsigned int) time( NULL ));

	[self prepareValues];

    shouldAnimate = YES;
    [[ogreView window] setDelegate:self];
	std::string mResourcePath = [[[NSBundle bundleForClass:[self class]] resourcePath] UTF8String];
	
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
    
    [self initializeTextureCordsForMeshNo:LEFT_TOP_MESH_NO entity:ent];

    
	// let us create an entity per window this should be easier to handle

    SceneNode* windowNodeA2 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    ent = mSceneMgr->createEntity("windowsA2", "WindowsA2.mesh");
    windowNodeA2->attachObject(ent);
    windowNodeA2->pitch(Degree(90)); 
    
    [self initializeTextureCordsForMeshNo:LEFT_BOTTOM_MESH_NO entity:ent];

    SceneNode* windowNodeB1 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    ent = mSceneMgr->createEntity("windowsB1", "WindowsB1.mesh");
    windowNodeB1->attachObject(ent);
    windowNodeB1->pitch(Degree(90)); 
    
    [self initializeTextureCordsForMeshNo:RIGHT_BOTTOM_MESH_NO entity:ent];

    SceneNode* windowNodeB2 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    ent = mSceneMgr->createEntity("windowsB2", "WindowsB2.mesh");
    windowNodeB2->attachObject(ent);
    windowNodeB2->pitch(Degree(90));

    [self initializeTextureCordsForMeshNo:RIGHT_BOTTOM_MESH_NO entity:ent];
    
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

    autoCam = YES;
    shouldAnimate = YES;
    progress = 1.0;    
    
	// create a timer that causes OGRE to render
	NSTimer *renderTimer = [NSTimer scheduledTimerWithTimeInterval:FRAMERATE target:self selector:@selector(renderFrame) userInfo:NULL repeats:YES];
	[[NSRunLoop currentRunLoop] addTimer:renderTimer forMode:NSEventTrackingRunLoopMode];
}

- (IBAction) showWebsite:(id)inSender
{
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://blinkenlights.net/stereoscope"]];
}

- (IBAction)setCameraPosition:(id)inSender
{
    int camera = [inSender tag];
    Vector3 pos;
    
    if (camera == 0) {
        // Enable Animation
        autoCam = YES;
        shouldAnimate = YES;
        progress = 1.0;
    } else {
        // Disable Animation
        autoCam = NO;
        to = cameras[camera-1];
        [self animateCamera];
    }
    
}

- (void)animateCamera {
    
    progress = 0;
    shouldAnimate = YES;
    from = mCamera->getPosition();
    //to = inTo;
}

- (void)renderFrame
{
    if (autoCam&&progress>=1) {
        // Choose next cam after random wait
        if (rand() % 100 == 1) {
        NSLog(@"AUTO!");
        
        to = cameras[rand() % 6];
        [self animateCamera];
        }
            
    }
    
    if (shouldAnimate) {
        progress = progress + (1.0/(2.0*60.0));
        //NSLog(@"progress: %f",progress);
        if (progress>=1) {
            
            shouldAnimate = NO;
            
        } else {
            double p = progress;
            if (p < 0.5) {
                p = p*p*2;
            } else {
                p =  - 2 * (p-1)*(p-1) + 1;
                
            }
            
            mCamera->setPosition(from.x+(to.x-from.x)*p, from.y+(to.y-from.y)*p, from.z+(to.z-from.z)*p);
        }
    }
    
	Ogre::Root::getSingleton().renderOneFrame();
}


- (void)windowDidResize:(NSNotification *)notification
{
    NSRect aRect = [[ogreView window] frame];
    mCamera->setAspectRatio(Real(aRect.size.width) / Real(aRect.size.height));
}

@end
