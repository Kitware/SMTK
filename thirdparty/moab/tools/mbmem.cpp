#include "moab/Core.hpp"
#include "moab/Range.hpp"
#include "moab/CN.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstdio>
#include <cstdlib>

#ifndef _MSC_VER
#include <sys/times.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

static void usage( const char* argv0, bool help = false )
{
  std::ostream& str = help ? std::cout : std::cerr;
  str << "Usage: " << argv0 << " [-H|-b|-k|-m] <filename> [<filename> ...]" << std::endl
      << "       " << argv0 << " [-H|-b|-k|-m] -T" << std::endl;
  if (!help) {
    str << "       " << argv0 << " -h" << std::endl;
    std::exit(1);
  }
  
  std::cerr << "  -H : human readable units" << std::endl
            << "  -b : bytes" << std::endl
            << "  -k : kilobytes (1 kB == 1024 bytes)" << std::endl
            << "  -m : megabytes (1 MB == 1024 kB)" << std::endl
            << "  -g : gigabytes (1 GB == 1024 MB)" << std::endl
            << "  -T : test mode" << std::endl
            << std::endl;
  std::exit(0);
}

enum Units { HUMAN, BYTES, KILOBYTES, MEGABYTES, GIGABYTES };
Units UNITS = HUMAN;

  // The core functionality of this example
static void print_memory_stats( moab::Interface& mb,
                                bool per_type = true,
                                bool per_tag = true,
                                bool totals = true,
                                bool sysstats = true );
  
  // Generate a series of meshes for testing
static void do_test_mode();
  
  // main routine: read any specified files and call print_memory_stats
int main( int argc, char* argv[] )
{
  moab::ErrorCode rval;
  bool no_more_flags = false;
  bool test_mode = false;
  std::vector<int> input_file_list;

    // load each file specified on command line
  for (int i = 1; i < argc; ++i) {
    if (!no_more_flags && argv[i][0] == '-') {
      if (!strcmp(argv[i],"-H"))
        UNITS = HUMAN;
      else if(!strcmp(argv[i],"-b"))
        UNITS = BYTES;
      else if(!strcmp(argv[i],"-k"))
        UNITS = KILOBYTES;
      else if(!strcmp(argv[i],"-m"))
        UNITS = MEGABYTES;
      else if(!strcmp(argv[i],"-g"))
        UNITS = GIGABYTES;
      else if(!strcmp(argv[i],"-T"))
        test_mode = true;
      else if(!strcmp(argv[i],"-h"))
        usage(argv[0],true);
      else if(!strcmp(argv[i],"--"))
        no_more_flags = true;
      else {
        std::cerr << argv[0] << ": Invalid flag: \"" << argv[i] << "\"." << std::endl << std::endl;
        usage(argv[0]);
      }
    }
    else {
      input_file_list.push_back(i);
    }
  }
  
  if (test_mode) {
    do_test_mode();
    if (input_file_list.empty())
      return 0;
  }

  moab::Core mbcore;
  moab::Interface& mb = mbcore;
  for (std::vector<int>::iterator it = input_file_list.begin(); 
       it != input_file_list.end(); ++it) {
    rval = mb.load_file(argv[*it]);

      // if file load failed, print some info and exit
    if (moab::MB_SUCCESS != rval) {
      std::string message;
      mb.get_last_error( message );
      std::cerr << mb.get_error_string(rval) << ": " << message << std::endl
                << argv[*it] << ": Failed to read file." << std::endl;
      return 1;
    }
    
    std::cout << "Loaded file: " << argv[*it] << std::endl;
  }
  
    // print summary of MOAB's memory use
  print_memory_stats(mb);
  return 0;
}

  // struct to store memory stats
struct MemStats {
  unsigned long long total_storage;
  unsigned long long total_amortized;
  unsigned long long entity_storage;
  unsigned long long entity_amortized;
  unsigned long long adjacency_storage;
  unsigned long long adjacency_amortized;
  unsigned long long tag_storage;
  unsigned long long tag_amortized;
};

 // test if MemStats object indicates no memory
static bool is_zero( const MemStats& stats );

  // populdate a MemStats structg by calling 
  // moab::Interface::estimated_memory_use
static void get_mem_stats( moab::Interface& mb,
                           MemStats& data,
                           moab::EntityType type = moab::MBMAXTYPE );

  // Formatted string representation of memory size value
static std::string memstr( unsigned long long val );

  // Get string describing tag data type
static std::string tag_type_string( moab::Interface& mb, moab::Tag tag );

  // Get string representation of tag storage type
