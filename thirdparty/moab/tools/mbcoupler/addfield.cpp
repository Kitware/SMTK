#include <iostream>
#include "moab/Core.hpp"
#include <math.h>
#include <cstdlib>
#include <assert.h>

using namespace std;

namespace moab {

//make a nice picture. tweak here
//217: x,y -> [-8, 8] /// z -> [-65, 65]
double physField(double x, double y, double z, double factor){

    // 1/r^2 decay from {0,0,0}
    // tjt - changing to 1/r
    /*
  double scale = 1.0/factor;
  out = fabs(x*scale) + fabs(y*scale) + fabs(z*scale);
  out += 1e-1; // clamp
  out = 1/out;
    */
  double out = factor * sqrt(x*x + y*y + z*z);

  return out;
}


void putElementField(Interface *mbi, const char *tagname, double factor){
  Range elems;

  mbi->get_entities_by_dimension(0, 3, elems);

  const double defVal = 0.;
  Tag fieldTag;
  ErrorCode rval = mbi->tag_get_handle(tagname, 1, MB_TYPE_DOUBLE, fieldTag, MB_TAG_DENSE|MB_TAG_CREAT, &defVal);
  assert(MB_SUCCESS == rval);
#ifdef NDEBUG
  if (MB_SUCCESS == rval) {}; // Line to avoid compiler warning about unused variable
#endif

  int numElems = elems.size();

  for(int i=0; i<numElems; i++){
      //cout << elems[i] << endl;
    EntityHandle elem = elems[i];

    double xyz[3];
    mbi->get_coords(&elem, 1, xyz);

    double fieldValue =  physField(xyz[0], xyz[1], xyz[2], factor);

    mbi->tag_set_data(fieldTag, &elem, 1, &fieldValue);
  }

}


void putVertexField(Interface *mbi, const char *tagname, double factor){
  Range verts;

  mbi->get_entities_by_type(0, MBVERTEX, verts);

  const double defVal = 0.;
  Tag fieldTag;
  mbi->tag_get_handle(tagname, 1, MB_TYPE_DOUBLE, fieldTag, MB_TAG_DENSE|MB_TAG_CREAT, &defVal);
 
  int numVerts = verts.size();
  for(int i=0; i<numVerts; i++){
    EntityHandle vert = verts[i]; //?

    double vertPos[3];
    mbi->get_coords(&vert, 1, vertPos);

    double fieldValue =  physField(vertPos[0], vertPos[1], vertPos[2], factor);

    mbi->tag_set_data(fieldTag, &vert, 1, &fieldValue);
  }



}


} // namespace moab

//Using moab instead of imesh for dev speed
int main(int argc, char **argv){

  using namespace moab;
  
  Interface *mbi = new Core();

  if (argc < 3){
    cout << "Usage: " << argv[0] << " <infile> <outfile> [factor]\n"
         << "Writes both vertex and element fields.\n";
    return 0;
  }

  mbi->load_mesh(argv[1]);

  double factor = 1.0;
  if (argc == 4) factor = atof(argv[3]);
  
  putVertexField(mbi, "vertex_field", factor);
  putElementField(mbi, "element_field", factor);

  ErrorCode result = mbi->write_mesh(argv[2]);
  if (MB_SUCCESS == result) cout << "wrote " << argv[2] << endl;
  else cout << "Failed to write " << argv[2] << endl;

    //  vector<double> coords;                                                                                                                               
    //  mbi->get_vertex_coordinates(coords);                                                                   
    //  double xavg = 0;                                                                                                                                           
    //  for (int i = 0; i < coords.size()/3; i++) xavg += coords[i];
    //  cout << xavg << endl;

  return 1;
}

