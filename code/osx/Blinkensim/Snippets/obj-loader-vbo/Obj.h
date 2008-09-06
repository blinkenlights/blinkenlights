#ifndef OBJ_H
#define OBJ_H

// A very basic reader for OBJ files
// format specification: http://www.robthebloke.org/source/obj.html

// LIMITATIONS 
// - no support for splines and friends
// - assumes 3d vertices and normals and 2d texture coordinates
// - normal generation does not take smoothing groups into account

// TODO
// - support for indexed vertex arrays
// - smoothing group support in normal generation

// kyrah@kyrah.net

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <string>
#include <vector>
#include <map>

#include <assert.h>

class Obj {
public:
  Obj(const std::string & filename);
  virtual ~Obj();
  void render();
  inline void getBoundingBox(float bbox[6]) const { 
    for (int i=0; i<6; i++) { bbox[i] = this->bbox[i]; }
  }
  inline void getCenter(float center[3]) const {
    for (int i=0; i<3; i++) { center[i] = this->center[i]; }
  }
  typedef enum {IMMEDIATE = 0, LIST = 1, ARRAY = 2, VBO = 3} rendermode;
  inline void setRenderMode(rendermode mode) { this->mode = mode; }
  typedef enum {FLAT = 0, SMOOTH = 1} shadingmodel;
  inline void setShadingModel(shadingmodel shading) { this->shading = shading; }

protected:
  void calculateBoundingBox();
  void generateNormals(bool generatePerVertexNormals, float creaseAngle = 85);
  void generateFaceNormal(struct Face *);
  void generateVertexNormal(int fi, std::vector<struct Face *> faces, int idx, float cos_angle);
  void initGL();
  void createDisplayList();
  void createVertexArray(struct MaterialGroup * mg);
  void createVBO(struct MaterialGroup * mg);
  void doRender() const;  
  void renderArrays(const struct MaterialGroup * mg) const;
  void renderImmediate(const struct MaterialGroup * mg) const;
  void printDebugInfo() const;

private:
  friend class ObjParser;
  class ObjParser * parser;
  std::vector<float> vertices;
  std::vector<float> normals;
  std::vector<float> texcoords;
  std::map<std::string, struct MaterialGroup *> materialgroups;
  struct MaterialGroup * m; // current material
  float bbox[6];
  float center[3];
  rendermode mode;
  shadingmodel shading;
  GLuint list;
  bool initialised;
};

// FIXME: This doesn't store any data, so should maybe be a singleton
// shared by all objects?
class ObjParser {
public:
  void readFile(const std::string & filename, Obj * obj);
protected:
  bool readMTLFile(const std::string & filename, Obj * obj);
  void parseFace(const std::string & line, Obj * obj);
  void triangulateFace(struct Face * face, std::vector<struct Face *> & faces);
};

struct Face {
  int vertexcount;
  float facenormal[3];
  std::vector<int> vertex;
  std::vector<int> texcoord;
  std::vector<int> normal;
};

struct Material {
  float ambient[3];
  float diffuse[3];
  float specular[3];
  float shininess[1];
  class Texture * texture;
};

struct MaterialGroup {
  Material material;
  std::vector<Face *> triangles; 
  float * tv;
  float * tt;
  float * tn;
  int ntv;
  GLuint vboid;
};

#endif // OBJ_H
