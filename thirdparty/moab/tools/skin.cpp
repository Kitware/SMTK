#include <iostream>
#include <set>
#include <limits>
#include <time.h>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <assert.h>
#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <fcntl.h>
#include "moab/Interface.hpp"
#include "MBTagConventions.hpp"
#include "moab/Core.hpp"
#include "moab/Range.hpp"
#include "moab/Skinner.hpp"
#include "moab/AdaptiveKDTree.hpp"
#include "moab/CN.hpp"

using namespace moab;

static void get_time_mem(double &tot_time, double &tot_mem);

// Different platforms follow different conventions for usage
#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <sys/resource.h>
#endif
#ifdef SOLARIS
extern "C" int getrusage(int, struct rusage *);
#ifndef RUSAGE_SELF
#include </usr/ucbinclude/sys/rusage.h>
#endif
#endif

const char DEFAULT_FIXED_TAG[] = "fixed"; 
const int MIN_EDGE_LEN_DENOM = 4;

#define CHKERROR( A ) do { if (MB_SUCCESS != (A)) { \
 std::cerr << "Internal error at line " << __LINE__ << std::endl; \
 return 3; } } while(false)

static ErrorCode merge_duplicate_vertices( Interface&, double epsilon );
static ErrorCode min_edge_length( Interface&, double& result );

static void usage( const char* argv0, bool help = false ) 
{
  std::ostream& str = help ? std::cout : std::cerr;

  str << "Usage: " << argv0 
      << " [-b <block_num> [-b ...] ] [-l] [-m] [-M <n>] [-p] [-s <sideset_num>] [-S] [-t|-T <name>] [-w] [-v|-V <n>]"
      << " <input_file> [<output_file>]" << std::endl;
  str << "Help : " << argv0 << " -h" << std::endl;
  if (!help)
    exit(1);
    
  str << "Options: " << std::endl;
  str << "-a : Compute skin using vert-elem adjacencies (more memory, less time)." << std::endl;
  str << "-b <block_num> : Compute skin only for material set/block <block_num>." << std::endl;
  str << "-p : Print cpu & memory performance." << std::endl;
  str << "-s <sideset_num> : Put skin in neumann set/sideset <sideset_num>." << std::endl;
  str << "-S : Look for and use structured mesh information to speed up skinning." << std::endl;
  str << "-t : Set '" << DEFAULT_FIXED_TAG << "' tag on skin vertices." << std::endl;
  str << "-T <name> : Create tag with specified name and set to 1 on skin vertices." << std::endl;
  str << "-w : Write out whole mesh (otherwise just writes skin)." << std::endl;
  str << "-m : consolidate duplicate vertices" << std::endl;
  str << "-M <n> : consolidate duplicate vertices with specified tolerance. "
          "(Default: min_edge_length/" << MIN_EDGE_LEN_DENOM << ")" << std::endl;
  str << "-l : List total numbers of entities and vertices in skin." << std::endl;
  exit(0);
}


