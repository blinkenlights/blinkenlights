
#ifndef APPLICATION_H_
#define APPLICATION_H_

//
// 
// 
//
class CShell
{
public:
	// initialization before the render API is intialized
	bool InitApplication();
	
	// release any memory/resources acquired by InitApplication()
	bool QuitApplication();

	// It is called any time the rendering API is initialised,
	// i.e. once at the beginning, and possibly again if the
	// resolution changes, or a power management even occurs, or
	// if the app requests a reinialisation.
	// The application should check here the configuration of
	// the rendering API; it is possible that requests made in
	// InitApplication() were not successful.
	// Since everything is initialised when this function is
	// called, you can load textures and perform rendering API
	// functions.
	bool InitView();

	// It will be called after the RenderScene() loop, before
	// shutting down the render API. It enables the application
	// to release any memory/resources acquired in InitView().
	bool ReleaseView();

	// update the camera matrix and other things that need to be done to setup rendering
	bool UpdateScene();

	// It is main application function in which you have to do your own rendering.  Will be
	// called repeatedly until the application exits.
	bool RenderScene();
};


#endif APPLICATION_H_
