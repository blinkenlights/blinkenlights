#include "Texture.h"

#include <string>
#include <iostream>

using namespace std;

// ---------------------- public interface --------------------------

Texture::~Texture()
{
  if (img) SDL_FreeSurface(img);
}

bool Texture::load() 
{
  return (this->loadSDL() && this->loadGL());
}

// --------------------------- internal  ----------------------------

bool Texture::loadSDL()
{
  SDL_Surface * tmp_img = IMG_Load(this->filename.data());

  if (!tmp_img) {
    cerr << "Loading texture image " << this->filename << " failed." << endl;
    return false;
  }

  if (tmp_img->format->BitsPerPixel == 8 || 
      tmp_img->format->BitsPerPixel == 32) {
    img = tmp_img;
  } else {
    img = SDL_CreateRGBSurface(SDL_SWSURFACE, tmp_img->w, tmp_img->h, 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                               0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
                               0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif
                               );
    SDL_BlitSurface(tmp_img, NULL, img, NULL);
    SDL_FreeSurface(tmp_img);

    SDL_LockSurface(img);
    Uint8 * newpixels = (Uint8*)malloc(img->pitch*img->h);
    Uint8 * imgpixels = (Uint8 *)img->pixels;
    // flip image vertically (SDL's coordinate system has its origo at the
    // upper left corner, while OpenGL's origo is at the lower left corner)
    for (int i=0; i<img->h; i++) {
      for (int j=0; j<img->pitch; j++) {
        newpixels[(img->h-1-i)*img->pitch+j] = imgpixels[i*img->pitch+j];
      }
    }
    memcpy(img->pixels, newpixels, img->pitch*img->h);
    SDL_UnlockSurface(img);
  }
  return true;
}

static bool check_for_GL_error(const char * msg)
{
  bool all_ok = true;
  GLenum errCode;
  const GLubyte *errString;
  while ((errCode = glGetError()) != GL_NO_ERROR) {
    all_ok = false;
    errString = gluErrorString(errCode);
    cerr << "OpenGL Error: " << msg << errString << endl;
  }
  return all_ok;
}


bool Texture::loadGL()
{
  // cout << "Attempting to upload texture " << this->filename << " to GL." << endl;

  glGenTextures(1, &id);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D, id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  if (!check_for_GL_error("before uploading texture")) { return false; }
  glTexImage2D(GL_TEXTURE_2D, 0, this->format(), this->w(), this->h(), 
               0, this->format(), GL_UNSIGNED_BYTE, this->pixels());
  if (!check_for_GL_error("after uploading texture")) { return false; }
  return true;
}

GLint
Texture::format() const
{
  switch (img->format->BitsPerPixel) {
  case 8:
    return GL_LUMINANCE;
  case 32:
    return GL_RGBA;
  default:
    return -1;
  }
}