int main( int argc, char* argv[] )
{
  int i = 1;
  std::vector<int> matsets;
  int neuset_num = -1;
  bool write_tag = false, write_whole_mesh = false;
  bool print_perf = false;
  bool use_vert_elem_adjs = false;
  bool merge_vertices = false;
  double merge_epsilon = -1;
  bool list_skin = false;
  bool use_scd = false;
  const char* fixed_tag = DEFAULT_FIXED_TAG;
  const char *input_file = 0, *output_file = 0;
  
  bool no_more_flags = false;
  char* endptr = 0;
  long block = 0; // initialize to eliminate compiler warning
  while (i < argc) {
    if (!no_more_flags && argv[i][0] == '-') {
      const int f = i++;
      for (int j = 1; argv[f][j]; ++j) {
        switch (argv[f][j]) {
          case 'a': use_vert_elem_adjs = true; break;
          case 'p': print_perf = true;         break;
          case 't': write_tag = true;          break;
          case 'w': write_whole_mesh = true;   break;
          case 'm': merge_vertices = true;     break;
          case '-': no_more_flags = true;      break;
          case 'h': usage( argv[0], true );    break;
          case 'l': list_skin = true;          break;
          case 'S': use_scd = true;            break;
          case 'b': 
            if (i == argc || 0 >= (block = strtol(argv[i],&endptr,0)) || *endptr) {
              std::cerr << "Expected positive integer following '-b' flag" << std::endl;
              usage(argv[0]);
            }
            matsets.push_back((int)block);
            ++i;
            break;
          case 's':
            if (i == argc || 0 >= (neuset_num = strtol(argv[i],&endptr,0)) || *endptr) {
              std::cerr << "Expected positive integer following '-s' flag" << std::endl;
              usage(argv[0]);
            }
            ++i;
            break;
          case 'T':
            if (i == argc || argv[i][0] == '-') {
              std::cerr << "Expected tag name following '-T' flag" << std::endl;
              usage(argv[0]);
            }
            fixed_tag = argv[i++];
            break;
          case 'M':  
            if (i == argc || 0.0 > (merge_epsilon = strtod(argv[i],&endptr)) || *endptr) {
              std::cerr << "Expected positive numeric value following '-M' flag" << std::endl;
              usage(argv[0]);
            }
            merge_vertices = true;
            ++i;
            break;
          default:
            std::cerr << "Unrecognized flag: '" << argv[f][j] << "'" << std::endl;
            usage(argv[0]);
            break;
        }
      }
    }
    else if (input_file && output_file) {
      std::cerr << "Extra argument: " << argv[i] << std::endl;
      usage(argv[0]);
    }
    else if (input_file) {
      output_file = argv[i++];
    }
    else {
      input_file = argv[i++];
    }
  }

  if (!input_file) {
    std::cerr << "No input file specified" << std::endl;
    usage(argv[0]);
  }

  
  ErrorCode result;
  Core mbimpl;
  Interface* iface = &mbimpl;
  
  if (print_perf) {
    double tmp_time1, tmp_mem1;
    get_time_mem(tmp_time1, tmp_mem1);
    std::cout << "Before reading: cpu time = " << tmp_time1 << ", memory = " 
              << tmp_mem1/1.0e6 << "MB." << std::endl;
  }

    // read input file
  result = iface->load_mesh( input_file );
  if (MB_SUCCESS != result)
  { 
    std::cerr << "Failed to load \"" << input_file << "\"." << std::endl; 
    return 2;
  }
  std::cerr << "Read \"" << input_file << "\"" << std::endl;
  if (print_perf) {
    double tmp_time2, tmp_mem2;
    get_time_mem(tmp_time2, tmp_mem2);
    std::cout << "After reading: cpu time = " << tmp_time2 << ", memory = " 
              << tmp_mem2/1.0e6 << "MB." << std::endl;
  }
  
  if (merge_vertices) {
    if (merge_epsilon < 0.0) {
      if (MB_SUCCESS != min_edge_length( *iface, merge_epsilon )) {
        std::cerr << "Error determining minimum edge length" << std::endl;
        return 1;
      }
      merge_epsilon /= MIN_EDGE_LEN_DENOM;
    }
    if (MB_SUCCESS != merge_duplicate_vertices( *iface, merge_epsilon )) {
      std::cerr << "Error merging duplicate vertices" << std::endl;
      return 1;
    }
  }
  
    // get entities of largest dimension
  int dim = 4;
  Range entities;
  while (entities.empty() && dim > 1)
  {
    dim--;
    result = iface->get_entities_by_dimension( 0, dim, entities );
    CHKERROR(result);
  }

  Range skin_ents;
  Tag matset_tag = 0, neuset_tag = 0;
  result = iface->tag_get_handle(MATERIAL_SET_TAG_NAME, 1, MB_TYPE_INTEGER, matset_tag);
  if (MB_SUCCESS != result) return 1;
  result = iface->tag_get_handle(NEUMANN_SET_TAG_NAME, 1, MB_TYPE_INTEGER, neuset_tag);
  if (MB_SUCCESS != result) return 1;

  if (matsets.empty()) skin_ents = entities;
  else {
      // get all entities in the specified blocks
    if (0 == matset_tag) {
      std::cerr << "Couldn't find any material sets in this mesh." << std::endl;
      return 1;
    }
    
    for (std::vector<int>::iterator vit = matsets.begin(); vit != matsets.end(); vit++) {
      int this_matset = *vit;
      const void *this_matset_ptr = &this_matset;
      Range this_range, ent_range;
      result = iface->get_entities_by_type_and_tag(0, MBENTITYSET, &matset_tag,
                                                    &this_matset_ptr, 1, this_range);
      if (MB_SUCCESS != result) {
        std::cerr << "Trouble getting material set #" << *vit << std::endl;
        return 1;
      }
      else if (this_range.empty()) {
        std::cerr << "Warning: couldn't find material set " << *vit << std::endl;
        continue;
      }
      
      result = iface->get_entities_by_dimension(*this_range.begin(), dim, ent_range, true);
      if (MB_SUCCESS != result) continue;
      skin_ents.merge(ent_range);
    }
  }
  
  if (skin_ents.empty()) {
    std::cerr << "No entities for which to compute skin; exiting." << std::endl;
    return 1;
  }

  if (use_vert_elem_adjs) {
      // make a call which we know will generate vert-elem adjs
    Range dum_range;
    result = iface->get_adjacencies(&(*skin_ents.begin()), 1, 1, false,
                                    dum_range);
    if (MB_SUCCESS != result)
      return 1;
  }
  
  double tmp_time = 0.0, tmp_mem = 0.0;
  if (print_perf) {
    get_time_mem(tmp_time, tmp_mem);
    std::cout << "Before skinning: cpu time = " << tmp_time << ", memory = " 
              << tmp_mem/1.0e6 << "MB." << std::endl;
  }

    // skin the mesh
  Range forward_lower, reverse_lower;
  Skinner tool( iface );
  if (use_scd) 
    result = tool.find_skin( 0, skin_ents, false, forward_lower, NULL, false, true, true);
  else
    result = tool.find_skin( 0, skin_ents, false, forward_lower, &reverse_lower );
  Range boundary;
  boundary.merge( forward_lower );
  boundary.merge( reverse_lower );
  if (MB_SUCCESS != result || boundary.empty())
  {
    std::cerr << "Mesh skinning failed." << std::endl;
    return 3;
  }

  if (list_skin) {
    Range skin_verts;
    result = iface->get_adjacencies(boundary, 0, true, skin_verts, Interface::UNION);
    std::cout << "Skin has "; 
    if (skin_ents.num_of_dimension(3)) 
      std::cout << boundary.num_of_dimension(2) << " faces and ";
    else if (skin_ents.num_of_dimension(2)) 
      std::cout << boundary.num_of_dimension(1) << " edges and ";
    std::cout << skin_verts.size() << " vertices." << std::endl;
  }
  if (write_tag) {
      // get tag handle
    Tag tag;
    int zero = 0;
    result = iface->tag_get_handle( fixed_tag, 1, MB_TYPE_INTEGER, tag, MB_TAG_DENSE|MB_TAG_CREAT, &zero );
    CHKERROR(result);
  
      // Set tags
    std::vector<int> ones;
    Range bverts;
    result = iface->get_adjacencies(boundary, 0, false, bverts, Interface::UNION);
    if (MB_SUCCESS != result) {
      std::cerr << "Trouble getting vertices on boundary." << std::endl;
      return 1;
    }
    ones.resize( bverts.size(), 1 );
    result = iface->tag_set_data( tag, bverts, &ones[0] );
    CHKERROR(result);
  }
  
  if (-1 != neuset_num) {
      // create a neumann set with these entities
    if (0 == neuset_tag) {
      result = iface->tag_get_handle( "NEUMANN_SET_TAG_NAME", 1, MB_TYPE_INTEGER,
                                       neuset_tag, MB_TAG_SPARSE|MB_TAG_CREAT );
      if (MB_SUCCESS != result || 0 == neuset_tag) return 1;
    }
    

      // always create a forward neumann set, assuming we have something in the set
    EntityHandle forward_neuset = 0;
    result = iface->create_meshset(MESHSET_SET, forward_neuset);
    if (MB_SUCCESS != result || 0 == forward_neuset) return 1;
    result = iface->tag_set_data(neuset_tag, &forward_neuset, 1, &neuset_num);
    if (MB_SUCCESS != result) return 1;

    if (!forward_lower.empty()) {
      result = iface->add_entities(forward_neuset, forward_lower);
      if (MB_SUCCESS != result) return 1;
    }
    if (!reverse_lower.empty()) {
      EntityHandle reverse_neuset = 1;
      result = iface->create_meshset(MESHSET_SET, reverse_neuset);
      if (MB_SUCCESS != result || 0 == forward_neuset) return 1;

      result = iface->add_entities(reverse_neuset, reverse_lower);
      if (MB_SUCCESS != result) return 1;
      Tag sense_tag;
      int dum_sense = 0;
      result = iface->tag_get_handle("SENSE", 1, MB_TYPE_INTEGER, sense_tag, MB_TAG_SPARSE|MB_TAG_CREAT, &dum_sense);
      if (result != MB_SUCCESS) return 1;
      int sense_val = -1;
      result = iface->tag_set_data(neuset_tag, &reverse_neuset, 1, &sense_val);
      if (MB_SUCCESS != result) return 0;
      result = iface->add_entities(forward_neuset, &reverse_neuset, 1);
      if (MB_SUCCESS != result) return 0;
    }
  }

  if (NULL != output_file && write_whole_mesh) {
    
      // write output file
    result = iface->write_mesh( output_file);
    if (MB_SUCCESS != result)
    { 
      std::cerr << "Failed to write \"" << output_file << "\"." << std::endl; 
      return 2;
    }
    std::cerr << "Wrote \"" << output_file << "\"" << std::endl;
  }
  else if (NULL != output_file) {
      // write only skin; write them as one set
    EntityHandle skin_set;
    result = iface->create_meshset(MESHSET_SET, skin_set);
    if (MB_SUCCESS != result) return 1;
    result = iface->add_entities(skin_set, forward_lower);
    if (MB_SUCCESS != result) return 1;
    result = iface->add_entities(skin_set, reverse_lower);
    if (MB_SUCCESS != result) return 1;

    int dum = 10000;
    result = iface->tag_set_data(matset_tag, &skin_set, 1, &dum);
    if (MB_SUCCESS != result) return 1;

    result = iface->write_mesh( output_file, &skin_set, 1);
    if (MB_SUCCESS != result)
    { 
      std::cerr << "Failed to write \"" << output_file << "\"." << std::endl; 
      return 2;
    }
    std::cerr << "Wrote \"" << output_file << "\"" << std::endl;
  }

  if (print_perf) {
    double tot_time, tot_mem;
    get_time_mem(tot_time, tot_mem);
    std::cout << "Total cpu time = " << tot_time << " seconds." << std::endl;
    std::cout << "Total skin cpu time = " << tot_time-tmp_time << " seconds." << std::endl;
    std::cout << "Total memory = " << tot_mem/1024 << " MB." << std::endl;
    std::cout << "Total skin memory = " << (tot_mem-tmp_mem)/1024 << " MB." << std::endl;
    std::cout << "Entities: " << std::endl;
    iface->list_entities(0, 0);
  }
  
  return 0;
}

