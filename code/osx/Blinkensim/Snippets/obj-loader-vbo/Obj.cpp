//#include <GL/glew.h>

#include "Obj.h"
#include "Texture.h"

#include <iostream>
#include <math.h>
#include <list>

using namespace std;

// Normalize vector in place
static void normalize(float * normal)
{
  float len = (float)(sqrt((normal[0] * normal[0]) 
                         + (normal[1] * normal[1]) 
                         + (normal[2] * normal[2])));
  if (len == 0.0f) len = 1.0f;
  normal[0] /= len;
  normal[1] /= len;
  normal[2] /= len;
}

// Return the dot product of a and b
static double dot(float * a, float * b)
{
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

// debug
static void printVector(const float * v, const string & msg = "") 
{
  cout << msg << v[0] << ", " << v[1] << ", " << v[2] << endl;
}

// Initialise mg's material with meaningful(?) default values.
static void initMaterial(MaterialGroup * mg)
{
  mg->tv = mg->tn = mg->tt = NULL;
  for (int i=0; i<3; i++) { 
    mg->material.ambient[i] = 0.8f;
    mg->material.diffuse[i] = 0.8f;
    mg->material.specular[i] = 0.8f;
  }
  mg->material.shininess[0] = 100.0f;
  mg->material.texture = NULL;
}

// Read object from filename. We will attempt to create normal vectors
// if none are specified in the file.
Obj::Obj(const string & filename):
  mode(IMMEDIATE), shading(SMOOTH), initialised(false)
{
  this->m = this->materialgroups["_default"] = new MaterialGroup;
  initMaterial(this->m);
  this->parser = new ObjParser;
  this->parser->readFile(filename, this);
  cout << "Loaded file " << filename << endl;
  this->calculateBoundingBox();
  // Generate per-vertex normals only if none are specified in the file.
  this->generateNormals((this->normals.size() == 0));
  // this->printDebugInfo();
}

// Destructor. No surprises here.
Obj::~Obj() 
{ 
  delete this->parser;
  while (!this->materialgroups.empty()) {
    MaterialGroup * mg = this->materialgroups.begin()->second;
    if (mg->tv) delete [] mg->tv;
    if (mg->tn) delete [] mg->tn;
    if (mg->tt) delete [] mg->tt;
    delete mg->material.texture;
    glDeleteBuffersARB(1, &mg->vboid);
    while (!mg->triangles.empty()) {
      delete *(mg->triangles.begin()); 
      mg->triangles.erase(mg->triangles.begin());
    }
    delete mg;
    this->materialgroups.erase(this->materialgroups.begin());
  }
} 

// Calculate the object's axis-aligned bounding box.
void
Obj::calculateBoundingBox()
{
  this->bbox[0] = this->bbox[3] = this->vertices[0];
  this->bbox[1] = this->bbox[4] = this->vertices[1];
  this->bbox[2] = this->bbox[5] = this->vertices[2];
  
  for (int i=0; i<this->vertices.size(); i+=3) {
    if (this->vertices[i] <= this->bbox[0]) this->bbox[0] = this->vertices[i];
    else if (this->vertices[i] > this->bbox[3]) this->bbox[3] = this->vertices[i];
    if (this->vertices[i+1] <= this->bbox[1]) this->bbox[1] = this->vertices[i+1];
    else if (this->vertices[i+1] > this->bbox[4]) this->bbox[4] = this->vertices[i+1];
    if (this->vertices[i+2] <= this->bbox[2]) this->bbox[2] = this->vertices[i+2];
    else if (this->vertices[i+2] > this->bbox[5]) this->bbox[5] = this->vertices[i+2];
  }

  this->center[0] = (this->bbox[3]+this->bbox[0])/2.0;
  this->center[1] = (this->bbox[4]+this->bbox[1])/2.0;
  this->center[2] = (this->bbox[5]+this->bbox[2])/2.0;
}

// Calculate per-face and per-vertex normals. 
void 
Obj::generateNormals(bool generatePerVertexNormals, float creaseAngle)
{
  // First, calculate per-face normals
  cout << "Calculating per-face normals" << endl;
  map<string, MaterialGroup *>::iterator miter;
  for (miter = this->materialgroups.begin(); miter != this->materialgroups.end(); miter++) {
    MaterialGroup * mg = miter->second;
    if (mg->triangles.size() == 0) continue; 
    for (int j=0; j<mg->triangles.size(); j++) {
      generateFaceNormal(mg->triangles[j]);
    }
  }

  if (!generatePerVertexNormals) return;

  // Secondly, generate smooth per-vertex normals

  // For each vertex, build a list of triangles in which this vertex is used.
  // Then go through the list of vertices and for each:
  //   go through the list of faces using that vertex and for each:
  //     calculate the vertex normal by averaging the per-face normals
  //     set the normal index in the triangle to the generated smooth normal

  cout << "Calculating per-vertex normals" << endl;
  vector<Face *> faces[this->vertices.size()/3];
  vector<int> faceindices[this->vertices.size()/3];
  for (miter = this->materialgroups.begin(); miter != this->materialgroups.end(); miter++) {
    MaterialGroup * mg = miter->second;
    if (mg->triangles.size() == 0) continue; 
    for (int j=0; j<mg->triangles.size(); j++) {
      Face * f = mg->triangles[j];
      for (int k=0; k<3; k++) {
        faces[f->vertex[k]].push_back(f);
        faceindices[f->vertex[k]].push_back(k); // remember if we are vertex 0, 1 or 2 in face
      }
    }
  }
  float cos_angle = cos(creaseAngle * M_PI / 180.0);
  double dp;
  float average[3];
  for (int i=0; i<this->vertices.size()/3; i++) {
    if (faces[i].size() == 0) continue;
    for (int j=0; j<faces[i].size();j++) {
      generateVertexNormal(j, faces[i], faceindices[i][j], cos_angle);
    }
  }
}

// Generate per-face normal for face. (The vector will be normalized.)
void
Obj::generateFaceNormal(Face * face)
{
  float v[9], a[3], b[3];
  float normal[3];

  // lookup vertices
  for (int k=0; k<3; k++) {
    for (int l=0; l<3; l++) {
      v[3*k+l] = this->vertices[3*face->vertex[k]+l];
    }
  }

  // vector v0 -> v1
  a[0] = v[0] - v[3];
  a[1] = v[1] - v[4];
  a[2] = v[2] - v[5];

  // vector v1 -> v2
  b[0] = v[3] - v[6]; 
  b[1] = v[4] - v[7]; 
  b[2] = v[5] - v[8]; 

  // cross product
  normal[0] = (a[1] * b[2]) - (a[2] * b[1]);
  normal[1] = (a[2] * b[0]) - (a[0] * b[2]);
  normal[2] = (a[0] * b[1]) - (a[1] * b[0]);

  normalize(normal);

  for (int i=0; i<3; i++) {
    face->facenormal[i] = normal[i];
  }
}

// Generate one per-vertex normal for one face 
//   thefaces is a vector of faces sharing one vertex
//   fi is the index of the current face within thefaces
//   idx is the index of the current vertex in the face
//   cos_angle is the creaseAngle in radians
void
Obj::generateVertexNormal(int fi, vector<Face *> thefaces, int idx, float cos_angle)
{
  Face * face = thefaces[fi];
  face->normal[idx] = 0;

  float average[3]; // start with face normal
  for (int i=0; i<3; i++) { average[i] = face->facenormal[i]; }

  for (int j=0; j<thefaces.size();j++) {
    if (fi != j) {
      float dp = dot(face->facenormal, thefaces[j]->facenormal);
      if (dp > cos_angle) {
        average[0] += thefaces[j]->facenormal[0];
        average[1] += thefaces[j]->facenormal[1];
        average[2] += thefaces[j]->facenormal[2];
      }
    }
  }

  normalize(average);

  // check if normal vector is already in list
  // FIXME: use a hash map for faster lookup!
  for (int i=0; i<this->normals.size()/3; i++) {
    if (this->normals[3*i] == average[0] &&
        this->normals[3*i+1] == average[1] &&
        this->normals[3*i+2] == average[2]) {
      face->normal[idx] = i;
      break;
    }
  }
  if (face->normal[idx] == 0) {
    this->normals.push_back(average[0]);
    this->normals.push_back(average[1]);
    this->normals.push_back(average[2]);
    face->normal[idx] = this->normals.size()/3 - 1;
  }
}

// OpenGL initialisation: create display lists, vertex arrays, and VBOs
void 
Obj::initGL()
{
  this->createDisplayList();

  // Create vertex arrays and VBOs
  map<string, MaterialGroup *>::const_iterator miter;
  for (miter = this->materialgroups.begin(); miter != this->materialgroups.end(); miter++) {
    MaterialGroup * mg = miter->second;
    if (mg->triangles.size() == 0) continue; 
    createVertexArray(mg);
    createVBO(mg);
  }
  this->initialised = true;
}

// Create and initialise display list.
void
Obj::createDisplayList()
{
  rendermode oldmode = this->mode;
  this->mode = LIST;
  list = glGenLists(1);
  glNewList(list, GL_COMPILE);
  doRender();
  glEndList();
  this->mode = oldmode;
}

// Create vertex array for the primitives in mg.
void
Obj::createVertexArray(MaterialGroup * mg)
{
  if (mg->triangles.size() == 0) {
    mg->ntv = 0;
    return;
  }

  std::list<float> lf, lt, ln;

  for (int j=0; j<mg->triangles.size(); j++) {
    for (int k=0; k<3; k++) {

      if (this->shading == FLAT) {
        ln.push_back(mg->triangles[j]->facenormal[0]);
        ln.push_back(mg->triangles[j]->facenormal[1]);
        ln.push_back(mg->triangles[j]->facenormal[2]);
      } else {
        if (mg->triangles[j]->normal[k] < 0) {
          static bool first = true;
          if (first) {
            cout << "### Warning, index: " << mg->triangles[j]->normal[k] << endl;
            first = false;
          }
        } else{
          ln.push_back(this->normals[3*mg->triangles[j]->normal[k]]);
          ln.push_back(this->normals[3*mg->triangles[j]->normal[k]+1]);
          ln.push_back(this->normals[3*mg->triangles[j]->normal[k]+2]);
        }
      }

      if (mg->triangles[j]->texcoord[0] != -1) { // do we have valid texture coordinates?
        lt.push_back(this->texcoords[2*mg->triangles[j]->texcoord[k]]);
        lt.push_back(this->texcoords[2*mg->triangles[j]->texcoord[k]+1]);
      }
 
      lf.push_back(this->vertices[3*mg->triangles[j]->vertex[k]]);
      lf.push_back(this->vertices[3*mg->triangles[j]->vertex[k]+1]);
      lf.push_back(this->vertices[3*mg->triangles[j]->vertex[k]+2]);
    }
  }

  mg->tv = new float[lf.size()];
  copy(lf.begin(), lf.end(), mg->tv);
  mg->tn = new float[ln.size()];
  copy(ln.begin(), ln.end(), mg->tn);
  mg->tt = new float[lt.size()];
  copy(lt.begin(), lt.end(), mg->tt);

  mg->ntv = lf.size()/3.0;
}

// Create VBO for the primitives in mg.
void
Obj::createVBO(MaterialGroup * mg)
{
  // each triangle has 3 floats/vertex and 3 floats/normal
  GLsizei size = 2 * mg->ntv * 3 * sizeof(float); 
  if (mg->material.texture) size += mg->ntv * 2 * sizeof(float); // tex coords
  glGenBuffersARB(1, &mg->vboid);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, mg->vboid);
  glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, 0, GL_STATIC_DRAW_ARB);
  glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, mg->ntv*3*sizeof(float), mg->tv); 
  glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, mg->ntv*3*sizeof(float), mg->ntv*3*sizeof(float), mg->tn);
  if (mg->material.texture) { 
    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 2*mg->ntv*3*sizeof(float), mg->ntv*2*sizeof(float), mg->tt);
  }
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, NULL);
}

