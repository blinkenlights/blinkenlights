# Uncomment CFLAGS and LIBS below as appropriate

# Mac OS X with SDL installed as framework
# CFLAGS = -I/Library/Frameworks/SDL.framework/Headers \
           -I/Library/Frameworks/SDL_Image.framework/Headers SDLMain.m
# LIBS = -framework SDL -framework SDL_Image -framework Cocoa -framework OpenGL

# Mac OS X with SDL installed as .dylib
CFLAGS =  `sdl-config --cflags`
LIBS = `sdl-config --libs` -framework OpenGL -lSDL_image

# other Unix 
#CFLAGS =  `sdl-config --cflags`
#LIBS = `sdl-config --libs` -lGL -lGLU -lGLEW -lSDL_image

all: readobj

%:%.cpp Obj.cpp ObjParser.cpp Texture.cpp
	g++ -g $(CFLAGS) $^ $(LIBS) -o $@