#if defined(_MSC_VER) || defined(__MINGW32__)
void get_time_mem(double &tot_time, double &tot_mem) 
{
  tot_time = (double)clock() / CLOCKS_PER_SEC;
  tot_mem = 0;
}
#else
void get_time_mem(double &tot_time, double &tot_mem) 
{
  struct rusage r_usage;
  getrusage(RUSAGE_SELF, &r_usage);
  double utime = (double)r_usage.ru_utime.tv_sec +
    ((double)r_usage.ru_utime.tv_usec/1.e6);
  double stime = (double)r_usage.ru_stime.tv_sec +
    ((double)r_usage.ru_stime.tv_usec/1.e6);
  tot_time = utime + stime;
  tot_mem = 0;
  if (0 != r_usage.ru_maxrss) {
    tot_mem = (double)r_usage.ru_maxrss; 
  }
  else {
      // this machine doesn't return rss - try going to /proc
      // print the file name to open
    char file_str[4096], dum_str[4096];
    int file_ptr = -1, file_len;
    file_ptr = open("/proc/self/stat", O_RDONLY);
    file_len = read(file_ptr, file_str, sizeof(file_str)-1);
    if (file_len == 0) {
      close(file_ptr);
      return;
    }

    close(file_ptr);
    file_str[file_len] = '\0';
      // read the preceeding fields and the ones we really want...
    int dum_int;
    unsigned int dum_uint, vm_size, rss;
    int num_fields = sscanf(file_str, 
                            "%d " // pid
                            "%s " // comm
                            "%c " // state
                            "%d %d %d %d %d " // ppid, pgrp, session, tty, tpgid
                            "%u %u %u %u %u " // flags, minflt, cminflt, majflt, cmajflt
                            "%d %d %d %d %d %d " // utime, stime, cutime, cstime, counter, priority
                            "%u %u " // timeout, itrealvalue
                            "%d " // starttime
                            "%u %u", // vsize, rss
                            &dum_int, 
                            dum_str, 
                            dum_str, 
                            &dum_int, &dum_int, &dum_int, &dum_int, &dum_int, 
                            &dum_uint, &dum_uint, &dum_uint, &dum_uint, &dum_uint,
                            &dum_int, &dum_int, &dum_int, &dum_int, &dum_int, &dum_int, 
                            &dum_uint, &dum_uint, 
                            &dum_int,
                            &vm_size, &rss);
    if (num_fields == 24)
      tot_mem = ((double)vm_size);
  }
}
#endif
  
  
  
