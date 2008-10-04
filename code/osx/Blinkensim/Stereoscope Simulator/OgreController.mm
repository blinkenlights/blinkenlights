#import "OgreController.h"
#import "TableSection.h"

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

// five seconds without a frame means timeout
#define CONNECTIION_NO_FRAME_TIMEOUT 10.0
#define HOST_RESOLVING_TIMEOUT      10.0
#define MINCAMERAPOSITIONTIME      10.0


GLfloat *windowMeshTextureCoords[] = {NULL,NULL,NULL,NULL};
static GLfloat windowtextureCoords[16][12];
GLfloat windowMeshTextureValues[16][2][2];

@implementation OgreController

@synthesize hostToResolve = _hostToResolve;
@synthesize proxyListConnection = _proxyListConnection;
@synthesize currentProxy = _currentProxy;
@synthesize messageDictionary = _messageDictionary;
@synthesize projectTableSections;

+ (void)initialize
{
	[[NSUserDefaults standardUserDefaults] registerDefaults:[NSDictionary dictionaryWithObjectsAndKeys:
		[NSNumber numberWithBool:YES],@"autoselectProxy",
		[NSNumber numberWithInt:0],@"message-number",
		@"4242",@"customProxyPort",
		@"localhost",@"customProxyAddress",
		nil]
	];
	srandomdev(); // have a good seed.
}

- (void)setOgreView:(OgreView *)inView
{
    ogreView = [inView retain];
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)inSender
{
	[_ibWindow makeKeyAndOrderFront:self];
	return NO;
}

