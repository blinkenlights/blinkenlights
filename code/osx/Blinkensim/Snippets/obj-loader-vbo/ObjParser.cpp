
#include "Obj.h"
#include "Texture.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <libgen.h>  // for dirname
// FIXME: Figure out how to do this on Windows

using namespace std;

static void initFace(Face * face, int vertexcount) 
{
  face->vertexcount = vertexcount;
  face->vertex.resize(vertexcount);
  face->normal.resize(vertexcount);
  face->texcoord.resize(vertexcount);
}

void 
ObjParser::readFile(const string & filename, class Obj * obj)
{
  string line, token, str;
  float f;
  bool mtl_available = false;

  ifstream objfile(filename.data());
  if (!objfile.is_open()) {
    cerr << "Unable to open file " << filename << endl;
    assert(false && "I can has file?");
  }

  while (objfile >> token) {
    getline(objfile, line);
    istringstream ins(line);
    if (token == "mtllib") {
      // FIXME: This will break if the material file name contains spaces.
      ins >> str;
      string materialpath = string(dirname((char*)filename.data())) + "/" + str;
      mtl_available = this->readMTLFile(materialpath, obj);
    } else if (token == "v") {
      while (ins >> f) {
        obj->vertices.push_back(f);
      }
    } else if (token == "vn") {
      while (ins >> f) {
        obj->normals.push_back(f);
      }
    } else if (token == "vt") {
      while (ins >> f) {
        obj->texcoords.push_back(f);
      }
    } else if (token == "usemtl") {
      if (mtl_available) {
        ins >> str;
        obj->m = obj->materialgroups[str];
      } else {
      // FIXME: material should exist, so is it the right solution to
      // be graceful and use the default material, or should we fail ?!
        cerr << "Tried to use material, but MTL file not available" << endl;
        obj->m = obj->materialgroups["_default"];
      }
      if (!obj->m) {
      }
    } else if (token == "f") {
      this->parseFace(line, obj);
    } 
  }
  objfile.close();
}

bool
ObjParser::readMTLFile(const string & filename, Obj * obj)
{
  ifstream mtlfile(filename.data());
  if (!mtlfile.is_open()) {
    cerr << "Unable to open material file " << filename << endl;  
    return false;
  } else {
    cout << "Loading material file " << filename << endl;
    MaterialGroup * mg;
    string token;
    string materialname;
    while (mtlfile >> token) {
      if (token == "newmtl") {
        mg = new MaterialGroup;
        mg->tv = mg->tn = mg->tt = NULL;
        mtlfile >> materialname;
        obj->materialgroups[materialname] = mg;
        mg->material.texture = NULL;
      } else if (token == "Ka") {
        mtlfile >> mg->material.ambient[0]
                >> mg->material.ambient[1] 
                >> mg->material.ambient[2];
      } else if (token == "Kd") {
        mtlfile >> mg->material.diffuse[0] 
                >> mg->material.diffuse[1] 
                >> mg->material.diffuse[2];
      } else if (token == "Ks") {
        mtlfile >> mg->material.specular[0] 
                >> mg->material.specular[1] 
                >> mg->material.specular[2];
      } else if (token == "Ns") {
        mtlfile >> mg->material.shininess[0];
      } else if (token == "map_Kd") {
        mtlfile >> token;
        mg->material.texture = new Texture(string(dirname((char*)filename.data()))+"/"+token);
        mg->material.texture->load();
      }
    }
    mtlfile.close();
  }
  return true;
}

void
ObjParser::parseFace(const string & line, Obj * obj)
{
  int vertexcount = 0;
  vector<string> str;
  istringstream ins;
  ins.str(line);
  string tmp;
  ins >> tmp;
  while (tmp != "") {
    vertexcount++; 
    str.push_back(tmp);
    tmp.erase();
    ins >> tmp;
  }

  Face * face = new Face;
  initFace(face, vertexcount);

  for (int i=0; i<face->vertexcount; i++) {

    int first_slash = str[i].find('/');
    int second_slash = str[i].find_last_of('/');

    // no slash in token means only geometry (no normals, no texcoords)
    if (first_slash == -1) { 
      face->vertex[i] = atoi(str[i].data());
      face->texcoord[i] = 0;
      face->normal[i] = 0;
    }
    // only one slash or two slashes next to each other means no texcoords or normals
    else if (second_slash == first_slash || second_slash == first_slash+1) { 
      if (obj->normals.size() > 0) {
        string vertex, normal;
        vertex = str[i].substr(0, first_slash);
        normal = str[i].substr(second_slash+1, str[i].length()-second_slash);
        face->vertex[i] = atoi(vertex.data());
        face->texcoord[i] = 0;
        face->normal[i] = atoi(normal.data());
      } else {
        assert(obj->texcoords.size() > 0);
        string vertex, texcoord;
        vertex = str[i].substr(0, first_slash);
        texcoord = str[i].substr(second_slash+1, str[i].length()-second_slash);
        face->vertex[i] = atoi(vertex.data());
        face->texcoord[i] = atoi(texcoord.data());
        face->normal[i] = 0;
      }
    } 
    // geometry, texcoords, and normals are present
    else {
      string vertex, texcoord, normal;
      vertex = str[i].substr(0, first_slash);
      texcoord = str[i].substr(first_slash+1, second_slash-first_slash-1);
      normal = str[i].substr(second_slash+1, str[i].length()-second_slash);
      face->vertex[i] = atoi(vertex.data());
      face->texcoord[i] = atoi(texcoord.data());          
      face->normal[i] = atoi(normal.data());          
    }

    // convert negative indices
    if (face->vertex[i] < 0) {
      face->vertex[i] = obj->vertices.size()/3.0 + face->vertex[i] + 1; 
    }
    if (face->texcoord[i] < 0) {
      face->texcoord[i] = obj->texcoords.size()/3.0 + face->texcoord[i] + 1; 
    }
    if (face->normal[i] < 0) {
      face->normal[i] = obj->normals.size()/3.0 + face->normal[i] + 1; 
    }

    // indices in the file start at index 1, convert to c-style start-at-0
    face->vertex[i] -= 1;
    face->normal[i] -= 1;
    face->texcoord[i] -= 1;
  }

  if (face->vertexcount == 3) {
    obj->m->triangles.push_back(face);
  } else {
    this->triangulateFace(face, obj->m->triangles);
  }
}

// Triangulate face and append faces to vector v
void ObjParser::triangulateFace(Face * face, vector<Face *> & faces) 
{
  for (int i=0; i<face->vertexcount-2; i++) {
    Face * t = new Face();
    initFace(t, 3);

    // all faces share vertex 0
    t->vertex[0] = face->vertex[0];  
    t->normal[0] = face->normal[0];
    t->texcoord[0] = face->texcoord[0];
    
    // vertices 1 and 2
    for (int j=1; j<=2; j++) { 
      t->vertex[j] = face->vertex[i+j];
      t->normal[j] = face->normal[i+j];
      t->texcoord[j] = face->texcoord[i+j];
    }
    faces.push_back(t);
  }
  delete face;
}
