#include "parse.hpp"

#include <iostream>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

using namespace moab;

void tag_syntax( std::ostream& s )
{
  s << "Tags are specified as <name>=[value], where the tag value " << std::endl
    << "is optional." << std::endl 
    << std::endl
    << "Values of integral types (INTEGER, BIT, and HANDLE) are " << std::endl
    << "specified using standard C integer notation (a 0x prefix " << std::endl
    << "for hexidecimal, 0 prefix for octal, and no prefix for " << std::endl
    << "decimal.)  The value of an opaque tag is interpreted as " << std::endl
    << "either a integral value or a character string. If the tag " << std::endl
    << "value begins with the prefix 0x it will be interpreted as a " << std::endl
    << "hexidecimal (base-16) number.  If the value does not begin " << std::endl
    << "with the 0x prefix, it is interpreted as a character string." << std::endl
    << "Characater strings will be padded with null characters as" << std::endl
    << "necessary to fill the tag." << std::endl
    << "Floating-point (real) values must be specified in base-10." << std::endl
    << "C exponential notation (e.g. 1e-10) is accepted." << std::endl
    << std::endl
    << "If the tag is an array of integral or floating-point values " << std::endl
    << "then the tag value must be specified as a comma-separated " << std::endl
    << "list, with NO spaces." << std::endl
    << std::endl
    << "Tags are created with the syntax name=type:size[=default_value]." << std::endl
    << "where type is one of {int,double,opaque,handle,bit} and size is " << std::endl
    << "the number of values of the specified type, or the number of " << std::endl
    << "bytes if the type is 'opaque',  A default value for the tag may " << std::endl
    << "be specified." 
    << std::endl;
}

  // Check endian-ness of platform.  Used when parsing numerical
  // value for opaque tags.
inline static bool is_platform_little_endian();

  // Parse tag value from a string (vals).  The passed size
  // is the number of values returned by MOAB from tag_get_length.
static void* parse_values( const char* vals, DataType type, int size );

  // Parse opque tag data as either a hexidecimal number or
  // an ASCII string.
static unsigned char* parse_opaque_value( const char* vals, int size );

  // Parse one or more non-opaque tag values in a comma-separated list.
  // The passed size is the size returned by MOAB (num values * sizeof(type)).
template<typename T> T* parse_values_typed( const char* vals, int size );

  // Template function to parse a single non-opaque tag value.
  // Parses the value in the string pointed to by "iter" and updates
  // iter to point passed the parsed value.  Returns zero on success.
template<typename T> int parse_value( const char*& iter, T& value );

  // Convert an ASCII hexidecimal digit to its numerical value.
static int hexdigit( char c );



inline static bool is_platform_little_endian()
{
  static const unsigned int one = 1;
  static const bool little = !*((char*)&one);
  return little;
}



template<typename T> int parse_value( const char*& iter, T& value )
{
  char* endptr;
  long parsed_val = strtol( iter, &endptr, 0 );
  if (endptr == iter)
    return 1;
  iter = endptr; 
  
  value = (T)parsed_val;
  if ((long)value != parsed_val)
  {
    std::cerr << "Value too large: " << iter << std::endl;
    return 2;
  }
  
  return 0;  
}



template<> int parse_value<double>( const char*& iter, double& value )
{
  char* endptr;
  value = strtod( iter, &endptr );
  if (endptr == iter)
    return 1;
  iter = endptr;
  return 0;
}



int hexdigit( char c )
{
  if (c >= '0' && c <= '9')
    return c - '0';
  else if (c >= 'a' && c <= 'f')
    return 10 + c - 'a';
  else if (c >= 'A' && c <= 'F')
    return 10 + c - 'A';
  else
    return -1;
}



unsigned char* parse_opaque_value( const char* vals, int size )
{
  unsigned char* data = (unsigned char*)malloc( size );
  if (vals[0] && vals[0] == '0' && vals[1] && toupper(vals[1]) == 'X')
  {
    unsigned char *iter, *end;
    int step;
    if (is_platform_little_endian())
    {
      iter = data;
      end = data + size;
      step = 1;
    }
    else
    {
      iter = data + size - 1;
      end = data - 1;
      step = -1;
    }
    
    const char* vals_end = vals + 1;
    const char* vals_iter = vals + strlen(vals) - 1;
    for ( ; iter != end; iter += step )
    {
      int less = 0;
      int most = 0;
      if (vals_iter != vals_end)
      {
        less = hexdigit( *vals_iter); 
        --vals_iter;
      }
      if (vals_iter != vals_end)
      {
        most = hexdigit( *vals_iter);
        --vals_iter;
      }
      if (less < 0 || most < 0)
      {
        std::cerr << "Error parsing hex value: " << vals << std::endl;
        free (data);
        return 0;
      }
      
      *iter = 16 * most + less;
    }
  }
  else
  {
    memset( data, 0, size );
    strcpy( (char*)data, vals + 2 );
  }
  
  return data;
}



template<typename T> T* parse_values_typed( const char* vals, int count )
{
  if (!count)
    return 0;
    
  T* data = (T*)malloc( count * sizeof(T) );
  T* end = data + count;
  if (parse_value<T>( vals, *data ))
  {
    free(data);
    return 0;
  }
  for ( T* ptr = data + 1; ptr != end; ++ptr )
  {
    if (*vals != ',')
    {
      std::cerr << "Expected ',' separating tag values: " << vals << std::endl;
      free( data );
      return 0;
    }
    ++vals;
    if (parse_value<T>( vals, *ptr ))
    {
      free(data);
      return 0;
    }
  }
  
  return data;
}



