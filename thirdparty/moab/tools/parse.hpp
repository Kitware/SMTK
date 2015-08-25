#ifndef PARSE_HPP
#define PARSE_HPP

#include <iosfwd>
#include "moab/Interface.hpp"

// A structure containing the parsed CL tag specification.
struct TagSpec
{
  moab::Tag handle;  // Tag handle
  void* value;   // Tag value (malloc'd) or NULL if no value specified
};

// Parse a tag specified in the form: tagname=value,
// where the "=value" portion is optional.  Returns 0 on
// success.
int parse_tag_spec( char* string, TagSpec& result, moab::Interface* iface );

// Parse a string specifying a new tag to create, and create it.
int parse_tag_create( char* string, TagSpec& result, moab::Interface* iface );

// Print description of syntax accepted in the string passed to
// parse_tag_spec.
void tag_syntax( std::ostream& stream );

// Data type used to store bit tags
typedef unsigned char bittype;

#endif