// Public render method. 
void 
Obj::render()
{
  // Delay initialisation because we might not have a valid OpenGL
  // context when the model is set up initially.
  if (!this->initialised) this->initGL();

  if (this->mode == LIST) {
    glCallList(list);
  } else if (this->mode == IMMEDIATE) {
    this->doRender();
  } else if (this->mode == ARRAY || this->mode == VBO) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    this->doRender();
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
  } else {
    assert(0 && "unknown render mode ");
  }
}


// Loops through all the materialgroups and renders the associated primitives.
void 
Obj::doRender() const
{
  // cout << "Rendering in immediate mode." << endl;
  map<string, MaterialGroup *>::const_iterator miter;
  for (miter = this->materialgroups.begin(); miter != this->materialgroups.end(); miter++) {
    MaterialGroup * mg = miter->second;
    if (mg->triangles.size() == 0) continue; 
    glMaterialfv(GL_FRONT, GL_AMBIENT, mg->material.ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mg->material.diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mg->material.specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mg->material.shininess);
    if (mg->material.texture) {
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, mg->material.texture->id);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    } else {
      glDisable(GL_TEXTURE_2D);
    }
    if (this->mode == IMMEDIATE|this->mode == LIST) {
      renderImmediate(mg);
    } else {
      this->renderArrays(mg);
    }
  }
}