static std::string tag_storage_string( moab::Interface& mb, moab::Tag tag );

  // Center
static std::string center( const char* str, size_t width );

void print_memory_stats( moab::Interface& mb,
                         bool per_type,
                         bool per_tag,
                         bool totals,
                         bool sysstats )
{
  moab::ErrorCode rval;
  const char ANON_TAG_NAME[] = "(anonymous)";
  const int TYPE_WIDTH = 10;
  const int MEM_WIDTH = 7;
  const int MEM2_WIDTH = 2*MEM_WIDTH+1;
  const int MIN_TAG_NAME_WIDTH = strlen(ANON_TAG_NAME);
  const int DTYPE_WIDTH = 12;
  const int STORAGE_WIDTH = 8;

    // per-entity-type table header
  MemStats stats;
  
  if (per_type) {
  
    std::cout.fill(' ');
    std::cout << std::left << std::setw(TYPE_WIDTH) << "Type" << ' '
              << center("Total",MEM2_WIDTH) << ' '
              << center("Entity",MEM2_WIDTH) << ' '
              << center("Adjacency",MEM2_WIDTH) << ' '
              << center("Tag",MEM2_WIDTH) << ' '
              << std::endl << std::setw(TYPE_WIDTH) << " ";
    for (int i = 0; i < 4; ++i)
      std::cout << ' ' << std::left << std::setw(MEM_WIDTH) << "Used"
                << ' ' << std::left << std::setw(MEM_WIDTH) << "Alloc";
    std::cout << std::endl;
    std::cout.fill('-');
    std::cout << std::setw(TYPE_WIDTH) << '-';
    for (int i = 0; i < 8; ++i)
      std::cout << ' ' << std::setw(MEM_WIDTH) << '-';
    std::cout.fill(' ');
    std::cout << std::endl;

      // per-entity-type memory use
    for (moab::EntityType t = moab::MBVERTEX; t != moab::MBMAXTYPE; ++t) {
      get_mem_stats( mb, stats, t );
      if (is_zero(stats)) continue;  // skip types with no allocated memory

      std::cout << std::left << std::setw(TYPE_WIDTH) << moab::CN::EntityTypeName(t) << ' '
                << std::right << std::setw(MEM_WIDTH) << memstr(stats.total_storage) << ' '
                << std::right << std::setw(MEM_WIDTH) << memstr(stats.total_amortized) << ' '
                << std::right << std::setw(MEM_WIDTH) << memstr(stats.entity_storage) << ' '
                << std::right << std::setw(MEM_WIDTH) << memstr(stats.entity_amortized) << ' '
                << std::right << std::setw(MEM_WIDTH) << memstr(stats.adjacency_storage) << ' '
                << std::right << std::setw(MEM_WIDTH) << memstr(stats.adjacency_amortized) << ' '
                << std::right << std::setw(MEM_WIDTH) << memstr(stats.tag_storage) << ' '
                << std::right << std::setw(MEM_WIDTH) << memstr(stats.tag_amortized) << std::endl;
    }
  } // end per_type
  
  if (per_tag) {
      // get list of tags
    std::vector<moab::Tag> tags;
    std::vector<moab::Tag>::const_iterator ti;
    mb.tag_get_tags( tags );

      // figure out required field with to fit longest tag name 
    unsigned maxlen = MIN_TAG_NAME_WIDTH;
    for (ti = tags.begin(); ti != tags.end(); ++ti) {
      std::string name;
      rval = mb.tag_get_name( *ti, name );
      if (moab::MB_SUCCESS != rval)
        continue;
      if (name.size() > maxlen)
        maxlen = name.size();
    }

      // print header for per-tag data
    if (!tags.empty()) {
      std::cout.fill(' ');
      std::cout << std::endl
                << std::left << std::setw(maxlen) << "Tag Name" << ' '
                << std::left << std::setw(DTYPE_WIDTH) << "Type" << ' '
                << std::left << std::setw(STORAGE_WIDTH) << "Storage" << ' '
                << std::left << std::setw(MEM_WIDTH) << "Used" << ' '
                << std::left << std::setw(MEM_WIDTH) << "Alloc" << std::endl;
      std::cout.fill('-');
      std::cout << std::setw(maxlen) << '-' << ' '
                << std::setw(DTYPE_WIDTH) << '-' << ' '
                << std::setw(STORAGE_WIDTH) << '-' << ' '
                << std::setw(MEM_WIDTH) << '-' << ' '
                << std::setw(MEM_WIDTH) << '-' << std::endl;
      std::cout.fill(' ');
    }

      // print per-tag memory use
    for (ti = tags.begin(); ti != tags.end(); ++ti) {
      std::string name;
      rval = mb.tag_get_name( *ti, name );
      if (moab::MB_SUCCESS != rval || name.empty())
        name = ANON_TAG_NAME;

      unsigned long long occupied, allocated;
      mb.estimated_memory_use( 0, 0, 0, 0, 0, 0, 0, 0, &*ti, 1, &occupied, &allocated );

      std::cout << std::left << std::setw(maxlen) << name << ' '
                << std::right << std::setw(DTYPE_WIDTH) << tag_type_string(mb,*ti) <<  ' '
                << std::right << std::setw(STORAGE_WIDTH) << tag_storage_string(mb,*ti) << ' '
                << std::right << std::setw(MEM_WIDTH) << memstr(occupied) << ' '
                << std::right << std::setw(MEM_WIDTH) << memstr(allocated) << std::endl;
    }
  } // end per_tag
  
  if (totals) {
      // print summary of overall memory use
    get_mem_stats( mb, stats );
    std::cout << std::endl
              << "TOTAL: (Used/Allocated)" << std::endl
              << "memory:    " << memstr(stats.total_storage) << "/" << memstr(stats.total_amortized) << std::endl
              << "entity:    " << memstr(stats.entity_storage) << "/" << memstr(stats.entity_amortized) << std::endl
              << "adjacency: " << memstr(stats.adjacency_storage) << "/" << memstr(stats.adjacency_amortized) << std::endl
              << "tag:       " << memstr(stats.tag_storage) << "/" << memstr(stats.tag_amortized) << std::endl
              << std::endl;

  } // end totals

  if (sysstats) {
    std::FILE* filp = std::fopen("/proc/self/stat", "r");
    unsigned long long vsize;
    long rss;
    if (filp && 2 == std::fscanf(filp,
                  "%*d " // pid
                  "%*s " // comm
                  "%*c " // state
                  "%*d " // ppid
                  "%*d " // pgrp
                  "%*d " // session
                  "%*d " // tty_nr
                  "%*d " // tpgid
                  "%*u " // flags
                  "%*u " // minflt
                  "%*u " // cminflt
                  "%*u " // majflt
                  "%*u " // cmajflt
                  "%*u " // utime
                  "%*u " // stime
                  "%*d " // cutime
                  "%*d " // cstime
                  "%*d " // priority
                  "%*d " // nice
                  "%*d " // num_threads
                  "%*d " // itrealvalue
                  "%*u " // starttime
                  "%llu " // vsize
                  "%ld", // rss
                  &vsize, &rss )) {
  #ifndef _MSC_VER
      long long tmprss = rss * getpagesize();
  #endif
      std::cout << std::endl << "SYSTEM:" 
                << std::endl << "Virtual memory:    " << memstr(vsize)
  #ifndef _MSC_VER
                << std::endl << "Resident set size: " << memstr(tmprss)
  #endif
                << std::endl;
    }
    else {
  #ifndef _MSC_VER
      struct rusage sysdata;
      if (getrusage( RUSAGE_SELF, &sysdata )) {
        std::cerr << "getrusage failed" << std::endl;
      }
      else {
        rss = sysdata.ru_maxrss;
        long long tmprss = rss * getpagesize();
        std::cerr << std::endl << "SYSTEM:"
                  << std::endl << "Resident set size: " << memstr(tmprss) 
                  << std::endl;
      }
  #endif
    }
  } // end sysstats
}


