/*
 * Copyright (c) 2005 Lawrence Livermore National Laboratory under
 * contract number B545069 with the University of Wisconsin - Madison.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <vector>
#include <cstdlib>

#include "parse.hpp"

#include "moab/Core.hpp"
#include "moab/Range.hpp"
#include "moab/Interface.hpp"
#define IS_BUILDING_MB
#include "Internals.hpp"
#undef IS_BUILDING_MB

using namespace moab;

#define CALL(A,B) \
  do { ErrorCode _r = iface->A B ; \
       if (MB_SUCCESS != _r) { \
         std::cerr << #A << #B << " failed at " << __FILE__ << ":" << __LINE__ << std::endl; \
         exit( 5 ); }\
       } \
  while (false)


Interface* iface = 0;
const char* exe_name = 0;

static void usage( bool error = true )
{
  std::ostream& s = error ? std::cerr : std::cout;
  
  s << "Usage: " << exe_name << " <options> <input_file> <output_file>" << std::endl
    << "       " << exe_name << " -h" << std::endl
    << "Options: " << std::endl
    << "  -t <ident_tag>[=<value>]  " << std::endl
    << "  -d <data_tag>[=<default>] " << std::endl
    << "  -c <data_tag=type:size>[=defult] " << std::endl
    << "  -w <write_tag>            " << std::endl
    << "  -n|-e|-E                  " << std::endl
    << std::endl;
  if (error) {
    s << "Try '-h' for verbose help." << std::endl;
    exit(1);
  }
  
  s << "This utility will write tag data to a subset of the mesh entities " << std::endl
    << "contained in a file.  The behavior is controlled by three main " << std::endl
    << "properties:" << std::endl
    << " 1) The ident_tag is used to identify sets of entities for which " << std::endl
    << "    data will be stored on each contained element or node. The -n " << std::endl
    << "    or -e flags can be used to restrict operation to only nodes or " << std::endl
    << "    elements, respectively." << std::endl
    << " 2) The data_tag is used to identify which value to write on to " << std::endl
    << "    each entity.  This is a tag on the set containing the entities." << std::endl
    << " 3) The write_tag is the name of the tag that the data is stored in " << std::endl
    << "    on each mesh entity." << std::endl
    << std::endl
    << " -t : Specify an ident_tag.  If a value is specified, only those " << std::endl
    << "      sets with the specified value are processed.  At least one " << std::endl
    << "      ident_tag must be specified.  Multiple ident_tags may be " << std::endl
    << "      specified, in which case any set that matches any of the " << std::endl
    << "      specified ident_tags will be processed (logical OR)." << std::endl
    << std::endl
    << " -d : Specify the data_tag.  If multiple ident_tags are specified " << std::endl
    << "      then the data_tag must be specified.  If only one ident_tag " << std::endl
    << "      is specified then the data_tag specification is optional." << std::endl
    << "      If no data_tag is specified, the value of the ident_tag " << std::endl
    << "      will be used.  If a value is specified for the data_tag, " << std::endl
    << "      then the specified value will be used for any set that " << std::endl
    << "      doesn't have a value for the data_tag." << std::endl
    << std::endl
    << " -c : Similar to -d, except that the tag is created if it doesn't" << std::endl
    << "      already exist.  If the tag is created, then all entities" << std::endl
    << "      receive the specified default value for the tag.  In this " << std::endl
    << "      case it is an error if no default value is specified." << std::endl
    << std::endl
    << " -w : Specify the tag to create and store values in on mesh " << std::endl
    << "      entities.  If no write_tag is specified, the data_tag " << std::endl
    << "      will be used." << std::endl
    << std::endl
    << " -n : Write tag data only on nodes (vertices)." << std::endl
    << " -e : Write tag data only on elements." << std::endl
    << " -E : Tag value on each node is that of one of its adjacent elements." << std::endl
    << std::endl
    << "The syntax for specifying tag values is as follows: " 
    << std::endl << std::endl;
  tag_syntax(s);
  s << std::endl;
  exit(0);
}

static void about( bool error = true )
{
  std::ostream& s = error ? std::cerr : std::cout;
  s << "A utility to propogate tag values from the entity sets "
       "containing mesh entities to the entities contained in "
       "those sets." << std::endl << std::endl;
  usage(error);
}
  
static void parse_error( const char* msg, const char* val = 0 )
{
  std::cerr << msg;
  if (val)
    std::cerr << ": " << val;
  std::cerr << std::endl;
  std::cerr << "Try '" << exe_name << " -h' for help" << std::endl;
  exit(1);
}

int main( int argc, char* argv[] )
{
  Core mb_core;
  exe_name = argv[0];
  iface = &mb_core;

  if (argc == 1)
    about();
    
    // find file names 
    // load input file before processing other options so
    // tags are defined
  const char* input_name = 0;
  const char* output_name = 0;
  for (int i = 1; i < argc; ++i)
  {
    if (argv[i][0] == '-')
    {
      switch (argv[i][1]) { 
        case 't': case 'c': case 'd': case 'w': 
          ++i; 
        case 'n': case 'e': case 'E':
          break;
        case 'h': 
          usage(false);
          break;
        default:
          parse_error( "Invalid option", argv[i] );
          break;
      }
    }
    else if (!input_name)
      input_name = argv[i];
    else if (!output_name)
      output_name = argv[i];
    else 
      parse_error( "Unexpected argument", argv[i] );
  }
  
  if (!input_name)
    parse_error( "No input file specified." );
  if (!output_name)
    parse_error( "No output file specified." );
  
    // Read the input file
  if (MB_SUCCESS != iface->load_mesh( input_name ))
  {
    std::cerr << "Failed to read file: " << input_name << std::endl;
    std::string message;
    if (MB_SUCCESS == iface->get_last_error(message))
      std::cerr << message << std::endl;
    return 2;
  }

    
    
  bool nodes_spec = false;
  bool elems_spec = false;
  bool node_from_elem_spec = false;
  bool have_data_tag = false;
  const char* write_tag_name = 0;
  Tag write_tag = 0;
  TagSpec data_tag = { 0, 0 };
  typedef std::vector<TagSpec> TagVect;
  TagVect ident_tags;
  int data_size = 0;
  
  for (int i = 1; i < argc; ++i)
  {
    if (argv[i] == input_name || argv[i] == output_name)
      continue;
    else if (!strcmp(argv[i],"-n"))
      nodes_spec = true;
    else if (!strcmp(argv[i], "-e"))
      elems_spec = true;
    else if (!strcmp(argv[i], "-E")) {
      node_from_elem_spec = true;
      elems_spec = true;
      nodes_spec = false;
    }
    else if (!argv[i][0])
      usage();
    else
    {
      char flag = argv[i][1];
      if ((flag != 't' && flag != 'd' && flag != 'w' && flag != 'c') || argv[i][2])
        parse_error( "Invalid argument", argv[i] );
      
      ++i;
      if (i == argc)
        parse_error( "Expected tag spec following option", argv[i-1] );
    
      if (flag == 'w')
      {
        if (write_tag_name)
          parse_error( "Invalid argument", argv[i] );
        write_tag_name = argv[i];
      }
      else if (flag == 'c')
      {
        TagSpec spec;
        if (parse_tag_create( argv[i], spec, iface ))
          parse_error( "Failed to parse tag spec", argv[i] );
        
        if (have_data_tag)
          parse_error( "Invalid argument", argv[i] );
        
        data_tag = spec;
        have_data_tag = true;
      }         
      else
      {
        TagSpec spec;
        if (parse_tag_spec( argv[i], spec, iface))
          parse_error("Failed to parse tag spec", argv[i] );
        
        if (flag == 'd')
        {
          if (have_data_tag)
            parse_error( "Invalid argument", argv[i] );
         
          data_tag = spec;
          have_data_tag = true;
        }
        else
        {
          ident_tags.push_back( spec );
        }
      }
    }
  } // for(args)
  
    // if neither, default to both
  if (!nodes_spec && !elems_spec)
    nodes_spec = elems_spec = true;
  
    // must have at least one identifying tag
  if (ident_tags.empty())
    parse_error ("At least one identifying tag must be specified.");
  
    // If data tag wasn't specified, use identifying tag for data
  if (!have_data_tag)
  {
    if (ident_tags.size() > 1)
      parse_error ("No data tag specified.");
    data_tag.value = 0;
    data_tag.handle = ident_tags[0].handle;
  }
  CALL( tag_get_bytes, (data_tag.handle, data_size) );
  
    // If write dat wasn't specified, use data tag 
  if (!write_tag_name)
  {
    write_tag = data_tag.handle;
  }
    // If write tag was specified, if it exists its type
    // msut match that of the data tag.  If it doesn't exist,
    // create it.
  else
  {
    DataType data_type;
    CALL( tag_get_data_type, (data_tag.handle, data_type) );
    
    CALL( tag_get_handle, (write_tag_name, data_size, data_type, write_tag, MB_TAG_SPARSE|MB_TAG_BYTES|MB_TAG_CREAT) );
  }
  
  
  /**************** Done processing input -- do actual work ****************/
 
    // Get list of sets with identifying tags
  Range sets, temp;
  for (TagVect::iterator i = ident_tags.begin(); i != ident_tags.end(); ++i)
  {
    const void* value[] = { i->value };
    CALL( get_entities_by_type_and_tag,
          (0, MBENTITYSET, &i->handle, i->value ? value : 0, 1, temp) );
    sets.merge( temp );
  }
  
    // For each set, set tag on contained entities
  std::vector<unsigned char> tag_data(data_size);
  for (Range::iterator i = sets.begin(); i != sets.end(); ++i)
  {
      // Get tag value
    ErrorCode rval = iface->tag_get_data( data_tag.handle, &*i, 1, &tag_data[0] );
    if (MB_TAG_NOT_FOUND == rval)
    {
      if (!data_tag.value)
      {
        std::cerr << "Data tag not set for entityset " 
                  << iface->id_from_handle(*i) << std::endl;
        continue;
      }
      memcpy( &tag_data[0], data_tag.value, data_size );
    }
    else if (MB_SUCCESS != rval)
    {
      CALL( tag_get_data, (data_tag.handle, &*i, 1, &tag_data[0]) );
    }
    
      // Get entities
    Range entities;
    CALL( get_entities_by_handle, (*i, entities, true) );
    int junk;
    Range::iterator eb = entities.lower_bound( entities.begin(), 
                                                 entities.end(),
                                                 CREATE_HANDLE(MBEDGE,0,junk) );
    if (elems_spec) 
      for (Range::iterator j = eb; j != entities.end(); ++j)
        CALL( tag_set_data, (write_tag, &*j, 1, &tag_data[0]) );
    if (nodes_spec)
      for (Range::iterator j = entities.begin(); j != eb; ++j)
        CALL( tag_set_data, (write_tag, &*j, 1, &tag_data[0]) );
    if (node_from_elem_spec) {
      Range elems;
      elems.merge( eb, entities.end() );
      entities.clear();
      CALL( get_adjacencies, (elems, 0, false, entities, Interface::UNION) );
      for (Range::iterator j = entities.begin(); j != entities.end(); ++j)
        CALL( tag_set_data, (write_tag, &*j, 1, &tag_data[0]) );
    }
  }
  
    // Write the output file
  if (MB_SUCCESS != iface->write_mesh( output_name ))
  {
    std::cerr << "Failed to write file: " << output_name << std::endl;
    std::string message;
    if (MB_SUCCESS == iface->get_last_error(message))
      std::cerr << message << std::endl;
    return 2;
  }
  
  return 0;
}

  
  
    
    
      
  
  
  