// Render in immediate mode.
void
Obj::renderImmediate(const MaterialGroup * mg) const
{
  if (mg->triangles.size() == 0) return;
  assert(mg->triangles[0]->vertexcount == 3 && "unknown face type");

  for (int j=0; j<mg->triangles.size(); j++) {
    glBegin(GL_TRIANGLES);

    for (int k=0; k<3; k++) {
      if (this->shading == FLAT) {
        glNormal3f(mg->triangles[j]->facenormal[0],
                   mg->triangles[j]->facenormal[1],
                   mg->triangles[j]->facenormal[2]);
      } else {
        if (mg->triangles[j]->normal[k] < 0) {
          static bool first = true;
          if (first) {
            cout << "### Warning, index: " << mg->triangles[j]->normal[k] << endl;
            first = false;
          }
        } else{
          glNormal3f(this->normals[3*mg->triangles[j]->normal[k]], 
                     this->normals[3*mg->triangles[j]->normal[k]+1], 
                     this->normals[3*mg->triangles[j]->normal[k]+2]);
        }
      }

      if (mg->triangles[j]->texcoord[0] != -1) { 
        // do we have valid texture coordinates?
        glTexCoord2f(this->texcoords[2*mg->triangles[j]->texcoord[k]],
                     this->texcoords[2*mg->triangles[j]->texcoord[k]+1]);
      }

      glVertex3f(this->vertices[3*mg->triangles[j]->vertex[k]], 
                 this->vertices[3*mg->triangles[j]->vertex[k]+1], 
                 this->vertices[3*mg->triangles[j]->vertex[k]+2]);
    }
    glEnd();
  }
}