bool is_zero( const MemStats& stats ) { return stats.total_amortized == 0; }

void get_mem_stats( moab::Interface& mb,
                    MemStats& data,
                    moab::EntityType type )
{
  if (type != moab::MBMAXTYPE) {
    moab::Range range;
    mb.get_entities_by_type( 0, type, range );
    mb.estimated_memory_use( range, 
                             &data.total_storage,
                             &data.total_amortized,
                             &data.entity_storage,
                             &data.entity_amortized,
                             &data.adjacency_storage,
                             &data.adjacency_amortized,
                             0, 0,
                             &data.tag_storage,
                             &data.tag_amortized );
  }
  else {
    mb.estimated_memory_use( 0, 0, 
                             &data.total_storage,
                             &data.total_amortized,
                             &data.entity_storage,
                             &data.entity_amortized,
                             &data.adjacency_storage,
                             &data.adjacency_amortized,
                             0, 0,
                             &data.tag_storage,
                             &data.tag_amortized );
  }
}

// rounded division
static unsigned long long rdiv( unsigned long long num, unsigned long long den )
{
  return (num + den/2) / den;
}

std::string memstr( unsigned long long val )
{
  const unsigned long long kb = 1024;
  const unsigned long long mb = kb*kb;
  const unsigned long long gb = kb*mb;
  const unsigned long long tb = kb*gb;
  
  std::ostringstream s;
  if (UNITS == HUMAN) {
    if (val >= 10*tb)
      s << rdiv( val, tb ) << "TB";
    else if (val >= 10*gb)
      s << rdiv( val, gb ) << "GB";
    else if (val >= 10*mb)
      s << rdiv( val, mb ) << "MB";
    else if (val >= 10*kb)
      s << rdiv( val, kb ) << "kB";
    else if (val > 0)
      s << val << " B";
    else 
      s << "0  ";
  }
  else {
    unsigned long long den = 1;
    switch (UNITS) {
      case BYTES: den = 1; break;
      case KILOBYTES: den = kb; break;
      case MEGABYTES: den = mb; break;
      case GIGABYTES: den = gb; break;
      case HUMAN: break; // handled above, list here to suppress warning
    }
    
    s << rdiv( val, den );
  }
  return s.str();
}