- (void)prepareValues
{
	// init textureCoordinates
	for (int i = 0; i<16;i++) {
		int row = i / 4;
		int column = i % 4;
		CGPoint topRight   = CGPointMake(0.0 + (column+1) * 0.25, 0.0 + (row+1) * 0.25);
		CGPoint bottomLeft = CGPointMake(0.0 +  column    * 0.25, 0.0 +  row    * 0.25);
	
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

- (NSDictionary *)manualProxy {
	NSArray *components = [[[NSUserDefaults standardUserDefaults] stringForKey:@"blinkenproxyAddress"] componentsSeparatedByString:@":"];
	NSString *address = [components count]>0 ? [components objectAtIndex:0] : @"";
	NSString *port    = [components count]>1 ? [components objectAtIndex:1] : nil;
	return [NSDictionary dictionaryWithObjectsAndKeys:
					address,@"address",
					port, @"port", // warning if no port port dictionary stops here
					nil];
}

- (BOOL)manualProxyIsSet {
	NSString *proxy = [[NSUserDefaults standardUserDefaults] stringForKey:@"blinkenproxyAddress"];
	return (proxy && [proxy length]);
}

- (IBAction)setCustomProxy:(id)inSender
{
	[NSApp beginSheet:_ibCustomSheet modalForWindow:_ibWindow modalDelegate:nil didEndSelector:NULL contextInfo:NULL];
}

- (IBAction)cancelCustomProxy:(id)inSender
{
	[_ibCustomSheet orderOut:self];
	[NSApp endSheet:_ibCustomSheet];
}
- (IBAction)connectToCustomProxy:(id)inSender;
{
	[_ibCustomSheet orderOut:self];
	[NSApp endSheet:_ibCustomSheet];
	[self connectToProxy:[NSDictionary dictionaryWithObjectsAndKeys:
					[[NSUserDefaults standardUserDefaults] objectForKey:@"customProxyAddress"],@"address",
					[[NSUserDefaults standardUserDefaults] objectForKey:@"customProxyPort"], @"port", // warning if no port port dictionary stops here
					nil]];
}

- (void)updateFeedMenu
{
	while ([_ibStreamsMenu numberOfItems] > 1) [_ibStreamsMenu removeItemAtIndex:[_ibStreamsMenu numberOfItems]-1];
	
	NSMenuItem *item = nil;
	
	for (TableSection *section in self.projectTableSections)
	{
		[_ibStreamsMenu addItem:[NSMenuItem separatorItem]];
		item = [[[NSMenuItem alloc] initWithTitle:[section heading] action:0 keyEquivalent:@""] autorelease];
		[_ibStreamsMenu addItem:item];
		for (NSDictionary *proxy in [section items])
		{
			item = [[[NSMenuItem alloc] initWithTitle:
				[NSString stringWithFormat:@"%@ %@- %@:%@",
					[proxy objectForKey:@"name"] ? [proxy objectForKey:@"name"] : @"unnamed",
					[proxy objectForKey:@"kind"] ? [NSString stringWithFormat:@"(%@) ",[proxy objectForKey:@"kind"]] : @"",
					[proxy objectForKey:@"address"],[proxy objectForKey:@"port"]] action:@selector(selectProxy:) keyEquivalent:@""] autorelease];
			[item setRepresentedObject:proxy];

			[_ibStreamsMenu addItem:item];
		}
	}
}

- (void)updateWithBlinkenstreams:(NSArray *)inBlinkenArray
{
	[[NSUserDefaults standardUserDefaults] setObject:inBlinkenArray forKey:@"blinkenArray"];
	[projectTableSections removeAllObjects];
	for (NSDictionary *project in inBlinkenArray)
	{
		NSString *name = [project objectForKey:@"name"];
		if ([[project objectForKey:@"building"] isEqualToString:@"stereoscope"]) {
			NSMutableArray *proxyArray = [NSMutableArray new];
			for (NSDictionary *proxy in [project objectForKey:@"proxies"])
			{
				if ([[proxy valueForKey:@"size"] isEqualToString:@"displayed"]) {
					NSMutableDictionary *proxyDict = [proxy mutableCopy];
					[proxyDict setValue:name forKey:@"projectName"];
					[proxyDict setValue:[project valueForKey:@"building"] forKey:@"projectBuilding"];
					[proxyArray addObject:proxy];
				}
			}
			if ([proxyArray count]) {
				TableSection *section = [TableSection sectionWithItems:proxyArray heading:name indexLabel:@""];
				section.representedObject = project;
				[projectTableSections addObject:section];
			}
			[proxyArray release];
		}
	}
	[self updateFeedMenu];
//	NSLog(@"%s %@",__FUNCTION__,projectTableSections);
}



- (void)resetTimeCompensation
{
	_maxTimeDifference = -999999999999999.0;
	_timeSamplesTaken = 0;
}


- (void)updateWindows:(unsigned char *)inDisplayState
{

    SubMesh *submesh = NULL;
    VertexData *vertexData = NULL;
	const VertexElement *texcoordVE = NULL;
    HardwareVertexBufferSharedPtr texcoordHVB;
    GLfloat* texcoords = NULL;
    IndexData *indexData = NULL;
    HardwareIndexBufferSharedPtr indexHB;
    unsigned short * vertexIndices = NULL;
    size_t numFaces = 0;
    GLfloat *originalTextureCoords = NULL;
    size_t vertexIndexPosition = 0;
    
	// bottom left tower
    submesh = bottomLeftWindows->getMesh()->getSubMesh(0);
    vertexData = submesh->vertexData;
	texcoordVE = vertexData->
		vertexDeclaration->findElementBySemantic(VES_TEXTURE_COORDINATES);
	texcoordHVB = vertexData->
		vertexBufferBinding->getBuffer(texcoordVE->getSource());
	texcoords = (GLfloat*) texcoordHVB->lock(0, texcoordHVB->getSizeInBytes(), 
			HardwareBuffer::HBL_NORMAL);
    indexData = submesh->indexData;
	indexHB = indexData->indexBuffer ;
	vertexIndices = (unsigned short*) indexHB->lock(
		0, indexHB->getSizeInBytes(), HardwareBuffer::HBL_READ_ONLY);
	numFaces = indexData->indexCount / 3 ;

	// bottom left tower
	originalTextureCoords = windowMeshTextureCoords[LEFT_BOTTOM_MESH_NO];
	vertexIndexPosition = 0;
	for (int y=22;y>22-7;y--) {
		unsigned char *rowStart = inDisplayState + y * 54;
		for (int x=0;x<22;x++) {
			unsigned char pixelValue = *(rowStart + x);
			int vcount = 6; // change the next 6 vertixes that form 2 triangles = one window
			// lookup the vertices and change the texcoords
			while (vcount--)
			{
				unsigned short vertexIndex = vertexIndices[vertexIndexPosition]; // two texture coords per vertex
				int isUpperRight = (int)((originalTextureCoords[vertexIndex * 2]) + 0.2);
				texcoords[vertexIndex * 8 + 6] = windowMeshTextureValues[pixelValue][0][isUpperRight];
				
				isUpperRight = (int)((originalTextureCoords[vertexIndex * 2 + 1]) + 0.2);
				texcoords[vertexIndex * 8 + 7] = windowMeshTextureValues[pixelValue][1][isUpperRight];

				vertexIndexPosition++;
			}
		}
	}
	indexHB->unlock();
	texcoordHVB->unlock();

	// top left part
    submesh = topLeftWindows->getMesh()->getSubMesh(0);
    vertexData = submesh->vertexData;
	texcoordVE = vertexData->
		vertexDeclaration->findElementBySemantic(VES_TEXTURE_COORDINATES);
	texcoordHVB = vertexData->
		vertexBufferBinding->getBuffer(texcoordVE->getSource());
	texcoords = (GLfloat*) texcoordHVB->lock(0, texcoordHVB->getSizeInBytes(), 
			HardwareBuffer::HBL_NORMAL);
    indexData = submesh->indexData;
	indexHB = indexData->indexBuffer ;
	vertexIndices = (unsigned short*) indexHB->lock(
		0, indexHB->getSizeInBytes(), HardwareBuffer::HBL_READ_ONLY);
	numFaces = indexData->indexCount / 3 ;

	// top left part
	originalTextureCoords = windowMeshTextureCoords[LEFT_TOP_MESH_NO];
	vertexIndexPosition = 0;

	for (int y=22-9;y>5;y--) {
		unsigned char *rowStart = inDisplayState + y * 54;
		for (int x=0;x<22;x++) {
			unsigned char pixelValue = *(rowStart + x);
			int vcount = 6; // change the next 6 vertices that form 2 triangles = one window
			// lookup the vertices and change the texcoords
			while (vcount--)
			{
				unsigned short vertexIndex = vertexIndices[vertexIndexPosition]; // two texture coords per vertex
				int isUpperRight = (int)((originalTextureCoords[vertexIndex * 2]) + 0.2);
				texcoords[vertexIndex * 8 + 6] = windowMeshTextureValues[pixelValue][0][isUpperRight];
				
				isUpperRight = (int)((originalTextureCoords[vertexIndex * 2 + 1]) + 0.2);
				texcoords[vertexIndex * 8 + 7] = windowMeshTextureValues[pixelValue][1][isUpperRight];

				vertexIndexPosition++;
			}
		}
	}

	indexHB->unlock();
	texcoordHVB->unlock();

	// top right tower
    submesh = topRightWindows->getMesh()->getSubMesh(0);
    vertexData = submesh->vertexData;
	texcoordVE = vertexData->
		vertexDeclaration->findElementBySemantic(VES_TEXTURE_COORDINATES);
	texcoordHVB = vertexData->
		vertexBufferBinding->getBuffer(texcoordVE->getSource());
	texcoords = (GLfloat*) texcoordHVB->lock(0, texcoordHVB->getSizeInBytes(), 
			HardwareBuffer::HBL_NORMAL);
    indexData = submesh->indexData;
	indexHB = indexData->indexBuffer ;
	vertexIndices = (unsigned short*) indexHB->lock(
		0, indexHB->getSizeInBytes(), HardwareBuffer::HBL_READ_ONLY);
	numFaces = indexData->indexCount / 3 ;

	// top right tower
	originalTextureCoords = windowMeshTextureCoords[RIGHT_TOP_MESH_NO];
	vertexIndexPosition = 0;

	for (int y=11;y>=0;y--) {
		unsigned char *rowStart = inDisplayState + y * 54 + 54-30;
		for (int x=0;x<30;x++) {
			unsigned char pixelValue = *(rowStart + x);
			int vcount = 6; // change the next 6 vertixec that form 2 triangles = one window
			// lookup the vertices and change the texcoords
			while (vcount--)
			{
				unsigned short vertexIndex = vertexIndices[vertexIndexPosition]; // two texture coords per vertex
				int isUpperRight = (int)((originalTextureCoords[vertexIndex * 2]) + 0.2);
				texcoords[vertexIndex * 8 + 6] = windowMeshTextureValues[pixelValue][0][isUpperRight];
				
				isUpperRight = (int)((originalTextureCoords[vertexIndex * 2 + 1]) + 0.2);
				texcoords[vertexIndex * 8 + 7] = windowMeshTextureValues[pixelValue][1][isUpperRight];

				vertexIndexPosition++;
			}
		}
	}

	indexHB->unlock();
	texcoordHVB->unlock();



	// upper right tower
    submesh = bottomRightWindows->getMesh()->getSubMesh(0);
    vertexData = submesh->vertexData;
	texcoordVE = vertexData->
		vertexDeclaration->findElementBySemantic(VES_TEXTURE_COORDINATES);
	texcoordHVB = vertexData->
		vertexBufferBinding->getBuffer(texcoordVE->getSource());
	texcoords = (GLfloat*) texcoordHVB->lock(0, texcoordHVB->getSizeInBytes(), 
			HardwareBuffer::HBL_NORMAL);
    indexData = submesh->indexData;
	indexHB = indexData->indexBuffer ;
	vertexIndices = (unsigned short*) indexHB->lock(
		0, indexHB->getSizeInBytes(), HardwareBuffer::HBL_READ_ONLY);
	numFaces = indexData->indexCount / 3 ;

	// top right tower
	originalTextureCoords = windowMeshTextureCoords[RIGHT_BOTTOM_MESH_NO];
	vertexIndexPosition = 0;

	for (int y=22;y>22-9;y--) {
		unsigned char *rowStart = inDisplayState + y * 54 + 54-30;
		for (int x=0;x<30;x++) {
			unsigned char pixelValue = *(rowStart + x);
			int vcount = 6; // change the next 6 vertixec that form 2 triangles = one window
			// lookup the vertices and change the texcoords
			while (vcount--)
			{
				unsigned short vertexIndex = vertexIndices[vertexIndexPosition]; // two texture coords per vertex
				int isUpperRight = (int)((originalTextureCoords[vertexIndex * 2]) + 0.2);
				texcoords[vertexIndex * 8 + 6] = windowMeshTextureValues[pixelValue][0][isUpperRight];
				
				isUpperRight = (int)((originalTextureCoords[vertexIndex * 2 + 1]) + 0.2);
				texcoords[vertexIndex * 8 + 7] = windowMeshTextureValues[pixelValue][1][isUpperRight];

				vertexIndexPosition++;
			}
		}
	}

	indexHB->unlock();
	texcoordHVB->unlock();


}

- (void)initializeTextureCordsForMeshNo:(int)meshNo entity:(Entity *)inEntity
{
    SubMesh *submesh = inEntity->getMesh()->getSubMesh(0);
    VertexData *vertexData = submesh->vertexData;

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
	for(int i=0 ; i<indexData->indexCount ; i++, vertexIndices++) {
		int p0 = *vertexIndices;
		windowMeshTextureCoords[meshNo][p0*2]     = texcoords[p0 * 8 + 6];
		windowMeshTextureCoords[meshNo][p0*2 + 1] = texcoords[p0 * 8 + 6 + 1];
	}
	indexHB->unlock();

	normHVB->unlock();

}


- (void)connectionDidBecomeAvailable {

	[self fetchStreamsXML];

	if (![self hasConnection]) {
		if ([[[NSUserDefaults standardUserDefaults] objectForKey:@"autoselectProxy"] boolValue])
		{
			[self connectToAutoconnectProxy];
		}
	}

}

- (void)applicationWillFinishLaunching:(NSNotification *)notification
{   
	_lastAutoCameraTime = [NSDate timeIntervalSinceReferenceDate];
	_xmlReadFailureCount = 0;
	_connectionLostTime = DBL_MAX;
	_hostResolveFailureTime = DBL_MAX;
	
	_frameQueue = [NSMutableArray new];

	projectTableSections = [NSMutableArray new];
	NSArray *savedStreams = [[NSUserDefaults standardUserDefaults] objectForKey:@"blinkenArray"];
	if (savedStreams) {
		[self updateWithBlinkenstreams:savedStreams];
	}

	int maxcount = 23*54;
	int value = 0;
	for (int i=0;i<maxcount;i++)
	{
		*(((char *)displayState)+i)=value;
		value = (i / 54) % 16;
	}

//	[self resetTimeCompensation];

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
    
    topLeftWindows = ent;
    [self initializeTextureCordsForMeshNo:LEFT_TOP_MESH_NO entity:ent];

    
	// let us create an entity per window this should be easier to handle

    SceneNode* windowNodeA2 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    ent = mSceneMgr->createEntity("windowsA2", "WindowsA2.mesh");
    windowNodeA2->attachObject(ent);
    windowNodeA2->pitch(Degree(90)); 
    
    bottomLeftWindows = ent;
    [self initializeTextureCordsForMeshNo:LEFT_BOTTOM_MESH_NO entity:ent];

    SceneNode* windowNodeB1 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    ent = mSceneMgr->createEntity("windowsB1", "WindowsB1.mesh");
    windowNodeB1->attachObject(ent);
    windowNodeB1->pitch(Degree(90)); 
    
    topRightWindows = ent;
    [self initializeTextureCordsForMeshNo:RIGHT_TOP_MESH_NO entity:ent];


    SceneNode* windowNodeB2 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    ent = mSceneMgr->createEntity("windowsB2", "WindowsB2.mesh");
    windowNodeB2->attachObject(ent);
    windowNodeB2->pitch(Degree(90));

	bottomRightWindows = ent;
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
    
    [self updateWindows:(unsigned char *)displayState];

	// create a timer that causes OGRE to render
	NSTimer *renderTimer = [NSTimer scheduledTimerWithTimeInterval:FRAMERATE target:self selector:@selector(renderFrame) userInfo:NULL repeats:YES];
	[[NSRunLoop currentRunLoop] addTimer:renderTimer forMode:NSEventTrackingRunLoopMode];
	[self connectionDidBecomeAvailable];
}

- (void)connectToProxyFromArray:(NSArray *)inArray {
	NSUInteger count = [inArray count];
	if (count) {
		[self connectToProxy:[inArray objectAtIndex:random() % count]];
	}
}

- (void)connectToAutoconnectProxy {
	if (![self hasConnection]) {
		// collect all live proxies
		NSMutableArray *liveProxiesToChooseFrom = [NSMutableArray array];
		NSMutableArray *proxiesToChooseFrom = [NSMutableArray array];
		for (TableSection *section in [self projectTableSections]) {
			for (NSDictionary *proxy in [section items]) {
				if ([proxy valueForKey:@"kind"] &&
					[[proxy valueForKey:@"kind"] caseInsensitiveCompare:@"live"] == NSOrderedSame) {
					[liveProxiesToChooseFrom addObject:proxy];
				}
				[proxiesToChooseFrom addObject:proxy];
			}
		}
		if ([liveProxiesToChooseFrom count]) {
//			NSLog(@"%s choosing from live proxies %@",__FUNCTION__,liveProxiesToChooseFrom);
			[self connectToProxyFromArray:liveProxiesToChooseFrom];
		} else if ([proxiesToChooseFrom count]) {
//			NSLog(@"%s choosing from all %@",__FUNCTION__,proxiesToChooseFrom);
			[self connectToProxyFromArray:proxiesToChooseFrom];
		}
	}
}

- (void)handleConnectionFailure {
	BOOL autoconnect = [[[NSUserDefaults standardUserDefaults] objectForKey:@"autoselectProxy"] boolValue];
	if (autoconnect) {
		[self connectToAutoconnectProxy];
	}
}


- (void)connectToProxy:(NSDictionary *)inProxy {
//	NSLog(@"%s %@",__FUNCTION__,inProxy);
	NSString *address = [inProxy valueForKey:@"address"];
	if (address) {
		[_blinkenListener stopListening];
		[_hostToResolve cancel];
		[_hostToResolve setDelegate:nil];
		self.hostToResolve = nil;
		self.currentProxy = inProxy;
		[self setStatusText:[NSString stringWithFormat:@"Resolving %@",address]];
		
		// this is just done for asychronous resolving so we know we have an ip address for this one
		self.hostToResolve = [TCMHost hostWithName:address port:1234 userInfo:nil];
		
		[_hostToResolve setDelegate:self];
		_hostResolveFailureTime = [NSDate timeIntervalSinceReferenceDate] + HOST_RESOLVING_TIMEOUT;
		[_hostToResolve resolve];
		
		NSString *kindString = [inProxy objectForKey:@"kind"];
		if (!kindString) kindString = @"";
//		_liveLabel.text = kindString;
		_showTime = [[inProxy objectForKey:@"showtime"] isEqualToString:@"showtime"];
	}
}

- (void)selectProxy:(id)inMenuItem
{
	[self connectToProxy:[inMenuItem representedObject]];
}

- (BOOL)validateMenuItem:(NSMenuItem *)inMenuItem {
	if ([inMenuItem action] == @selector(selectProxy:)) {
		if ([self.currentProxy isEqualTo:[inMenuItem representedObject]])
		{
			[inMenuItem setState:NSOnState];
		} else {
			[inMenuItem setState:NSOffState];
		}
	}
	return YES;
}


- (void)connectToManualProxy
{
	[self connectToProxy:[self manualProxy]];
}


- (void)fetchStreamsXML
{
	NSURL *urlToFetch = [NSURL URLWithString:(_xmlReadFailureCount % 2) == 0 ? 
		@"http://www.blinkenlights.net/config/blinkenstreams.xml" : 
		@"http://www.blinkenlights.de/config/blinkenstreams.xml"];
	NSURLRequest *request = [NSURLRequest requestWithURL:urlToFetch cachePolicy:NSURLRequestReloadIgnoringLocalCacheData timeoutInterval:30.0];
	[_responseData release];
	_responseData = NULL;
	_responseData = [NSMutableData new];
	self.proxyListConnection = [[[NSURLConnection alloc] initWithRequest:request delegate:self] autorelease];
}



- (void)hostDidResolveAddress:(TCMHost *)inHost;
{
	NSString *addressString = [_currentProxy valueForKey:@"address"];
	id portValue = [_currentProxy valueForKey:@"port"];
	if (portValue) addressString = [addressString stringByAppendingFormat:@":%@",portValue];
	_fadeOutOnBlinkenframe = YES;
	[self setStatusText:[NSString stringWithFormat:@"Connecting to %@",addressString]];

	[[_hostToResolve retain] autorelease];
	[_hostToResolve cancel];
	[_hostToResolve setDelegate:nil];
	self.hostToResolve = nil;
	_hostResolveFailureTime = DBL_MAX;

	_connectionLostTime = [NSDate timeIntervalSinceReferenceDate] + CONNECTIION_NO_FRAME_TIMEOUT;

	// create this one lazyly
	if (!_blinkenListener) {
		_blinkenListener = [BlinkenListener new];
		[_blinkenListener setDelegate:self];
	}
	
	[_blinkenListener setProxyAddress:addressString];
	[_blinkenListener listen];
}

- (void)host:(TCMHost *)inHost didNotResolve:(NSError *)inError;
{
	[self setStatusText:[NSString stringWithFormat:@"Could not resolve %@",[(id)inHost name]]];
	[[_hostToResolve retain] autorelease];
	[_hostToResolve cancel];
	[_hostToResolve setDelegate:nil];
	self.hostToResolve = nil;
	_hostResolveFailureTime = DBL_MAX;
// 	NSLog(@"%s %@ %@",__FUNCTION__,inHost, inError);
 	[self handleConnectionFailure];
}

- (void)failHostResolving {
	if (self.hostToResolve) {
		[self host:self.hostToResolve didNotResolve:nil];
	}
}

- (void)consumeFrame
{
	NSArray *frames = [_frameQueue lastObject];
	for (BlinkenFrame *frame in frames)
	{
		CGSize frameSize = frame.frameSize;
		NSData *frameData = frame.frameData;
		BOOL isNibbles = frame.bitsPerPixel != 8;
		unsigned char *bytes = (unsigned char *)frameData.bytes;
		unsigned char *bytesEnd = bytes + frameData.length;
		NSUInteger bytesPerRow = (NSUInteger)frameSize.width;
		unsigned char maxValue = frame.maxValue;
		if (isNibbles) bytesPerRow = (bytesPerRow + 1) / 2;

		unsigned char screenID = frame.screenID;

		if (screenID == 0)
		{
			if ((int)frameSize.width == 96 && (int)frameSize.height == 32)
			{
				int rowIndex = 0;
				for (;bytes < bytesEnd;bytes+=bytesPerRow,rowIndex++)
				{
					if (rowIndex >= 5)
					{
						if (isNibbles)
						{
							int offset = 8;
							for (offset = 8; offset < 8+11; offset++)
							{
								displayState[rowIndex - 5][(offset-8)*2]   = (((int)*(bytes + offset)) >>  4) * 15 / maxValue;
								displayState[rowIndex - 5][(offset-8)*2+1] = (((int)*(bytes + offset)) & 0xf) * 15 / maxValue;
							}
							for (offset = 25; offset < 25+15; offset++)
							{
								displayState[rowIndex - 5][24+(offset-25)*2]   = (((int)*(bytes + offset)) >>  4) * 15 / maxValue;
								displayState[rowIndex - 5][24+(offset-25)*2+1] = (((int)*(bytes + offset)) & 0xf) * 15 / maxValue;
							}
						}
						else
						{
							int offset = 16;
							for (offset = 16; offset < 16+22; offset++)
							{
								displayState[rowIndex - 5][offset - 16] = ((int)*(bytes + offset)) * 15 / maxValue;
							}
							for (offset = 50; offset < 50+30; offset++)
							{
								displayState[rowIndex - 5][offset - 50 + 24] = ((int)*(bytes + offset)) * 15 / maxValue;
							}
						}
					}
					if (rowIndex >= 31 - 4) break;
				}
			}
		} 
		else if (screenID == 5)
		{
			// this is the left tower probably
			if ((int)frameSize.width == 22 && (int)frameSize.height == 17)
			{
				int rowIndex = 0;
				for (;bytes < bytesEnd;bytes+=bytesPerRow,rowIndex++)
				{
					if (isNibbles)
					{
						int x = 0;
						for (x = 0; x < 11; x++)
						{
							displayState[rowIndex + 6][x*2]   = (((int)*(bytes + x)) >>  4) * 15 / maxValue;
							displayState[rowIndex + 6][x*2+1] = (((int)*(bytes + x)) & 0xf) * 15 / maxValue;
						}
					}
					else
					{
						int x = 0;
						for (x = 0; x < 22; x++)
						{
							displayState[rowIndex + 6][x] = ((int)*(bytes + x)) * 15 / maxValue;
						}
					}
				}
			}
		}
		else if (screenID == 6)
		{
			// this is the right tower probably
			if ((int)frameSize.width == 30 && (int)frameSize.height == 23)
			{
				int rowIndex = 0;
				for (;bytes < bytesEnd;bytes+=bytesPerRow,rowIndex++)
				{
					if (isNibbles)
					{
						int x = 0;
						for (x = 0; x < 15; x++)
						{
							displayState[rowIndex][24+x*2]   = (((int)*(bytes + x)) >>  4) * 15 / maxValue;
							displayState[rowIndex][24+x*2+1] = (((int)*(bytes + x)) & 0xf) * 15 / maxValue;
						}
					}
					else
					{
						int x = 0;
						for (x = 0; x < 30; x++)
						{
							displayState[rowIndex][24+x] = ((int)*(bytes + x)) * 15 / maxValue;
						}
					}
				}
			}
		}
	}
	[_frameQueue removeLastObject];
	[self updateWindows:(unsigned char *)displayState];
}


- (void)blinkenListener:(BlinkenListener *)inListener receivedFrames:(NSArray *)inFrames atTimestamp:(uint64_t)inTimestamp
{
	if (_fadeOutOnBlinkenframe) {
		[self fadeoutStatusText];
		_fadeOutOnBlinkenframe = NO;
	}
	NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
	_connectionLostTime = now + CONNECTIION_NO_FRAME_TIMEOUT;
	// handle the frame

	[_frameQueue insertObject:inFrames atIndex:0];

	if (inTimestamp != 0) 
	{
		if (_showTime) {
			static NSDateFormatter *dateFormatter = nil;
			static NSDateFormatter *timeFormatter = nil;
			if (dateFormatter == nil) {
				dateFormatter = [NSDateFormatter new];
				[dateFormatter setTimeStyle:NSDateFormatterNoStyle];
				[dateFormatter setDateStyle:NSDateFormatterShortStyle];

				timeFormatter = [NSDateFormatter new];
				[timeFormatter setDateStyle:NSDateFormatterNoStyle];
				[timeFormatter setTimeStyle:NSDateFormatterShortStyle];
			}
			NSTimeInterval interval = ((NSTimeInterval)inTimestamp / 1000.0);
			NSDate *dateToDisplay = [NSDate dateWithTimeIntervalSince1970:interval];
			NSString *dateAndTimeString = [NSString stringWithFormat:@"%@\n%@",
				[dateFormatter stringForObjectValue:dateToDisplay],
				[timeFormatter stringForObjectValue:dateToDisplay]];
//			_liveLabel.text = dateAndTimeString;
		}
	
		NSTimeInterval now = [[NSDate date] timeIntervalSince1970] ;
		NSTimeInterval timeDifference = now - ((NSTimeInterval)inTimestamp / 1000.0);
		NSTimeInterval compensationTime = (_maxTimeDifference - timeDifference);
		if (_timeSamplesTaken > 5 && ABS(timeDifference - _maxTimeDifference) > 3.0) 
		{
			[self resetTimeCompensation];
		}
		_maxTimeDifference = MAX(timeDifference,_maxTimeDifference);
		if (_timeSamplesTaken <= 5 && compensationTime > 0)
		{
			_timeSamplesTaken++;
			[self consumeFrame];
		}
		else
		{
			// compensate
			[self performSelector:@selector(consumeFrame) withObject:nil afterDelay:compensationTime inModes:[NSArray arrayWithObjects: NSDefaultRunLoopMode,
NSModalPanelRunLoopMode, NSRunLoopCommonModes, nil]];
			_maxTimeDifference -= 0.01; // shrink the time difference again to catch up if we only had one hickup
		}
		// NSLog(@"%s ts:0x%016qx %@ now: %@ 0x%016qx",__FUNCTION__,inTimestamp,[NSDate dateWithTimeIntervalSince1970:inTimestamp / (double)1000.0],now, (uint64_t)([now timeIntervalSince1970] * 1000));
		//NSLog(@"time difference: %0.5f s - compensation %0.5f s",timeDifference,compensationTime);
	} else {
		[self consumeFrame];
	}
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
	_xmlReadFailureCount++;
	if (_xmlReadFailureCount % 2) {
		[self fetchStreamsXML];
	}
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)inData
{
//	NSLog(@"%s %@",__FUNCTION__,inData);
	[_responseData appendData:inData];
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection
{
	NSString *responseString = [[NSString alloc] initWithData:_responseData encoding:NSUTF8StringEncoding];

	// parse XML
	NSXMLParser *parser = [[NSXMLParser alloc] initWithData:_responseData];
	[parser setDelegate:self];
	[parser parse];
	[parser release];
//	NSLog(@"%s new xml proxy list:\n%@",__FUNCTION__, responseString);
	[responseString release];
	[_responseData release];
	_responseData = nil;
	if ([_blinkenStreamsArray count])
	{
		[self updateWithBlinkenstreams:_blinkenStreamsArray];
		[_blinkenStreamsArray release];
		_blinkenStreamsArray = nil;
		if (![self hasConnection]) {
			if ([[[NSUserDefaults standardUserDefaults] objectForKey:@"autoselectProxy"] boolValue])
			{
				[self connectToAutoconnectProxy];
			}
		}

	} else {
		_xmlReadFailureCount++;
		if (_xmlReadFailureCount % 2) {
			[self fetchStreamsXML];
		}
	}
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

- (BOOL)hasConnection {
	return [NSDate timeIntervalSinceReferenceDate] + CONNECTIION_NO_FRAME_TIMEOUT > _connectionLostTime;
}

#pragma mark -
#pragma mark XML Parsing

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qualifiedName attributes:(NSDictionary *)attributeDict
{
	static NSMutableArray *proxyArray = nil;
	       if ([elementName isEqualToString:@"blinkenstreams"]) {
		_blinkenStreamsArray = [NSMutableArray new];
	} else if ([elementName isEqualToString:@"project"]) {
		if ([attributeDict objectForKey:@"name"]) {
			proxyArray = [NSMutableArray array];
			NSMutableDictionary *projectDict = [[attributeDict mutableCopy] autorelease];
			[projectDict setObject:proxyArray forKey:@"proxies"];
			[_blinkenStreamsArray addObject:projectDict];
		}
	} else if ([elementName isEqualToString:@"proxy"]) {
		[proxyArray addObject:attributeDict];
	} else if ([elementName isEqualToString:@"message"]) {
		NSMutableDictionary *messageDict = [attributeDict mutableCopy];
		
		self.messageDictionary = messageDict;
		[messageDict release];
	}
}

- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string 
{
	if (_messageDictionary && ![[_messageDictionary objectForKey:@"messageWasShown"] boolValue]) {
		NSMutableString *messageText = [_messageDictionary objectForKey:@"_messageText"];
		if (!messageText) {
			messageText = [NSMutableString string];
			[_messageDictionary setObject:messageText forKey:@"_messageText"];
		}
		[messageText appendString:string];
	}
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName
{
	if ([elementName isEqualToString:@"message"]) {
		NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
		if ([defaults integerForKey:@"lastShownMessageNumber"] >= [[_messageDictionary objectForKey:@"message-number"] intValue])
		{
			// now show - already seen
			return;
		}
		
		NSString *messageTitle = [_messageDictionary objectForKey:@"title"];
		if (!messageTitle) messageTitle = @"Message";
		NSString *messageText = [[_messageDictionary objectForKey:@"_messageText"] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
		BOOL hasURL = [_messageDictionary objectForKey:@"url"] != 0;
		NSString *urlTitle = [_messageDictionary objectForKey:@"url-title"];
		if (!urlTitle) urlTitle = @"Goto Site";
		NSAlert *alert = [NSAlert alertWithMessageText:messageTitle defaultButton:@"OK" alternateButton:hasURL ? urlTitle : nil otherButton:nil informativeTextWithFormat:@"%@",messageText];
		[alert beginSheetModalForWindow:nil modalDelegate:self didEndSelector:@selector(messageAlertDidEnd:returnCode:contextInfo:) contextInfo:[[_messageDictionary objectForKey:@"message-number"] retain]];
	}
}

- (void)messageAlertDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
//	NSLog(@"%s",__FUNCTION__);
	if ([_messageDictionary objectForKey:@"url"] && NSAlertAlternateReturn == returnCode) 
	{
		[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[_messageDictionary objectForKey:@"url"]]];
	}
	NSNumber *number = (NSNumber *)contextInfo;
	[[NSUserDefaults standardUserDefaults] setObject:number forKey:@"lastShownMessageNumber"];
	[number autorelease];
}


- (void)renderFrame
{
	// check connection
	NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];

	if (now > _connectionLostTime) {
		// connection is lost
		_connectionLostTime = DBL_MAX;
		[self setStatusText:@"No Frames"];
		[_blinkenListener stopListening];
		[self handleConnectionFailure];
	}

	if (now > _hostResolveFailureTime) {
		// connection is lost
		[self failHostResolving];
	}

    if (autoCam && progress>=1) {
        // Choose next cam after random wait
        if (_lastAutoCameraTime + MINCAMERAPOSITIONTIME < [NSDate timeIntervalSinceReferenceDate] &&
        	rand() % 100 == 1) {
//        NSLog(@"AUTO!");
        	_lastAutoCameraTime = [NSDate timeIntervalSinceReferenceDate];

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

- (void)fadeoutStatusText
{
	[_ibWindow setTitle:[NSString stringWithFormat:@"Stereoscope Simulator"]];
}

-(void)setStatusText:(NSString *)aStatusText
{
	[_ibWindow setTitle:[NSString stringWithFormat:@"Stereoscope Simulator - %@",aStatusText]];
}

#pragma mark -

- (IBAction)openBlinkenlightsHomepage:(id)inSender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://blinkenlights.net/"]];
}

- (IBAction)openStereoscopeHomepage:(id)inSender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://blinkenlights.net/stereoscope"]];
}

- (IBAction)openBlinkenlightsBlog:(id)inSender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://blinkenlights.net/blog"]];
}



@end
