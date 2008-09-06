Basic loader for OBJ files (see comment in Obj.h)

Instructions:
 - edit Makefile to fit your system
 - make
 - ./readobj data/teapot.obj

Dependencies:
 - SDL
 - SDL_Image
 - OpenGL

Usage:
 - Include Obj.cpp, ObjParser.cpp, and Texture.cpp in your project.
 - To load and render an OBJ file:

     #include "Obj.h"
     Obj * obj = new Obj("my_model.obj");
     obj->render();
     delete obj;

License: BSD
 - This basically means: do with this code as you please, but 
   there is no warranty that it works or even that it won't eat 
   your data. (If it breaks, you can keep both halves.)

Have fun!
  <kyrah@kyrah.net>