std::string tag_type_string( moab::Interface& mb, moab::Tag tag )
{
  moab::ErrorCode rval;
  std::ostringstream s;
  
  moab::DataType type;
  rval = mb.tag_get_data_type( tag, type );
  if (moab::MB_SUCCESS != rval)
    return std::string();

  int typesize;
  std::string typestr;
  switch (type) {
    case moab::MB_TYPE_INTEGER:
      typestr = "int";
      typesize = sizeof(int);
      break;
    case moab::MB_TYPE_DOUBLE:
      typestr = "double";
      typesize = sizeof(double);
      break;
    case moab::MB_TYPE_HANDLE:
      typestr = "handle";
      typesize = sizeof(moab::EntityHandle);
      break;
    case moab::MB_TYPE_BIT:
      typesize = 1;
      typestr = "bits";
      break;
    case moab::MB_TYPE_OPAQUE:
      typesize = 1;
      typestr = "bytes";
      break;
    default:
      typesize = 1;
      typestr = "???";
      break;
  }

  int size;
  rval = mb.tag_get_length( tag, size );
  if (moab::MB_VARIABLE_DATA_LENGTH == rval) 
    s << "VAR " << typestr;
  else if (moab::MB_SUCCESS == rval) 
    s << size/typesize << " " << typestr;
  // else do nothing
  
  return s.str();
}

std::string tag_storage_string( moab::Interface& mb, moab::Tag tag )
{
  moab::ErrorCode rval;
  moab::TagType type;
  rval = mb.tag_get_type( tag, type );
  if (moab::MB_SUCCESS != rval)
    return std::string();

  switch (type) {
    case moab::MB_TAG_DENSE: return "dense"; 
    case moab::MB_TAG_SPARSE: return "sparse"; 
    case moab::MB_TAG_BIT: return "bit"; 
    default: return "(none)"; 
  }
}

 
std::string center( const char* str, size_t width )
{
  std::string text(str);
  if (text.size() >= width)
    return text;
 
  width -= text.size();
  if (1u == width) {
    text += " ";
    return text;
  }
 
  std::ostringstream s;
  s << std::setw(width/2) << ' ' << text << std::setw(width/2 + width%2) << ' ';
  return s.str();
}