ErrorCode min_edge_length( Interface& moab, double& result )
{
  double sqr_result = std::numeric_limits<double>::max();
  
  ErrorCode rval;
  Range entities;
  rval = moab.get_entities_by_handle( 0, entities );
  if (MB_SUCCESS != rval) return rval;
  Range::iterator i = entities.upper_bound( MBVERTEX );
  entities.erase( entities.begin(), i );
  i = entities.lower_bound( MBENTITYSET );
  entities.erase( i, entities.end() );
  
  std::vector<EntityHandle> storage;
  for (i = entities.begin(); i != entities.end(); ++i) {
    EntityType t = moab.type_from_handle( *i );
    const EntityHandle* conn;
    int conn_len, indices[2];
    rval = moab.get_connectivity( *i, conn, conn_len, true, &storage );
    if (MB_SUCCESS != rval) return rval;
    
    int num_edges = CN::NumSubEntities( t, 1 );
    for (int j = 0; j < num_edges; ++j) {
      CN::SubEntityVertexIndices( t, 1, j, indices );
      EntityHandle v[2] = { conn[indices[0]], conn[indices[1]] };
      if (v[0] == v[1])
        continue;
      
      double c[6];
      rval = moab.get_coords( v, 2, c );
      if (MB_SUCCESS != rval) return rval;
      
      c[0] -= c[3];
      c[1] -= c[4];
      c[2] -= c[5];
      double len_sqr = c[0]*c[0] + c[1]*c[1] + c[2]*c[2];
      if (len_sqr < sqr_result)
        sqr_result = len_sqr;
    }
  }
  
  result = sqrt(sqr_result);
  return MB_SUCCESS;
}
  
  
  