// Render vertex arrays.
void 
Obj::renderArrays(const MaterialGroup * mg) const
{
  if (mg->material.texture) glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  if (mode == VBO) {
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, mg->vboid); 
    glVertexPointer(3, GL_FLOAT, 0, 0); // last parameter is now vbo offset
    glNormalPointer(GL_FLOAT, 0, (void*)(mg->ntv*3*sizeof(float)));
    if (mg->material.texture) glTexCoordPointer(2, GL_FLOAT, 0, (void*)(2*mg->ntv*3*sizeof(float)));
    glDrawArrays(GL_TRIANGLES, 0, mg->ntv);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0); // switch vbo off again
  } else {
    assert(mode == ARRAY && "unknown render mode");
    glVertexPointer(3, GL_FLOAT, 0, mg->tv);
    glNormalPointer(GL_FLOAT, 0, mg->tn);
    if (mg->material.texture) glTexCoordPointer(2, GL_FLOAT, 0, mg->tt);
    glDrawArrays(GL_TRIANGLES, 0, mg->ntv);
  } 

  if (mg->material.texture) glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

// Guess.
void Obj::printDebugInfo() const
{
  cout << "### VERTICES " << endl;
  for (int i=0; i<this->vertices.size(); i++) {
    cout << "Vertex [" << i << "]: " << this->vertices[i] << endl;
  }
  cout << endl;

  cout << "### NORMALS " << endl;
  for (int i=0; i<this->normals.size(); i++) {
    cout << "Normal [" << i << "]: " << this->normals[i] << endl;
  }
  cout << endl;

  cout << "### MATERIAL GROUPS " << endl;
  map<string, MaterialGroup *>::const_iterator iter;
  for (iter = this->materialgroups.begin(); 
       iter != this->materialgroups.end(); iter++) {
    MaterialGroup * mg = iter->second;
    cout << "Material[\"" << (*iter).first << "\"]" << endl;    

    cout << "size of triangles: " << mg->triangles.size() <<endl;

    for (int i=0; i<mg->triangles.size(); i++) {
      cout << "triangle [" << i << "]: " ;
      for (int j=0; j<3; j++) {
        cout << mg->triangles[i]->vertex[j] << "/" 
             << mg->triangles[i]->texcoord[j] << "/" 
             << mg->triangles[i]->normal[j] << " ";
      }
      cout << "Face normal: " << mg->triangles[i]->facenormal[0] << ", "
           << mg->triangles[i]->facenormal[1] << ", " << mg->triangles[i]->facenormal[2] << endl; 
    }
  }
  cout << endl;
}