void do_test_mode() 
{
  const char prefix[] = "****************";
  moab::Core mbcore;
  moab::Interface& mb = mbcore;
  moab::ErrorCode rval;
  moab::Range handles;
  moab::EntityHandle h;
  moab::Range::iterator jt, it;
  const unsigned N = 1000;
  
  // creating some vertices
  double coords[3] = { 1, 2, 3 };
  for (unsigned i = 0; i < N; ++i) 
    mb.create_vertex( coords, h );
  std::cout << std::endl << prefix << "Created " << N << " vertices" << std::endl;
  print_memory_stats( mb, true, false, true, true );
  
  for (unsigned i = 0; i < N; ++i) 
    mb.create_vertex( coords, h );
  std::cout << std::endl << prefix << "Created another " << N << " vertices" << std::endl;
  print_memory_stats( mb, true, false, true, true );
  
  for (int i = 0; i < 100; ++i) {      
    for (unsigned j = 0; j < N; ++j) 
      mb.create_vertex( coords, h );
  }
  std::cout << std::endl << prefix << "Created another " << 100*N << " vertices" << std::endl;
  print_memory_stats( mb, true, false, true, true );
  
  // create some elements
  handles.clear();
  mb.get_entities_by_type( 0, moab::MBVERTEX, handles );
  it = handles.begin();
  for (unsigned i = 0; i < N-2; ++i, ++it) {
    jt = it;
    moab::EntityHandle conn[3];
    conn[0] = *jt; ++jt;
    conn[1] = *jt; ++jt;
    conn[2] = *jt; ++jt;
    mb.create_element( moab::MBTRI, conn, 3, h );
  }
  std::cout << std::endl << prefix << "Created " << N-2 << " triangles" << std::endl;
  print_memory_stats( mb, true, false, true, true );
  
  it = handles.begin();
  for (unsigned i = 0; i < N-3; ++i, ++it) {
    jt = it;
    moab::EntityHandle conn[4];
    conn[0] = *jt; ++jt;
    conn[1] = *jt; ++jt;
    conn[2] = *jt; ++jt;
    conn[3] = *jt; ++jt;
    mb.create_element( moab::MBQUAD, conn, 4, h );
  }
  std::cout << std::endl << prefix << "Created " << N-3 << " quads" << std::endl;
  print_memory_stats( mb, true, false, true, true );
  
  for (int i = 0; i < 100; ++i) {
    it = handles.begin();
    for (unsigned j = 0; j < N-3; ++j, ++it) {
      jt = it;
      moab::EntityHandle conn[4];
      conn[0] = *jt; ++jt;
      conn[1] = *jt; ++jt;
      conn[2] = *jt; ++jt;
      conn[3] = *jt; ++jt;
      mb.create_element( moab::MBQUAD, conn, 4, h );
    }
  }
  std::cout << std::endl << prefix << "Created another " << 100*(N-3) << " quads" << std::endl;
  print_memory_stats( mb, true, false, true, true );

  // set global ID
  moab::Tag tag;
  rval = mb.tag_get_handle( "GLOBAL_ID", 1, moab::MB_TYPE_INTEGER, tag );
  if (moab::MB_SUCCESS != rval) {
    std::cerr << "Failed to get GLOBAL_ID tag handle" << std::endl;
    return;
  }
  handles.clear();
  mb.get_entities_by_type( 0, moab::MBVERTEX, handles );
  int id = 1;
  for (it = handles.begin(); it != handles.end(); ++it) {
    mb.tag_set_data( tag, &*it, 1, &id );
    ++id;
  }
  std::cout << std::endl << prefix << "Set global ID tag on " << handles.size() << " vertices" << std::endl;
  print_memory_stats( mb, true, true, true, true );
  
  handles.clear();
  mb.get_entities_by_type( 0, moab::MBQUAD, handles );
  id = 1;
  for (it = handles.begin(); it != handles.end(); ++it) {
    mb.tag_set_data( tag, &*it, 1, &id );
    ++id;
  }
  std::cout << std::endl << prefix << "Set global ID tag on " << handles.size() << " quads" << std::endl;
  print_memory_stats( mb, true, true, true, true );
  
  // create and set a sparse tag
  mb.tag_get_handle( "mem_test_tag", 3, moab::MB_TYPE_DOUBLE, tag, moab::MB_TAG_SPARSE|moab::MB_TAG_CREAT);
  handles.clear();
  mb.get_entities_by_type( 0, moab::MBVERTEX, handles );
  for (it = handles.begin(); it != handles.end(); ++it) {
    mb.get_coords( &*it, 1, coords );
    mb.tag_set_data( tag, &*it, 1, coords );
  }
  std::cout << std::endl << prefix << "Copied vertex coords to sparse tag for " << handles.size() << " vertices" << std::endl;
  print_memory_stats( mb, true, true, true, true );
  
  // create and set bit tag
  mb.tag_get_handle( "mem_test_bit", 1, moab::MB_TYPE_BIT, tag, moab::MB_TAG_CREAT );
  handles.clear();
  mb.get_entities_by_type( 0, moab::MBTRI, handles );
  for (it = handles.begin(); it != handles.end(); ++it) {
    char byte = '\001';
    mb.tag_set_data( tag, &*it, 1, &byte );
  }
  std::cout << std::endl << prefix << "Set 1-bit tag for " << handles.size() << " triangles" << std::endl;
  print_memory_stats( mb, true, true, true, true );
  
  // create vertex to element adjacency data
  handles.clear();
  mb.get_entities_by_type( 0, moab::MBVERTEX, handles );
  std::vector<moab::EntityHandle> adj_vec;
  mb.get_adjacencies( &*handles.begin(), 1, 2, false, adj_vec );
  std::cout << std::endl << prefix << "Created vertex-to-element adjacencies" << std::endl;
  print_memory_stats( mb, true, false, true, true );
  std::cout << std::endl;
}

  