void* parse_values( const char* vals, DataType type, int size )
{
  switch( type ) {
    case MB_TYPE_OPAQUE:  return parse_opaque_value               ( vals, size );
    case MB_TYPE_INTEGER: return parse_values_typed<          int>( vals, size );
    case MB_TYPE_DOUBLE:  return parse_values_typed<       double>( vals, size );
    case MB_TYPE_BIT:     return parse_values_typed<      bittype>( vals, size );
    case MB_TYPE_HANDLE:  return parse_values_typed<EntityHandle>(vals, size );
    default:
      std::cerr << "Unknown tag data type: " << (int)type << std::endl;
      return 0;
  }
}



int parse_tag_spec( char* name, TagSpec& result, Interface* iface )
{
    //  Separate optional tag value from tag name
  char* val = strrchr( name, '=' );
  if (val)
  {
      // zero-length tag name>
    if (val == name)
    {
      std::cerr << "Cannot create tag w/out name: " << name << std::endl;
      return 1;
    }
    *val = '\0';
    if (!*++val) // if name ends with an '=', set val to NULL.
      val = 0;
  } 
  
    // Get tag
  ErrorCode rval = iface->tag_get_handle( name, 0, MB_TYPE_OPAQUE, result.handle, MB_TAG_ANY );
  if (MB_TAG_NOT_FOUND == rval)
  {
    std::cerr << "Tag not found: " << name << std::endl;
    return 2;
  }
  else if (MB_SUCCESS != rval)
  {
    std::cerr << "Error retrieving tag handle: " << name << std::endl;
    return 3;
  }
  
    // Parse tag value
  result.value = 0;
  if (val)
  {
    DataType type;
    rval = iface->tag_get_data_type( result.handle, type );
    if (MB_SUCCESS != rval)
    {
      std::cerr << "Error retrieving type for tag: " << name << std::endl;
      return 3;
    }
    
    int size;
    rval = iface->tag_get_length( result.handle, size );
    if (MB_SUCCESS != rval)
    {
      std::cerr << "Error retrieving size for tag: " << name << std::endl;
      return 3;
    }
    
    result.value = parse_values( val, type, size );
    if (!result.value)
      return 1;
  }
  
  return 0;
}

  
  

int parse_tag_create( char* name, TagSpec& result, Interface* iface )
{
    // split at '=' signs
  
  char* eq1 = strrchr( name, '=' );
  if (!eq1)
  {
    std::cerr << "Invalid tag specification: " << name << std::endl;
    return 1;
  }
  *eq1 = '\0';
  ++eq1;
  char *type_str = eq1, *val = 0;

  char* eq2 = strrchr( name, '=' );
  if (eq2)
  {
    *eq2 = '\0';
    ++eq2;
    val = ('\0' == eq1[0]) ? 0 : eq1;
    type_str = eq2;
  }
  
    // parse type data
  char* size_str = strchr( type_str, ':' );
  if (!size_str)
  {
    std::cerr << "Invalid tag type specification: " << type_str << std::endl;
    return 1;
  }
  *size_str = '\0';
  ++size_str;
  DataType type;
  if (!strcmp(type_str,"int"))
  {
    type = MB_TYPE_INTEGER;
  }
  else if (!strcmp(type_str,"double"))
  {
    type = MB_TYPE_DOUBLE;
  }
  else if (!strcmp(type_str,"bit"))
  {
    type = MB_TYPE_BIT;
  }
  else if (!strcmp(type_str,"handle"))
  {
    type = MB_TYPE_HANDLE;
  }
  else if (!strcmp(type_str,"opaque"))
  {
    type = MB_TYPE_OPAQUE;
  }
  else
  {
    std::cerr << "Invalid tag type specification: " << type_str << std::endl;
    return 1;
  }
  char* end_ptr;
  int count = (int)strtol(size_str, &end_ptr, 0);
  if (!*size_str || *end_ptr || count < 1)
  {
    std::cerr << "Invalid tag size specification: " << size_str << std::endl;
    return 1;
  }
  
    // parse default value
  result.value = 0;
  if (val)
  {
    result.value = parse_values( val, type, count );
    if (!result.value)
      return 1;
  }
  
    // check if tag exists
  if (MB_SUCCESS == iface->tag_get_handle( name, 0, MB_TYPE_OPAQUE, result.handle, MB_TAG_ANY ))
  {
      // make sure it matches
    DataType etype;
    int esize;
    if (MB_SUCCESS != iface->tag_get_data_type( result.handle, etype ) ||
        MB_SUCCESS != iface->tag_get_length( result.handle, esize ))
    {
      std::cerr << "Error accessing properties of tag: " << name << std::endl;
      return 3;
    }
    
    if (etype != type || esize != count)
    {
      std::cerr << "Tag already exists with different type: " << name << std::endl;
      return 1;
    }
    
    std::vector<unsigned char> value(esize);
    if (result.value)
    {
      ErrorCode rval = iface->tag_get_default_value( result.handle, &value[0] );
      if (rval != MB_ENTITY_NOT_FOUND && rval != MB_SUCCESS)
      {
        std::cerr << "Error checking default value of tag: " << name << std::endl;
        return 3;
      }
      else if (rval == MB_ENTITY_NOT_FOUND || memcmp(&value[0], result.value, esize) )
      {
        std::cerr << "Tag already exists and default value doesn't match: " << name << std::endl;
        return 1;
      }
    }
  }
  else
  {
    ErrorCode rval = iface->tag_get_handle( name, 
                                   count, type,
                                   result.handle,
                                   MB_TAG_SPARSE|MB_TAG_CREAT,
                                   result.value );
    if (MB_SUCCESS != rval)
    {
      std::cerr << "Failed to create tag: " << name << std::endl;
      return 3;
    }
  }
  
  return 0;
}

     
  
    
  
