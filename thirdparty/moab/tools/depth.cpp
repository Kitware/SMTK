#include "moab/Range.hpp"
#include "moab/Core.hpp"
#include "moab/Skinner.hpp"
#include <iostream>
#include <stdlib.h>

using namespace moab;

enum {
  NO_ERROR= 0,
  SYNTAX_ERROR = 1,
  FILE_IO_ERROR = 2,
  INTERNAL_ERROR = 3
};

const char* DEFAULT_TAG_NAME = "depth";

static void usage( const char* argv0 )
{
  std::cerr << "Usage: " << argv0 << "[-t <tag name] <input_file> <output_file>" << std::endl
                         << argv0 << "-h" << std::endl;
  exit(SYNTAX_ERROR);
}

static void check( ErrorCode rval )
{
  if (MB_SUCCESS != rval) {
    std::cerr << "Internal error.  Aborting." << std::endl;
    exit(INTERNAL_ERROR);
  }
}

static void tag_depth( Interface& moab, Tag tag );

int main( int argc, char* argv[] )
{
  const char *input = 0, *output = 0, *tagname = DEFAULT_TAG_NAME;
  bool expect_tag_name = false;
  for (int i = 1; i < argc; ++i) {
    if (expect_tag_name) {
      tagname = argv[i];
      expect_tag_name = false;
    }
    else if (!strcmp("-t", argv[i]))
      expect_tag_name = true;
    else if (input == 0)
      input = argv[i];
    else if (output == 0)
      output = argv[i];
    else {
      std::cerr << "Unexpected argument: '" << argv[i] << "'" << std::endl;
      usage(argv[0]);
    }
  }
  
  if (expect_tag_name) {
    std::cerr << "Expected argument following '-t'" << std::endl;
    usage(argv[0]);
  }
  if (!input) {
    std::cerr << "No input file" << std::endl;
    usage(argv[0]);
  }
  if (!output) {
    std::cerr << "No output file" << std::endl;
    usage(argv[0]);
  }

  Core moab;
  Interface& mb = moab;
  
  EntityHandle file;
  ErrorCode rval;
  rval = mb.create_meshset( MESHSET_SET, file ); check(rval);
  rval = mb.load_file( input, &file );
  if (MB_SUCCESS != rval) {
    std::cerr << "Failed to load file: " << input << std::endl;
    return FILE_IO_ERROR;
  }
  
  int init_val = -1;
  Tag tag;
  bool created;
  rval = mb.tag_get_handle( tagname, 1, MB_TYPE_INTEGER, tag, MB_TAG_DENSE|MB_TAG_CREAT, &init_val, &created );
  if (!created) {
    rval = mb.tag_delete( tag ); check(rval);
    rval = mb.tag_get_handle( tagname, 1, MB_TYPE_INTEGER, tag, MB_TAG_DENSE|MB_TAG_CREAT, &init_val, &created );
    check(rval);
  }
  
  tag_depth( mb, tag );
  
  rval = mb.write_file( output, 0, 0, &file, 1 );
  if (rval == MB_SUCCESS)
    std::cout << "Wrote file: " << output << std::endl;
  else {
    std::cerr << "Failed to write file: " << output << std::endl;
    return FILE_IO_ERROR;
  }
  
  return NO_ERROR;
}

static ErrorCode get_adjacent_elems( Interface& mb, const Range& verts, Range& elems )
{
  elems.clear();
  ErrorCode rval;
  for (int dim = 3; dim > 0; --dim) {
    rval = mb.get_adjacencies( verts, dim, false, elems, Interface::UNION );
    if (MB_SUCCESS != rval)
      break;
  }
  return rval;
}

void tag_depth( Interface& mb, Tag tag )
{
  ErrorCode rval;
  int dim;
  
  Skinner tool(&mb);
  Range verts, elems;
  dim = 3;
  while (elems.empty()) {
    rval = mb.get_entities_by_dimension( 0, dim, elems ); check(rval);
    if (--dim == 0)
      return; // no elements
  }
  rval = tool.find_skin( 0, elems, 0, verts ); check(rval);
  rval = get_adjacent_elems( mb, verts, elems ); check(rval);
  
  std::vector<int> data;
  int val, depth = 0;
  while (!elems.empty()) {
    data.clear();
    data.resize( elems.size(), depth++ );
    rval = mb.tag_set_data( tag, elems, &data[0] ); check(rval);
    
    verts.clear();
    rval = mb.get_adjacencies( elems, 0, false, verts, Interface::UNION );
    check(rval);
    
    Range tmp;
    rval = get_adjacent_elems( mb, verts, tmp ); check(rval);
    elems.clear();
    for (Range::reverse_iterator i = tmp.rbegin(); i != tmp.rend(); ++i) {
      rval = mb.tag_get_data( tag, &*i, 1, &val ); check(rval);
      if (val == -1)
        elems.insert( *i );
    }
  }
  
  std::cout << "Maximum depth: " << depth << std::endl;
}
