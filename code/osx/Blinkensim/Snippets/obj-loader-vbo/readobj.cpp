//#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include <SDL.h>

#include <iostream>
#include <unistd.h>

#include "Obj.h"
#include "assert.h"

#define USE_VBL_SYNC 1

using namespace std;

// FIXME: This is of course a huge mess. DO NOT DO THIS IN REAL CODE! ;)
float rotation = 180, rotation_x = 0;
float translation = 0.0;
Obj * obj;
bool rotating_left = false, 
  rotating_right = false, 
  rotating_up = false, 
  rotating_down = false;
bool translating_forward = false, translating_backwards = false;
bool animating = false;

void myinit(int width, int height) 
{
  glClearColor(0.9f, 0.9f, 1.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);  
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glViewport(0, 0, width, height);  

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, (float)width/(float)height, 0.1, 1000.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  GLfloat light_position[] = { 2.0, 2.0, 2.0, 0.0 };
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  float dims[6];
  obj->getBoundingBox(dims);

#if 1
  cout << "Bounding box: " 
       << dims[0] << ", " 
       << dims[1] << ", " 
       << dims[2] << ", " 
       << dims[3] << ", " 
       << dims[4] << ", " 
       << dims[5] << endl;
#endif

  // FIXME: "viewall"
  gluLookAt(0, 0, -2*(dims[5]-dims[2]),
            0, 0, 0, 
            0, 1, 0);
}

void mydisplay()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
  glPushMatrix();

  glTranslatef(0.0, 0.0, translation);
  glRotatef(rotation, 0, 1, 0);
  glRotatef(rotation_x, 1, 0, 0);

  // center object
  float center[3];
  obj->getCenter(center);
  glTranslatef(-center[0], -center[1], -center[2]);

  obj->render();

  glPopMatrix();
}

bool processEvents() 
{
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) return false;
    if (event.type == SDL_KEYDOWN) {
      switch(event.key.keysym.sym) {
      case SDLK_ESCAPE:
        return false;
        break;
      case SDLK_LEFT:
        rotating_left = true;
        break;
      case SDLK_RIGHT:
        rotating_right = true;
        break;
      case SDLK_UP:
        rotating_up = true;
        break;
      case SDLK_DOWN:
        rotating_down = true;
        break;
      case SDLK_w:
        translating_forward = true;
        break;
      case SDLK_s:
        translating_backwards = true;
        break;
      case SDLK_a:
        animating = !animating;
        break;
      case SDLK_1:
        cout << "Setting render mode to IMMEDIATE." << endl;
        obj->setRenderMode(Obj::IMMEDIATE);
        break;
      case SDLK_2:
        cout << "Setting render mode to LIST." << endl;
        obj->setRenderMode(Obj::LIST);
        break;
      case SDLK_3:
        cout << "Setting render mode to ARRAY." << endl;
        obj->setRenderMode(Obj::ARRAY);
        break;
      case SDLK_4:
        cout << "Setting render mode to VBO." << endl;
        obj->setRenderMode(Obj::VBO);
        break;
      default:
        break;
      }
    }
    else if (event.type == SDL_KEYUP) {
      switch(event.key.keysym.sym) {
      case SDLK_LEFT:
        rotating_left = false;
        break;
      case SDLK_RIGHT:
        rotating_right = false;
        break;
      case SDLK_UP:
        rotating_up = false;
        break;
      case SDLK_DOWN:
        rotating_down = false;
        break;
      case SDLK_w:
        translating_forward = false;
        break;
      case SDLK_s:
        translating_backwards = false;
        break;
      default:
        break;
      }
    }
  }
  return true;
}

void update_animation(float delta)
{
  delta /= 10.0f;
  float factor = 0.0f;

  // automatic animation 
  if (animating) {
    rotation += delta;
    if (rotation > 360) rotation -= 360;
    else if (rotation < 0) rotation += 360;
  }

  // interaction
  if (rotating_left) factor = delta;
  else if (rotating_right) factor = -delta;
  else factor = 0.0f;
  rotation = rotation + factor;
  if (rotation > 360) rotation -= 360;
  else if (rotation < 0) rotation += 360;

  if (rotating_up) factor = delta; 
  else if (rotating_down) factor = -delta; 
  else factor = 0.0f;
  rotation_x = rotation_x + factor;
  if (rotation_x > 360) rotation_x -= 360;
  else if (rotation_x < 0) rotation_x += 360;

  if (translating_backwards) factor = -delta;
  else if (translating_forward) factor = delta;
  else factor = 0.0f;
  translation += factor;
}

int main(int argc, char ** argv)
{
  assert(argc >= 2);

  Obj::rendermode mode = Obj::IMMEDIATE; 

  if (argc > 2) { 
    if (strcmp(argv[1], "-l") == 0) {
      cout << "Rendering as list" << endl;
      mode = Obj::LIST;
    } else if (strcmp(argv[1], "-va") == 0) {
      cout << "Rendering as vertex array" << endl;
      mode = Obj::ARRAY;
    } else if (strcmp(argv[1], "-vbo") == 0) {
      cout << "Rendering as VBO" << endl;
      mode = Obj::VBO;
    } 
    argv[1] = argv[argc-1];
  }

  int width = 800, height = 600;
  //int width = 1024, height = 786;

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    cerr << "Unable to init SDL: " << SDL_GetError() << endl;
    return -1;
  }

#if USE_VBL_SYNC
  cout << "Turning on VBL sync." << endl;
  SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
#endif

  if (!SDL_SetVideoMode(width, height, 32, SDL_OPENGL)) {
    cerr << "Unable to set SDL video mode: " << SDL_GetError() << endl;
    SDL_Quit();
    return -2;
  }
  SDL_WM_SetCaption("OBJ loader test", NULL);  

  //glewInit();

  unsigned int x1 = SDL_GetTicks();
  obj = new Obj(argv[1]);
  unsigned int x2 = SDL_GetTicks();
  cout << "Time to create object: " << x2-x1 <<" msecs." << endl;

  myinit(width, height);  // initialize OpenGL
  obj->setRenderMode(mode);

  // obj->setShadingModel(Obj::FLAT);
  
  bool running = true;
  int ctr=0;
  unsigned int now, prev = SDL_GetTicks();
  while (running) {
    mydisplay();
    SDL_GL_SwapBuffers();
    running = processEvents(); 
    now = SDL_GetTicks();
#if 0
    ++ctr;
    if (ctr == 10) {
      //cout << "frametime: " << now-prev << endl;
      cout << 1000.0/double(now-prev) << " FPS" << endl;      
      ctr = 0;
    }
#endif
    update_animation(now-prev);
    prev = now;
  }

  SDL_Quit();
  delete obj;
  return 0;
}