ErrorCode merge_duplicate_vertices( Interface& moab, const double epsilon )
{
  ErrorCode rval;
  Range verts;
  rval = moab.get_entities_by_type( 0, MBVERTEX, verts );
  if (MB_SUCCESS != rval)
    return rval;
  
  AdaptiveKDTree tree( &moab );
  EntityHandle root;
  rval = tree.build_tree( verts, &root );
  if (MB_SUCCESS != rval) {
    fprintf(stderr,"Failed to build kD-tree.\n");
    return rval;
  }
  
  std::set<EntityHandle> dead_verts;
  std::vector<EntityHandle> leaves;
  for (Range::iterator i = verts.begin(); i != verts.end(); ++i) {
    double coords[3];
    rval = moab.get_coords( &*i, 1, coords );
    if (MB_SUCCESS != rval) return rval;
    
    leaves.clear();;
    rval = tree.distance_search(coords, epsilon, leaves, epsilon, epsilon);
    if (MB_SUCCESS != rval) return rval;
    
    Range near;
    for (std::vector<EntityHandle>::iterator j = leaves.begin(); j != leaves.end(); ++j) {
      Range tmp;
      rval = moab.get_entities_by_type( *j, MBVERTEX, tmp );
      if (MB_SUCCESS != rval)
        return rval;
      near.merge( tmp.begin(), tmp.end() );
    }
    
    Range::iterator v = near.find( *i );
    assert( v != near.end() );
    near.erase( v );
    
    EntityHandle merge = 0;
    for (Range::iterator j = near.begin(); j != near.end(); ++j) {
      if (*j < *i && dead_verts.find( *j ) != dead_verts.end())
        continue;
      
      double coords2[3];
      rval = moab.get_coords( &*j, 1, coords2 );
      if (MB_SUCCESS != rval) return rval;
      
      coords2[0] -= coords[0];
      coords2[1] -= coords[1];
      coords2[2] -= coords[2];
      double dsqr = coords2[0]*coords2[0] +
                    coords2[1]*coords2[1] +
                    coords2[2]*coords2[2];
      if (dsqr <= epsilon*epsilon) {
        merge = *j;
        break;
      }
    }
    
    if (merge) {
      dead_verts.insert(*i);
      rval = moab.merge_entities( merge, *i, false, true );
      if (MB_SUCCESS != rval) return rval;
    }
  }
  
  if (dead_verts.empty()) 
    std::cout << "No duplicate/coincident vertices." << std::endl;
  else
    std::cout << "Merged and deleted " << dead_verts.size() << " vertices "
              << "coincident within " << epsilon << std::endl;
  
  return MB_SUCCESS;
}

    

