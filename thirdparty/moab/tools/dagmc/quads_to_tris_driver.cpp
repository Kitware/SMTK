#include <iostream>
#include <assert.h>
#include "moab/Core.hpp"
#include "quads_to_tris.hpp"

using namespace moab;

#define MBI mb_instance()
static Interface* mb_instance();

// Read a DAGMC-style file of quads and convert it to tris
// Input argument is the input filename.
// Output file will be called input_filename_tris.h5m.
int main(int argc, char **argv) {

  if( 2 > argc ) {
    std::cout << "Need name of input file with quads." << std::endl;
    return 0;
  }

  // load file from input argument
  ErrorCode result;
  std::string filename = argv[1];
  result = MBI->load_file( filename.c_str() );
  if ( MB_SUCCESS != result ) {
    std::cout << "Error reading file." << std::endl;
    return 1;
  }

  result = quads_to_tris( MBI, 0 );
  if ( MB_SUCCESS != result ) {
    std::cout << "Error converting to tris." << std::endl;
    return 1;
  }


  // Write the file that has been converted from quads to tris.
  // Cut off the .h5m
  int len1 = filename.length();
  filename.erase(len1 - 4);
  std::string filename_new = filename + "_tris.h5m";
  result = MBI->write_mesh( filename_new.c_str());
    assert(MB_SUCCESS == result);

  return 0;
}
Interface* mb_instance() {
  static Core inst;
  return &inst;
}
