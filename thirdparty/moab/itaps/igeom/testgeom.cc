/**
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 */
/**
 * \file testgeom.cc
 *
 * \brief testgeom, a unit test for the ITAPS geometry interface
 *
 */
#include "FBiGeom.h"
#include <iostream>
#include <set>
#include <algorithm>
#include <vector>
#include <iterator>
#include <algorithm>
#include <iomanip>
#include <assert.h>
#include <string.h>
#include <math.h>
#define CHECK( STR ) if (err != iBase_SUCCESS) return print_error( STR, err, geom, __FILE__, __LINE__ )

#define STRINGIFY(S) XSTRINGIFY(S)
#define XSTRINGIFY(S) #S

static bool print_error( const char* desc, 
                         int err,
                         FBiGeom_Instance geom,
                         const char* file,
                         int line )
{
  char buffer[1024];
  FBiGeom_getDescription( geom, buffer, sizeof(buffer) );
  buffer[sizeof(buffer)-1] = '\0';
  
  std::cerr << "ERROR: " << desc << std::endl
            << "  Error code: " << err << std::endl
            << "  Error desc: " << buffer << std::endl
            << "  At        : " << file << ':' << line << std::endl
            ;
  
  return false; // must always return false or CHECK macro will break
}

typedef iBase_TagHandle TagHandle;
typedef iBase_EntityHandle GentityHandle;
typedef iBase_EntitySetHandle GentitysetHandle;

/* Frees allocated arrays for us */
template <typename T> class SimpleArray
{
  private:
    T* arr;
    int arrSize;
    int arrAllocated;
     
  public:
    SimpleArray() : arr(0) , arrSize(0), arrAllocated(0) {}
    SimpleArray( unsigned s ) :arrSize(s), arrAllocated(s) {
      arr = (T*)malloc(s*sizeof(T));
      for (unsigned i = 0; i < s; ++i)
        new (arr+i) T();
    }
    
    ~SimpleArray() {
      for (int i = 0; i < size(); ++i)
        arr[i].~T();
      free(arr);
    }

    T**  ptr()            { return &arr; }
    int& size()           { return arrSize; }
    int  size()     const { return arrSize; }
    int& capacity()       { return arrAllocated; }
    int  capacity() const { return arrAllocated; }
    
    typedef T* iterator;
    typedef const T* const_iterator;
    iterator       begin()       { return arr; }
    const_iterator begin() const { return arr; }
    iterator         end()       { return arr + arrSize; }
    const_iterator   end() const { return arr + arrSize; }
    
    
    T& operator[]( unsigned idx )       { return arr[idx]; }
    T  operator[]( unsigned idx ) const { return arr[idx]; }
};

#define ARRAY_INOUT( A ) A.ptr(), &A.capacity(), &A.size()
#define ARRAY_IN( A ) &A[0], A.size()

bool gLoad_test(const std::string filename, FBiGeom_Instance);

bool tags_test(FBiGeom_Instance geom);
bool tag_get_set_test(FBiGeom_Instance geom);
bool tag_info_test(FBiGeom_Instance geom);
bool gentityset_test(FBiGeom_Instance geom, bool /*multiset*/, bool /*ordered*/);
bool topology_adjacencies_test(FBiGeom_Instance geom);
bool geometry_evaluation_test(FBiGeom_Instance geom);
bool construct_test(FBiGeom_Instance geom);
bool primitives_test(FBiGeom_Instance geom);
bool transforms_test(FBiGeom_Instance geom);
bool booleans_test(FBiGeom_Instance geom);
bool shutdown_test(FBiGeom_Instance geom, std::string &engine_opt);
bool save_entset_test(FBiGeom_Instance geom);
bool mesh_size_test(FBiGeom_Instance geom);

void handle_error_code(const bool result,
                       int &number_failed,
                       int &/*number_not_implemented*/,
                       int &number_successful)
{
  if (result) {
    std::cout << "Success";
    number_successful++;
  }
  else {
    std::cout << "Failure";    
    number_failed++;
  }
}

int main( int argc, char *argv[] )
{
    std::string filename = STRINGIFY(MESHDIR) "/shell.h5m";
    std::string engine_opt;

  if (argc == 1) {
    std::cout << "Using default input file: " << filename << std::endl;
  }
  else if (argc == 2) {
    filename = argv[1];
  }
  else {
    std::cerr << "Usage: " << argv[0] << " [geom_filename]" << std::endl;
    return 1;
  }

  bool result;
  int number_tests = 0;
  int number_tests_successful = 0;
  int number_tests_not_implemented = 0;
  int number_tests_failed = 0;

    // initialize the Mesh
  int err;
  FBiGeom_Instance geom;
  FBiGeom_newGeom( engine_opt.c_str(), &geom, &err, engine_opt.length() );

    // Print out Header information
  std::cout << "\n\nITAPS GEOMETRY INTERFACE TEST PROGRAM:\n\n";
    // gLoad test
  std::cout << "   gLoad: ";
  result = gLoad_test(filename, geom);
  handle_error_code(result, number_tests_failed,
                    number_tests_not_implemented,
                    number_tests_successful);
  number_tests++;
  std::cout << "\n";
  
    // tags test
  std::cout << "   tags: ";
  result = tags_test(geom);
  handle_error_code(result, number_tests_failed,
                    number_tests_not_implemented,
                    number_tests_successful);
  number_tests++;
  std::cout << "\n";
  /*
    // gentitysets test
  std::cout << "   gentity sets: ";
  result = gentityset_test(geom, false, false);
  handle_error_code(result, number_tests_failed,
                    number_tests_not_implemented,
                    number_tests_successful);
  number_tests++;
  std::cout << "\n";
  */
    // topology adjacencies test
  std::cout << "   topology adjacencies: ";
  result = topology_adjacencies_test(geom);
  handle_error_code(result, number_tests_failed,
                    number_tests_not_implemented,
                    number_tests_successful);
  number_tests++;
  std::cout << "\n";

    // geometry evaluation test
  std::cout << "   geometry evaluation: ";
  result = geometry_evaluation_test(geom);
  handle_error_code(result, number_tests_failed,
                    number_tests_not_implemented,
                    number_tests_successful);
  number_tests++;
  std::cout << "\n";
  /*
    // construct test
  std::cout << "   construct: ";
  result = construct_test(geom);
  handle_error_code(result, number_tests_failed,
                    number_tests_not_implemented,
                    number_tests_successful);
  number_tests++;
  std::cout << "\n";
  
    // primitives test
  std::cout << "   primitives: ";
  result = primitives_test(geom);
  handle_error_code(result, number_tests_failed,
                    number_tests_not_implemented,
                    number_tests_successful);
  number_tests++;
  std::cout << "\n";

    // transforms test
  std::cout << "   transforms: ";
  result = transforms_test(geom);
  handle_error_code(result, number_tests_failed,
                    number_tests_not_implemented,
                    number_tests_successful);
  number_tests++;
  std::cout << "\n";

    // booleans test
  std::cout << "   booleans: ";
  result = booleans_test(geom);
  handle_error_code(result, number_tests_failed,
                    number_tests_not_implemented,
                    number_tests_successful);
  number_tests++;
  std::cout << "\n";
  
#if defined(HAVE_ACIS) && !defined(FORCE_OCC)
  std::cout << "   mesh size: ";
  result = mesh_size_test(geom);
  handle_error_code(result, number_tests_failed,
                    number_tests_not_implemented,
                    number_tests_successful);
  number_tests++;
  std::cout << "\n";

    // save entset test
  std::cout << "   save entset: ";
  result = save_entset_test(geom);
  handle_error_code(result, number_tests_failed,
                    number_tests_not_implemented,
                    number_tests_successful);
  number_tests++;
  std::cout << "\n";
#endif  
  */  
    // shutdown test
  std::cout << "   shutdown: ";
  result = shutdown_test(geom, engine_opt);
  handle_error_code(result, number_tests_failed,
                    number_tests_not_implemented,
                    number_tests_successful);
  number_tests++;
  std::cout << "\n";
  

    // summary

  std::cout << "\nTSTT TEST SUMMARY: \n"
            << "   Number Tests:           " << number_tests << "\n"
            << "   Number Successful:      " << number_tests_successful << "\n"
            << "   Number Not Implemented: " << number_tests_not_implemented << "\n"
            << "   Number Failed:          " << number_tests_failed 
            << "\n\n" << std::endl;
  
  return number_tests_failed;
}

/*!
  @test 
  Load Mesh
  @li Load a mesh file
*/
bool gLoad_test(const std::string filename, FBiGeom_Instance geom)
{
  int err;
  FBiGeom_load( geom, &filename[0], 0, &err, filename.length(), 0 );
  CHECK( "ERROR : can not load a geometry" );
  
  iBase_EntitySetHandle root_set;
  FBiGeom_getRootSet( geom, &root_set, &err );
  CHECK( "ERROR : getRootSet failed!" );
  
    // print out the number of entities
  std::cout << "Model contents: " << std::endl;
  const char *gtype[] = {"vertices: ", "edges: ", "faces: ", "regions: "};
  for (int i = 0; i <= 3; ++i) {
    int count;
    FBiGeom_getNumOfType( geom, root_set, i, &count, &err );
    CHECK( "Error: problem getting entities after gLoad." );
    std::cout << gtype[i] << count << std::endl;
  }

  return true;
}

/*!
  @test 
  Test tag creating, reading, writing, deleting
  @li Load a mesh file
*/
bool tags_test(FBiGeom_Instance geom)
{
  bool success = true;

  success = tag_info_test(geom);
  if (!success) return success;
  
  success = tag_get_set_test(geom);
  if (!success) return success;

  return true;
}

bool tag_info_test(FBiGeom_Instance geom) 
{
  int err;
  
  iBase_EntitySetHandle root_set;
  FBiGeom_getRootSet( geom, &root_set, &err );
  CHECK( "ERROR : getRootSet failed!" );

    // create an arbitrary tag, size 4
  iBase_TagHandle this_tag, tmp_handle;
  std::string tag_name("tag_info tag"), tmp_name;
  FBiGeom_createTag( geom, &tag_name[0], 4, iBase_BYTES, &this_tag, &err, tag_name.length() );
  CHECK( "ERROR : can not create a tag." );

    // get information on the tag
  
  char name_buffer[256];
  FBiGeom_getTagName( geom, this_tag, name_buffer, &err, sizeof(name_buffer) );
  CHECK( "ERROR : Couldn't get tag name." );
  if (tag_name != name_buffer) {
    std::cerr << "ERROR: getTagName returned '" << name_buffer 
              << "' for tag created as '" << tag_name << "'" << std::endl;
    return false;
  }
  
  
  FBiGeom_getTagHandle( geom, &tag_name[0], &tmp_handle, &err, tag_name.length() );
  CHECK( "ERROR : Couldn't get tag handle." );
  if (tmp_handle != this_tag) {
    std::cerr << "ERROR: getTagHandle didn't return consistent result." << std::endl;
    return false;
  }  

  int tag_size;
  FBiGeom_getTagSizeBytes( geom, this_tag, &tag_size, &err );
  CHECK( "ERROR : Couldn't get tag size." );
  if (tag_size != 4) {
    std::cerr << "ERROR: getTagSizeBytes: expected 4, got " << tag_size << std::endl;
    return false;
  }

  FBiGeom_getTagSizeValues( geom, this_tag, &tag_size, &err );
  CHECK( "ERROR : Couldn't get tag size." );
  if (tag_size != 4) {
    std::cerr << "ERROR: getTagSizeValues: expected 4, got " << tag_size << std::endl;
    return false;
  }

  int tag_type;
  FBiGeom_getTagType( geom, this_tag, &tag_type, &err );
  CHECK( "ERROR : Couldn't get tag type." );
  if (tag_type != iBase_BYTES) {
    std::cerr << "ERROR: getTagType: expected " << iBase_BYTES 
              << ", got " << tag_type << std::endl;
    return false;
  }
  
  FBiGeom_destroyTag( geom, this_tag, true, &err );
  CHECK( "ERROR : Couldn't delete a tag." );

    // print information about all the tags in the model

  std::set<iBase_TagHandle> tags;
  SimpleArray<iBase_EntityHandle> entities;
  FBiGeom_getEntities( geom, root_set, iBase_ALL_TYPES, 
                     ARRAY_INOUT(entities),  &err );
  CHECK( "getEntities( ..., iBase_ALL_TYPES, ... ) failed." );
  for (int i = 0; i < entities.size(); ++i) {
    SimpleArray<iBase_TagHandle> tag_arr;
    FBiGeom_getAllTags( geom, entities[i], ARRAY_INOUT(tag_arr), &err);
    CHECK( "getAllTags failed." );
    std::copy( tag_arr.begin(), tag_arr.end(), std::inserter( tags, tags.begin() ) );
  }
  
  std::cout << "Tags defined on model: ";
  bool first = true;
  for (std::set<iBase_TagHandle>::iterator sit = tags.begin(); sit != tags.end(); ++sit) {
    FBiGeom_getTagName( geom, *sit, name_buffer, &err, sizeof(name_buffer) );
    name_buffer[sizeof(name_buffer)-1] = '\0'; // mnake sure of NUL termination
    CHECK( "getTagName failed." );
    
    if (!first) std::cout << ", ";
    std::cout << name_buffer;
    first = false;
  }
  if (first) std::cout << "<none>";
  std::cout << std::endl;

  return true;
}

bool tag_get_set_test(FBiGeom_Instance geom) 
{
  int err;
  
    // create an arbitrary tag, size 4
  iBase_TagHandle this_tag;
  std::string tag_name("tag_get_set tag");
  FBiGeom_createTag( geom, &tag_name[0], sizeof(int), iBase_BYTES, &this_tag, &err, tag_name.length() );
  CHECK( "ERROR : can not create a tag for get_set test." );
  
  iBase_EntitySetHandle root_set;
  FBiGeom_getRootSet( geom, &root_set, &err );
  CHECK( "ERROR : getRootSet failed!" );
  
    // set this tag to an integer on each entity; keep track of total sum
  int sum = 0, num = 0, dim;
  for (dim = 0; dim <= 3; dim++) {
    SimpleArray<iBase_EntityHandle> gentity_handles;
    FBiGeom_getEntities( geom, root_set, dim, ARRAY_INOUT( gentity_handles ), &err );
    int num_ents = gentity_handles.size();
    std::vector<int> tag_vals( num_ents );
    for (int i = 0; i < num_ents; ++i) {
      tag_vals[i] = num;
      sum += num;
      ++num;
    }
    
    FBiGeom_setArrData( geom, ARRAY_IN( gentity_handles ),
                      this_tag, 
                      (char*)&tag_vals[0], tag_vals.size()*sizeof(int),
                      &err );
    CHECK( "ERROR : can't set tag on entities" );
  }
  
    // check tag values for entities now
  int get_sum = 0;
  for (dim = 0; dim <= 3; dim++) {
    SimpleArray<iBase_EntityHandle> gentity_handles;
    FBiGeom_getEntities( geom, root_set, dim, ARRAY_INOUT( gentity_handles ), &err );
    int num_ents = gentity_handles.size();
    
    SimpleArray<char> tag_vals;
    FBiGeom_getArrData( geom, ARRAY_IN( gentity_handles ), this_tag, 
        (void**)tag_vals.ptr(), &tag_vals.capacity(), &tag_vals.size(), &err );
    CHECK( "ERROR : can't get tag on entities" );
    
    int* tag_ptr = (int*)(&tag_vals[0]);
    for (int i = 0; i < num_ents; ++i)
      get_sum += tag_ptr[i];
  }
  
  if (get_sum != sum) {
    std::cerr << "ERROR: getData didn't return consistent results." << std::endl;
    return false;
  }
  
  FBiGeom_destroyTag( geom, this_tag, true, &err );
  CHECK( "ERROR : couldn't delete tag." );

  return true;
}

/*!
  @test
  TSTT gentity sets test (just implemented parts for now)
  @li Check gentity sets
*/
bool gentityset_test(FBiGeom_Instance geom, bool /*multiset*/, bool /*ordered*/)
{
  int num_type = 4;
  iBase_EntitySetHandle ges_array[4];
  int number_array[4];
  int num_all_gentities_super = 0;
  int ent_type = iBase_VERTEX;

  int err;
  iBase_EntitySetHandle root_set;
  FBiGeom_getRootSet( geom, &root_set, &err );
  CHECK( "ERROR : getRootSet failed!" );
  
    // get the number of sets in the whole model
  int all_sets = 0;
  FBiGeom_getNumEntSets( geom, root_set, 0, &all_sets, &err );
  CHECK( "Problem getting the number of all gentity sets in whole model." );

    // add gentities to entitysets by type
  for (; ent_type < num_type; ent_type++) {
      // initialize the entityset
    FBiGeom_createEntSet( geom, true, &ges_array[ent_type], &err );
    CHECK( "Problem creating entityset." );

      // get entities by type in total "mesh"
    SimpleArray<iBase_EntityHandle> gentities;
    FBiGeom_getEntities( geom, root_set, ent_type, ARRAY_INOUT(gentities), &err );
    CHECK( "Failed to get gentities by type in gentityset_test." );
    
      // add gentities into gentity set
    FBiGeom_addEntArrToSet( geom, ARRAY_IN( gentities ), ges_array[ent_type], &err );
    CHECK( "Failed to add gentities in entityset_test." );
    
      // Check to make sure entity set really has correct number of entities in it
    FBiGeom_getNumOfType( geom, ges_array[ent_type], ent_type, &number_array[ent_type], &err );
    CHECK( "Failed to get number of gentities by type in entityset_test." );

      // compare the number of entities by type
    int num_type_gentity = gentities.size();

    if (number_array[ent_type] != num_type_gentity)
    {
      std::cerr << "Number of gentities by type is not correct"
                << std::endl;
      return false;
    }

      // add to number of all entities in super set
    num_all_gentities_super += num_type_gentity;
  }

    // make a super set having all entitysets
  iBase_EntitySetHandle super_set;
  FBiGeom_createEntSet( geom, true, &super_set, &err );
  CHECK( "Failed to create a super set in gentityset_test." );

  for (int i = 0; i < num_type; i++) {
    FBiGeom_addEntSet( geom, ges_array[i], super_set, &err );
    CHECK( "Failed to create a super set in gentityset_test." );
  }

    //----------TEST BOOLEAN OPERATIONS----------------//

  iBase_EntitySetHandle temp_ges1;
  FBiGeom_createEntSet( geom, true, &temp_ges1, &err );
  CHECK( "Failed to create a super set in gentityset_test." );

    // Subtract
    // add all EDGEs and FACEs to temp_es1
    // get all EDGE entities
  SimpleArray<iBase_EntityHandle> gedges, gfaces, temp_gentities1;
  FBiGeom_getEntities( geom, ges_array[iBase_EDGE], iBase_EDGE, ARRAY_INOUT(gedges), &err );
  CHECK( "Failed to get gedge gentities in gentityset_test." );

    // add EDGEs to ges1
  FBiGeom_addEntArrToSet( geom, ARRAY_IN(gedges), temp_ges1, &err );
  CHECK( "Failed to add gedge gentities in gentityset_test." );

    // get all FACE gentities
  FBiGeom_getEntities( geom, ges_array[iBase_FACE], iBase_FACE, ARRAY_INOUT(gfaces), &err );
  CHECK( "Failed to get gface gentities in gentityset_test." );

    // add FACEs to es1
  FBiGeom_addEntArrToSet( geom, ARRAY_IN(gfaces), temp_ges1, &err );
  CHECK( "Failed to add gface gentities in gentityset_test." );

    // subtract EDGEs
  FBiGeom_subtract( geom, temp_ges1, ges_array[iBase_EDGE], &temp_ges1, &err );
  CHECK( "Failed to subtract gentitysets in gentityset_test." );
  
  FBiGeom_getEntities( geom, temp_ges1, iBase_FACE, ARRAY_INOUT(temp_gentities1), &err );
  CHECK( "Failed to get gface gentities in gentityset_test." );

  if (gfaces.size() != temp_gentities1.size()) {
    std::cerr << "Number of entitysets after subtraction not correct \
             in gentityset_test." << std::endl;
    return false;
  }

    // check there's nothing but gfaces in temp_ges1
  int num_gents;
  FBiGeom_getNumOfType( geom, temp_ges1, iBase_EDGE, &num_gents, &err );
  CHECK( "Failed to get dimensions of gentities in gentityset_test." );
  if (0 != num_gents) {
    std::cerr << "Subtraction failed to remove all edges" << std::endl;
    return false;
  }

    //------------Intersect------------
    //

    // clean out the temp_ges1
  FBiGeom_rmvEntArrFromSet( geom, ARRAY_IN(gfaces), temp_ges1, &err );
  CHECK( "Failed to remove gface gentities in gentityset_test." );

    // check if it is really cleaned out
  FBiGeom_getNumOfType( geom, temp_ges1, iBase_FACE, &num_gents, &err );
  CHECK( "Failed to get number of gentities by type in gentityset_test." );

  if (num_gents != 0) {
    std::cerr << "failed to remove correctly." << std::endl;
    return false;
  }
  
    // add EDGEs to temp ges1
  FBiGeom_addEntArrToSet( geom, ARRAY_IN(gedges), temp_ges1, &err );
  CHECK( "Failed to add gedge gentities in gentityset_test." );

    // add FACEs to temp ges1
  FBiGeom_addEntArrToSet( geom, ARRAY_IN(gfaces), temp_ges1, &err );
  CHECK( "Failed to add gface gentities in gentityset_test." );

    // intersect temp_ges1 with gedges set 
    // temp_ges1 entityset is altered
  FBiGeom_intersect( geom, temp_ges1, ges_array[iBase_EDGE], &temp_ges1, &err );
  CHECK( "Failed to intersect in gentityset_test." );
  
    // try to get FACEs, but there should be nothing but EDGE
  FBiGeom_getNumOfType( geom, temp_ges1, iBase_FACE, &num_gents, &err );
  CHECK( "Failed to get gface gentities in gentityset_test." );

  if (num_gents != 0) {
    std::cerr << "wrong number of gfaces." << std::endl;
    return false;
  }


    //-------------Unite--------------

    // get all regions
  iBase_EntitySetHandle temp_ges2;
  SimpleArray<iBase_EntityHandle> gregions;

  FBiGeom_createEntSet( geom, true, &temp_ges2, &err );
  CHECK( "Failed to create a temp gentityset in gentityset_test." );
  
  FBiGeom_getEntities( geom, ges_array[iBase_REGION], iBase_REGION, ARRAY_INOUT(gregions), &err );
  CHECK( "Failed to get gregion gentities in gentityset_test." );
  
    // add REGIONs to temp es2
  FBiGeom_addEntArrToSet( geom, ARRAY_IN(gregions), temp_ges2, &err );
  CHECK( "Failed to add gregion gentities in gentityset_test." );

    // unite temp_ges1 and temp_ges2
    // temp_ges1 gentityset is altered
  FBiGeom_unite( geom, temp_ges1, temp_ges2, &temp_ges1, &err );
  CHECK( "Failed to unite in gentityset_test." );

    // perform the check
  FBiGeom_getNumOfType( geom, temp_ges1, iBase_REGION, &num_gents, &err );
  CHECK( "Failed to get number of gregion gentities by type in gentityset_test." );
  
  if (num_gents != number_array[iBase_REGION]) {
    std::cerr << "different number of gregions in gentityset_test." << std::endl;
    return false;
  }


    //--------Test parent/child stuff in entiysets-----------

    // Add 2 sets as children to another
  iBase_EntitySetHandle parent_child;
  FBiGeom_createEntSet( geom, true, &parent_child, &err );
  CHECK( "Problem creating gentityset in gentityset_test." );

  FBiGeom_addPrntChld( geom, ges_array[iBase_VERTEX], parent_child, &err );
  CHECK( "Problem add parent in gentityset_test." );

    // check if parent is really added
  SimpleArray<iBase_EntitySetHandle> parents;
  FBiGeom_getPrnts( geom, parent_child, 1, ARRAY_INOUT(parents), &err );
  CHECK( "Problem getting parents in gentityset_test." );

  if (parents.size() != 1) {
    std::cerr << "number of parents is not correct in gentityset_test."
              << std::endl;
    return false;
  }

    // add parent and child
  //sidl::array<void*> parent_child_array = sidl::array<void*>::create1d(1);
  //int num_parent_child_array;
  //sidl::array<void*> temp_gedge_array = sidl::array<void*>::create1d(1);
  //int num_temp_gedge_array;
  //parent_child_array.set(0, parent_child);
  //temp_gedge_array.set(0, ges_array[TSTTG::EntityType_EDGE]);
  FBiGeom_addPrntChld( geom, ges_array[iBase_EDGE], parent_child, &err );
  CHECK( "Problem adding parent and child in gentityset_test." );

  //sidl::array<void*> temp_gface_array = sidl::array<void*>::create1d(1);
  //int num_temp_gface_array;
  //temp_gface_array.set(0, ges_array[TSTTG::EntityType_FACE]);
  FBiGeom_addPrntChld( geom, parent_child, ges_array[iBase_FACE], &err );
  CHECK( "Problem adding parent and child in gentityset_test." );

    // add child
  FBiGeom_addPrntChld( geom, parent_child, ges_array[iBase_REGION], &err );
  CHECK( "Problem adding child in gentityset_test." );

    // get the number of parent gentitysets
  num_gents = -1;
  FBiGeom_getNumPrnt( geom, parent_child, 1, &num_gents, &err );
  CHECK( "Problem getting number of parents in gentityset_test." );

  if (num_gents != 2) {
    std::cerr << "number of parents is not correct in gentityset_test."
              << std::endl;
    return false;
  }

    // get the number of child gentitysets
  num_gents = -1;
  FBiGeom_getNumChld( geom, parent_child, 1, &num_gents, &err );
  CHECK( "Problem getting number of children in gentityset_test." );

  if (num_gents != 2) {
    std::cerr << "number of children is not correct in gentityset_test."
              << std::endl;
    return false;
  }

  SimpleArray<iBase_EntitySetHandle> children;
  FBiGeom_getChldn( geom, parent_child, 1, ARRAY_INOUT(children), &err );
  CHECK( "Problem getting children in gentityset_test." );

  if (children.size() != 2) {
    std::cerr << "number of children is not correct in gentityset_test."
              << std::endl;
    return false;
  }

    // remove children
  FBiGeom_rmvPrntChld( geom, parent_child, ges_array[iBase_FACE], &err );
  CHECK( "Problem removing parent child in gentityset_test." );

    // get the number of child gentitysets
  FBiGeom_getNumChld( geom, parent_child, 1, &num_gents, &err );
  CHECK( "Problem getting number of children in gentityset_test." );

  if (num_gents != 1) {
    std::cerr << "number of children is not correct in gentityset_test."
              << std::endl;
    return false;
  }

    // parent_child and ges_array[TSTTG::EntityType_EDGE] should be related
  int result = 0;
  FBiGeom_isChildOf( geom, ges_array[iBase_EDGE], parent_child, &result, &err );
  CHECK( "Problem checking relation in gentityset_test." );
  if (!result) {
    std::cerr << "parent_child and ges_array[TSTTG::EntityType_EDGE] should be related" << std::endl;
    return false;
  }

    // ges_array[TSTTG::EntityType_FACE] and ges_array[TSTTG::REGION] are not related
  result = 2;
  FBiGeom_isChildOf( geom, ges_array[iBase_FACE], ges_array[iBase_REGION], &result, &err );
  if (result) {
    std::cerr << "ges_array[TSTTG::REGION] and ges_array[TSTTG::EntityType_FACE] should not be related" << std::endl;
    return false;
  }
  

    //--------test modify and query functions-----------------------------
  
    // check the number of gentity sets in whole mesh
  SimpleArray<iBase_EntitySetHandle> gentity_sets;
  FBiGeom_getEntSets( geom, root_set, 1, ARRAY_INOUT( gentity_sets ), &err );
  CHECK( "Problem to get all gentity sets in mesh." );
  
  if (gentity_sets.size() != all_sets + 8) {
    std::cerr << "the number of gentity sets in whole mesh should be 8 times of num_iter."
              << std::endl;
    return false;
  }

    // get all gentity sets in super set
  SimpleArray<iBase_EntitySetHandle> ges_array1;
  FBiGeom_getEntSets( geom, super_set, 1, ARRAY_INOUT( ges_array1 ), &err );
  CHECK( "Problem to get gentity sets in super set." );

    // get the number of gentity sets in super set
  int num_super;
  FBiGeom_getNumEntSets( geom, super_set, 1, &num_super, &err );
  CHECK( "Problem to get the number of all gentity sets in super set." );
  
    // the number of gentity sets in super set should be same
  if (num_super != ges_array1.size()) {
    std::cerr << "the number of gentity sets in super set should be same." << std::endl;
    return false;
  }

    // get all entities in super set
  SimpleArray<iBase_EntitySetHandle> all_gentities;
  FBiGeom_getEntSets( geom, super_set, 1, ARRAY_INOUT( all_gentities ), &err );
  CHECK( "Problem to get all gentities in super set." );
  
    // compare the number of all gentities in super set
  // HJK : num_hops is not implemented
  //if (num_all_gentities_super != ARRAY_SIZE(all_gentities)) {
  //std::cerr << "number of all gentities in super set should be same." << std::endl;
  //success = false;
  //}

    // test add, remove and get all entitiy sets using super set
    // check GetAllGentitysets works recursively and dosen't return
    // multi sets
  for (int k = 0; k < num_super; k++) {
      // add gentity sets of super set to each gentity set of super set
      // make multiple child super sets
    iBase_EntitySetHandle ges_k = ges_array1[k];

    for (int a = 0; a < ges_array1.size(); a++) {
      FBiGeom_addEntSet( geom, ges_array1[a], ges_k, &err );
      CHECK( "Problem to add entity set." );
    }
    
      // add super set to each entity set
    //    sidl::array<GentitysetHandle> superset_array
    //= sidl::array<GentitysetHandle>::create1d(1);
    //superset_array.set(0, super_set);
    //int num_superset_array;
    
    FBiGeom_addEntSet( geom, super_set, ges_k, &err );
    CHECK( "Problem to add super set to gentitysets." );

      // add one gentity sets multiple times
    // HJK: ??? how to deal this case?
    //sidl::array<GentitysetHandle> temp_array1
    //= sidl::array<GentitysetHandle>::create1d(1);
    //int num_temp_array1;
    //temp_array1.set(0, temp_ges1);

    //for (int l = 0; l < 3; l++) {
    FBiGeom_addEntSet( geom, temp_ges1, ges_k, &err );
    CHECK( "Problem to add temp set to gentitysets." );
      //}
  }

  return true;
}
  
/*!
@test
TSTTG topology adjacencies Test
@li Check topology information
@li Check adjacency
*/
// make each topological entity vectors, check their topology
// types, get interior and exterior faces of model
bool topology_adjacencies_test(FBiGeom_Instance geom)
{
  int i, err;
  iBase_EntitySetHandle root_set;
  FBiGeom_getRootSet( geom, &root_set, &err );
  CHECK( "ERROR : getRootSet failed!" );

  int top = iBase_VERTEX;
  int num_test_top = iBase_ALL_TYPES;
  std::vector< std::vector<iBase_EntityHandle> > gentity_vectors(num_test_top);

  // fill the vectors of each topology entities
  // like lines vector, polygon vector, triangle vector,
  // quadrilateral, polyhedrron, tet, hex, prism, pyramid,
  // septahedron vectors
  for (i = top; i < num_test_top; i++) {
    SimpleArray<iBase_EntityHandle> gentities;
    FBiGeom_getEntities( geom, root_set, i, ARRAY_INOUT( gentities ), &err );
    CHECK("Failed to get gentities in adjacencies_test.");
  
    gentity_vectors[i].resize( gentities.size() );
    std::copy( gentities.begin(), gentities.end(), gentity_vectors[i].begin() );
  }

  // check number of entities for each topology
  for (i = top; i < num_test_top; i++) {
    int num_tops = 0;
    FBiGeom_getNumOfType( geom, root_set, i, &num_tops, &err );
    CHECK( "Failed to get number of gentities in adjacencies_test." );
    
    if (static_cast<int>(gentity_vectors[i].size()) != num_tops) {
      std::cerr << "Number of gentities doesn't agree with number returned for dimension " 
                << i << std::endl;
      return false;
    }
  }

  // check adjacencies in both directions
  std::vector<iBase_EntityHandle>::iterator vit;
  for (i = iBase_REGION; i >= iBase_VERTEX; i--) {
    for (vit = gentity_vectors[i].begin(); vit != gentity_vectors[i].end(); vit++) {
      iBase_EntityHandle this_gent = *vit;

        // check downward adjacencies
      for (int j = iBase_VERTEX; j < i; j++) {

        SimpleArray<iBase_EntityHandle> lower_ents;
        FBiGeom_getEntAdj( geom, this_gent, j, ARRAY_INOUT(lower_ents), &err );
        CHECK( "Bi-directional adjacencies test failed." );

          // for each of them, make sure they are adjacent to the upward ones
        int num_lower = lower_ents.size();
        for (int k = 0; k < num_lower; k++) {
          SimpleArray<iBase_EntityHandle> upper_ents;
          FBiGeom_getEntAdj( geom, lower_ents[k], i, ARRAY_INOUT(upper_ents), &err );
          CHECK( "Bi-directional adjacencies test failed." );
          if (std::find(upper_ents.begin(),upper_ents.end(), this_gent) ==
              upper_ents.end()) {
            std::cerr << "Didn't find lower-upper adjacency which was supposed to be there, dims = "
                 << i << ", " << j << std::endl;
            return false;
          }
        }
      }
    }
  }

  return true;
}

/*!
@test
FBiGeom_MOAB topology adjacencies Test
@li Check topology information
@li Check adjacency
*/
// make each topological entity vectors, check their topology
// types, get interior and exterior faces of model
bool geometry_evaluation_test(FBiGeom_Instance geom)
{
  int i, err;
  iBase_EntitySetHandle root_set;
  FBiGeom_getRootSet( geom, &root_set, &err );
  CHECK( "ERROR : getRootSet failed!" );

  int top = iBase_VERTEX;
  int num_test_top = iBase_ALL_TYPES;
  std::vector< std::vector<iBase_EntityHandle> > gentity_vectors(num_test_top);

  // fill the vectors of each topology entities
  // like lines vector, polygon vector, triangle vector,
  // quadrilateral, polyhedrron, tet, hex, prism, pyramid,
  // septahedron vectors
  for (i = top; i < num_test_top; i++) {
    SimpleArray<iBase_EntityHandle> gentities;
    FBiGeom_getEntities( geom, root_set, i, ARRAY_INOUT( gentities ), &err );
    CHECK("Failed to get gentities in adjacencies_test.");
  
    gentity_vectors[i].resize( gentities.size() );
    std::copy( gentities.begin(), gentities.end(), gentity_vectors[i].begin() );
  }
 
  // check adjacencies in both directions
  double min[3], max[3], on[3];
  double near[3] = {.0, .0, .0};
  std::vector<iBase_EntityHandle>::iterator vit;
  for (i = iBase_REGION; i >= iBase_VERTEX; i--) {
    if (i != iBase_EDGE) {
      for (vit = gentity_vectors[i].begin(); vit != gentity_vectors[i].end(); vit++) {
	iBase_EntityHandle this_gent = *vit;
	FBiGeom_getEntBoundBox(geom, this_gent, &min[0], &min[1], &min[2],
			     &max[0], &max[1], &max[2], &err);
	CHECK("Failed to get bounding box of entity.");
	
	FBiGeom_getEntClosestPt(geom, this_gent, near[0], near[1], near[2],
			      &on[0], &on[1], &on[2], &err);
	CHECK("Failed to get closest point on entity.");
      }
    }
  }

  return true;
}

/*!
@test
TSTTG construct Test
@li Check construction of geometry
*/
bool construct_test(FBiGeom_Instance geom)
{
  int err;
  iBase_EntityHandle new_body = 0;

    // construct a cylinder, sweep it about an axis, and delete the result
  iBase_EntityHandle cyl = 0;
  FBiGeom_createCylinder( geom, 1.0, 1.0, 0.0, &cyl, &err );
    // Is the minor radius really supposed to be zero??? - JK
  CHECK( "Creating cylinder failed." );
  
    // move it onto the y axis
  FBiGeom_moveEnt( geom, cyl, 0.0, 1.0, -0.5, &err );
  CHECK( "Problems moving surface." );

      // get the surface with max z
  iBase_EntityHandle max_surf = 0;
  SimpleArray<iBase_EntityHandle> surfs;
  FBiGeom_getEntAdj( geom, cyl, iBase_FACE, ARRAY_INOUT(surfs), &err );
  CHECK( "Problems getting max surf for rotation." );
  
  SimpleArray<double> max_corn, min_corn;
  FBiGeom_getArrBoundBox( geom, ARRAY_IN(surfs), iBase_INTERLEAVED, 
                        ARRAY_INOUT( min_corn ),
                        ARRAY_INOUT( max_corn ),
                        &err );
  CHECK( "Problems getting max surf for rotation." );
  double dtol = 1.0e-6;
  for (int i = 0; i < surfs.size(); ++i) {
    if ((max_corn[3*i+2]) <= dtol && (max_corn[3*i+2]) >= -dtol &&
        (min_corn[3*i+2]) <= dtol && (min_corn[3*i+2]) >= -dtol) {
      max_surf = surfs[i];
      break;
    }
  }  
  
  if (0 == max_surf) {
    std::cerr << "Couldn't find max surf for rotation." << std::endl;
    return false;
  }
  
    // sweep it around the x axis
  FBiGeom_moveEnt( geom, cyl, 0.0, 1.0, 0.0, &err );
  CHECK( "Problems moving surface." );

  FBiGeom_sweepEntAboutAxis( geom, max_surf, 360.0, 1.0, 0.0, 0.0, &new_body, &err );
  CHECK( "Problems sweeping surface about axis." );
  
    // now delete
  FBiGeom_deleteEnt( geom, new_body, &err );
  CHECK( "Problems deleting cylinder or swept surface body." );
  
    // if we got here, we were successful
  return true;
}

static bool compare_box( const double* expected_min,
                         const double* expected_max,
                         const double* actual_min,
                         const double* actual_max )
{
  bool same = true;
  double dtol = 1.0e-6;
  
  for (int i = 0; i < 3; ++i)
  {
    if (expected_min[i] < actual_min[i] - dtol || 
        expected_min[i]*10 >  actual_min[i] ||
        expected_max[i] > actual_max[i] + dtol ||
        expected_max[i]*10 < actual_max[i])  
      same = false;
  } 
  return same;
}

bool primitives_test(FBiGeom_Instance geom) 
{
  int err;
  SimpleArray<iBase_EntityHandle> prims(3);
  iBase_EntityHandle prim;
 
  FBiGeom_createBrick( geom, 1.0, 2.0, 3.0, &prim, &err );
  CHECK( "createBrick failed." );
  prims[0] = prim;
  
  FBiGeom_createCylinder( geom, 1.0, 4.0, 2.0, &prim, &err );
  CHECK( "createCylinder failed." );
  prims[1] = prim;
  
  FBiGeom_createTorus( geom, 2.0, 1.0, &prim, &err );
  CHECK( "createTorus failed." );
  prims[2] = prim;
  
    // verify the bounding boxes for Acis based entities
  SimpleArray<double> max_corn, min_corn;
  FBiGeom_getArrBoundBox( geom, ARRAY_IN(prims), iBase_INTERLEAVED, 
                        ARRAY_INOUT(min_corn), ARRAY_INOUT(max_corn), &err );

  double preset_min_corn[] = 
      // min brick corner xyz
    {-0.5, -1.0, -1.5, 
      // min cyl corner xyz
     -4.0, -2.0, -0.5,
      // min torus corner xyz
     -3.0, -3.0, -1.0
    };
  
  double preset_max_corn[] = 
      // max brick corner xyz
    {0.5, 1.0, 1.5, 
      // max cyl corner xyz
     4.0, 2.0, 0.5,
      // max torus corner xyz
     3.0, 3.0, 1.0
    };
  
  if (!compare_box( preset_min_corn, preset_max_corn,
                    &min_corn[0], &max_corn[0] )) {
    std::cerr << "Box check failed for brick" << std::endl;
    return false;
  }
  
  if (!compare_box( preset_min_corn+3, preset_max_corn+3,
                    &min_corn[3], &max_corn[3] )) {
    std::cerr << "Box check failed for cylinder" << std::endl;
    return false;
  }
  
  if (!compare_box( preset_min_corn+6, preset_max_corn+6,
                    &min_corn[6], &max_corn[6] )) {
    std::cerr << "Box check failed for torus" << std::endl;
    return false;
  }
    // must have worked; delete the entities then return
  for (int i = 0; i < 3; ++i) {
    FBiGeom_deleteEnt( geom, prims[i], &err );
    CHECK( "Problems deleting primitive after boolean check." );
  }
  
  return true;
}
  
bool transforms_test(FBiGeom_Instance geom) 
{
  int err;
  
    // construct a brick
  iBase_EntityHandle brick = 0;
  FBiGeom_createBrick( geom, 1.0, 2.0, 3.0, &brick, &err );
  CHECK( "Problems creating brick for transforms test." );
  
    // move it, then test bounding box
  FBiGeom_moveEnt( geom, brick, 0.5, 1.0, 1.5, &err );
  CHECK( "Problems moving brick for transforms test." );
  
  double bb_min[3], bb_max[3];
  FBiGeom_getEntBoundBox( geom, brick, bb_min, bb_min+1, bb_min+2, bb_max, bb_max+1, bb_max+2, &err );
  CHECK( "Problems getting bounding box after move." );

  double dtol = 1.0e-6;
  if ((bb_min[0]) >= dtol || (bb_min[0]) <= -dtol || 
      (bb_min[1]) >= dtol || (bb_min[1]) <= -dtol || 
      (bb_min[2]) >= dtol || (bb_min[2]) <= -dtol ||
      (bb_max[0]-1) >= dtol || 1-bb_max[0] >=dtol ||
      (bb_max[1]-2) >= dtol || 2 - bb_max[1] >=dtol||
      (bb_max[2]-3) >= dtol || 3 - bb_max[2] >= dtol) {
    std::cerr << "Wrong bounding box after move." << std::endl;
    return false;
  }
  
    // now rotate it about +x, then test bounding box
  FBiGeom_rotateEnt( geom, brick, 90, 1.0, 0.0, 0.0, &err );
  CHECK( "Problems rotating brick for transforms test." );
  
  FBiGeom_getEntBoundBox( geom, brick, bb_min, bb_min+1, bb_min+2, bb_max, bb_max+1, bb_max+2, &err );
  CHECK( "Problems getting bounding box after rotate." );

  if ((bb_min[0]) >= dtol || -bb_min[0] >= dtol ||
      (bb_min[1]+3) >= dtol || -(bb_min[1]+3) >= dtol ||
      (bb_min[2]) >= dtol || -(bb_min[2]) >= dtol ||
      (bb_max[0]-1) >= dtol || 1-bb_max[0] >= dtol ||
      (bb_max[1]) >= dtol || -(bb_max[1]) >= dtol ||
      (bb_max[2]-2) >= dtol || 2-bb_max[2] >=dtol) {
    std::cerr << "Wrong bounding box after rotate." << std::endl;
    return false;
  }
  
    // now reflect through y plane; should recover original bb
  FBiGeom_reflectEnt( geom, brick, 0.0, 1.0, 0.0, &err );
  CHECK( "Problems reflecting brick for transforms test." );
  
  FBiGeom_getEntBoundBox( geom, brick, bb_min, bb_min+1, bb_min+2, bb_max, bb_max+1, bb_max+2, &err );
  CHECK( "Problems getting bounding box after reflect." );
  
  if ((bb_min[0]) >= dtol || -(bb_min[0]) >= dtol ||(bb_min[1]) >= dtol || 
      (bb_min[2]) >= dtol || -(bb_min[1]) >= dtol || -(bb_min[2]) >= dtol ||
      (bb_max[0]-1) >= dtol || 1- bb_max[0] >= dtol ||
      (bb_max[1]-3) >= dtol || 3 - bb_max[1] >= dtol ||
      (bb_max[2]-2) >= dtol || 2 - bb_max[2] >= dtol) {
    std::cerr << "Wrong bounding box after reflect." << std::endl;
    return false;
  }

    // must have worked; delete the entities then return
  FBiGeom_deleteEnt( geom, brick, &err );
  CHECK( "Problems deleting brick after transforms check." );
  return true;
}


bool booleans_test(FBiGeom_Instance geom) 
{
  int err;

    // construct a brick size 1, and a cylinder rad 0.25 height 2
  iBase_EntityHandle brick = 0, cyl = 0;
  FBiGeom_createBrick( geom, 1.0, 0.0, 0.0, &brick, &err );
  CHECK( "Problems creating brick for booleans test." );
  FBiGeom_createCylinder( geom, 1.0, 0.25, 0.0, &cyl, &err );
  CHECK( "Problems creating cylinder for booleans test." );

    // subtract the cylinder from the brick
  iBase_EntityHandle subtract_result = 0;
  FBiGeom_subtractEnts( geom, brick, cyl, &subtract_result, &err );
  CHECK( "Problems subtracting for booleans subtract test." );

    // section the brick
  iBase_EntityHandle section_result = 0;
  FBiGeom_sectionEnt( geom, subtract_result, 1.0, 0.0, 0.0, 0.25, true, &section_result, &err );
  CHECK( "Problems sectioning for booleans section test." );

    // unite the section result with a new cylinder
  FBiGeom_createCylinder( geom, 1.0, 0.25, 0.0, &cyl, &err );
  CHECK( "Problems creating cylinder for unite test." );
  iBase_EntityHandle unite_results;
  iBase_EntityHandle unite_input[] = { section_result, cyl };
  FBiGeom_uniteEnts( geom, unite_input, 2, &unite_results, &err );
  CHECK( "Problems uniting for booleans unite test." );
  
  FBiGeom_deleteEnt( geom, unite_results, &err );
  CHECK( "Problems deleting for booleans unite test." );
  return true;
}

static int get_entities( FBiGeom_Instance geom, int entity_type,
                          std::vector<iBase_EntityHandle>& entities_out,
                          iBase_TagHandle id_tag = 0,
                          std::vector<int>* ids_out = 0 )
{
  int err, num;
  iBase_EntitySetHandle root;
  FBiGeom_getRootSet( geom, &root, &err ); 
  if (iBase_SUCCESS != err)
    return err;
  FBiGeom_getNumOfType( geom, root, entity_type, &num, &err ); 
  if (iBase_SUCCESS != err)
    return err;
  
  entities_out.resize(num);
  int junk1 = entities_out.size(), junk2;
  iBase_EntityHandle* junk_ptr = &entities_out[0];;
  FBiGeom_getEntities( geom, root, entity_type, &junk_ptr, &junk1, &junk2, &err );
  if (iBase_SUCCESS != err)
    return err;
  assert( num == junk1 && num == junk2 );
  
  if (!ids_out)
    return iBase_SUCCESS;
  
  ids_out->resize(num);
  int* int_ptr = &(*ids_out)[0];
  FBiGeom_getIntArrData( geom, &entities_out[0], num, id_tag, &int_ptr, &junk1, &junk2, &err );
  if (iBase_SUCCESS != err)
    return err;
  assert( num == junk1 && num == junk2 );
  
  return iBase_SUCCESS;
}

static int check_firmness( FBiGeom_Instance geom,
                           const std::vector<iBase_EntityHandle>& entities,
                           const std::vector<int>& ids,
                           iBase_TagHandle firmness_tag,
                           const char* expected_value,
                           const char* ent_type_str )
{
  const int firmness_size = 4;
  std::vector<char> firmness(firmness_size * entities.size());

  char* byte_ptr = &firmness[0];
  int err, junk1 = firmness.size(), junk2 = entities.size()*firmness_size;
  FBiGeom_getArrData( geom, &entities[0], entities.size(), firmness_tag, (void**)&byte_ptr, &junk1, &junk2, &err );
  if (iBase_SUCCESS != err)
    return err;
  
  bool all_correct = true;
  for (unsigned i = 0; i < entities.size(); ++i)
    if (std::string(&firmness[firmness_size*i],firmness_size) != expected_value)
      all_correct = false;
  if (!all_correct) {
    std::cout << "ERROR: Expected \"" << expected_value << "\" firmness "
              << "for all " << ent_type_str << "." << std::endl;
    std::cout << "ID  Actual  " << std::endl;
    for (unsigned i = 0; i < entities.size(); ++i)
      std::cout << std::setw(2) << ids[i] << "  "
                << std::string(&firmness[firmness_size*i],firmness_size)
                << std::endl;
    return iBase_FAILURE;
  }
  
  return iBase_SUCCESS;
}

static int count_num_with_tag( FBiGeom_Instance geom,
                               const std::vector<iBase_EntityHandle>& ents,
                               iBase_TagHandle tag )
{
  int err, bytes;
  FBiGeom_getTagSizeBytes( geom, tag, &bytes, &err );
  if (iBase_SUCCESS != err)
    return -1;
  std::vector<char> data(bytes);
  
  int success_count = 0;
  for (size_t i = 0; i < ents.size(); ++i) {
    char* ptr = &data[0];
    int junk1 = bytes, junk2;
    FBiGeom_getData( geom, ents[i], tag, (void**)&ptr, &junk1, &junk2, &err );
    if (iBase_TAG_NOT_FOUND == err)
      continue;
    if (iBase_SUCCESS != err)
      return -1;
    ++success_count;
  }
  
  return success_count;
}
  

bool mesh_size_test(FBiGeom_Instance geom)
{
  const char* filename = STRINGIFY(SRCDIR) "/size.sat";
  int err, junk1, junk2;
  bool result = true;
  
  FBiGeom_deleteAll( geom, &err ); CHECK("");
  FBiGeom_load( geom, filename, 0, &err, strlen(filename), 0 );
  CHECK( "Failed to load input file: 'size.sat'" );
  
    // get tag handles
  iBase_TagHandle interval, size, firmness, id;
  FBiGeom_getTagHandle( geom, "MESH_INTERVAL", &interval, &err, strlen("MESH_INTERVAL") );
  CHECK( "FBiGeom_getTagHandle(\"MESH_INTERVAL\")" );
  FBiGeom_getTagHandle( geom, "MESH_SIZE", &size, &err, strlen("MESH_SIZE") );
  CHECK( "FBiGeom_getTagHandle(\"MESH_SIZE\")" );
  FBiGeom_getTagHandle( geom, "SIZE_FIRMNESS", &firmness, &err, strlen("SIZE_FIRMNESS") );
  CHECK( "FBiGeom_getTagHandle(\"SIZE_FIRMNESS\")" );
  FBiGeom_getTagHandle( geom, "GLOBAL_ID", &id, &err, strlen("GLOBAL_ID") );
  CHECK( "FBiGeom_getTagHandle(\"GLOBAL_ID\")" );
  
    // get entity lists
  std::vector<iBase_EntityHandle> verts, curves, surfs, vols;
  std::vector<int> vert_ids, curve_ids, surf_ids, vol_ids;
  err = get_entities( geom, iBase_VERTEX, verts,  id, &vert_ids  ); CHECK("");
  err = get_entities( geom, iBase_EDGE,   curves, id, &curve_ids ); CHECK("");
  err = get_entities( geom, iBase_FACE,   surfs,  id, &surf_ids  ); CHECK("");
  err = get_entities( geom, iBase_REGION, vols,   id, &vol_ids   ); CHECK("");
  
    // expect interval count to be the same as ID for every curve
  std::vector<int> intervals(curves.size());
  int *int_ptr = &intervals[0];
  junk1 = junk2 = curves.size();
  FBiGeom_getIntArrData( geom, &curves[0], curves.size(), interval, &int_ptr, &junk1, &junk2, &err ); 
  CHECK("Failed to get intervals for curves");
  if (intervals != curve_ids) {
    std::cout << "ERROR: Incorrect curve intervals for one or more curves." << std::endl;
    std::cout << "ID  Expected  Actual" << std::endl;
    for (unsigned i = 0; i < curves.size(); ++i)
      std::cout << std::setw(2) << curve_ids[i] << "  "
                << std::setw(8) << curve_ids[i] << "  "
                << std::setw(6) << intervals[i] << std::endl;
    result = false;
  }
  
    // expect size to be the same as ID for every surface
  std::vector<double> sizes(surfs.size());
  double* dbl_ptr = &sizes[0];
  junk1 = junk2 = surfs.size();
  FBiGeom_getDblArrData( geom, &surfs[0], surfs.size(), size, &dbl_ptr, &junk1, &junk2, &err ); 
  CHECK("Failed to get sizes for surfaces");
  bool all_correct = true;
  for (unsigned i = 0; i < surfs.size(); ++i)
    if (fabs(sizes[i] - (double)surf_ids[i] ) > 1e-8)
      all_correct = false;
  if (!all_correct) {
    std::cout << "ERROR: Incorrect mesh size for one or more surfaces." << std::endl;
    std::cout << "ID  Expected  Actual  " << std::endl;
    for (unsigned i = 0; i < surfs.size(); ++i)
      std::cout << std::setw(2) << surf_ids[i] << "  "
                << std::setw(8) << (double)surf_ids[i] << "  "
                << std::setw(8) << sizes[i] << std::endl;
    result = false;
  }
  
  
  err = result ? iBase_SUCCESS : iBase_FAILURE;
  CHECK("Invalid size or interval data");
    
    // expect "HARD" firmness on all curves
  err = check_firmness( geom, curves, curve_ids, firmness, "HARD", "curves" );
  CHECK("Invalid curve firmness");
    // expect "SOFT" firmness on all surfaces
  err = check_firmness( geom, surfs, surf_ids, firmness, "SOFT", "surfaces" );
  CHECK("Invalid surface firmness");
  
    // expect no firmnes on other entities
  err = count_num_with_tag( geom, verts, firmness ) ? iBase_FAILURE : iBase_SUCCESS;
  CHECK("Got firmness for vertex.");
  err = count_num_with_tag( geom, vols, firmness ) ? iBase_FAILURE : iBase_SUCCESS;
  CHECK("Got firmness for volume.");

    // expect no interval tag on any entities except curves
  err = count_num_with_tag( geom, verts, interval ) ? iBase_FAILURE : iBase_SUCCESS;
  CHECK("Got interval count for vertex.");
  err = count_num_with_tag( geom, vols, interval ) ? iBase_FAILURE : iBase_SUCCESS;
  CHECK("Got interval count for volume.");

    // expect no size tag on any entities except surfaces
    // curves should have size of one of their parent surfaces
  err = count_num_with_tag( geom, verts, size ) ? iBase_FAILURE : iBase_SUCCESS;
  CHECK("Got mesh size for vertex.");
  err = count_num_with_tag( geom, vols, size ) ? iBase_FAILURE : iBase_SUCCESS;
  CHECK("Got mesh size for volume.");

  return true;
}

bool shutdown_test(FBiGeom_Instance geom, std::string &engine_opt) 
{
  int err;

    // test shutdown & startup of interface
  FBiGeom_dtor(geom, &err);
  CHECK( "Interface destruction didn't work properly." );
  
  FBiGeom_newGeom(engine_opt.c_str(), &geom, &err, engine_opt.length());
  CHECK( "Interface re-construction didn't work properly." );
  
  FBiGeom_dtor(geom, &err);
  CHECK( "2nd Interface destruction didn't work properly." );
  
  return true;
}

bool save_entset_test(FBiGeom_Instance geom) 
{
  int err;

#ifdef FORCE_OCC
  std::string filename = "testout.brep";
#elif defined (HAVE_ACIS)
  std::string filename = "testout.sat";
#elif defined (HAVE_OCC)
  std::string filename = "testout.brep";
#else
  std::string filename = "testout.sat";
#endif

    // initialize number of ents and sets to compare with later
  int num_ents_bef, num_sets_bef;
  iBase_EntitySetHandle root;
  FBiGeom_getRootSet( geom, &root, &err ); 
  CHECK("Failed to get root set.");
  FBiGeom_getNumEntSets(geom, root, 1, &num_sets_bef, &err);
  CHECK("Failed to get number of ent sets.");
  FBiGeom_getNumOfType(geom, root, iBase_REGION, &num_ents_bef, &err);
  CHECK("Failed to get number of entities.");

    // create set, and entity to add to set
  iBase_EntityHandle cyl;
  FBiGeom_createCylinder( geom, 1.0, 0.25, 0.0, &cyl, &err );
  CHECK( "Problems creating cylinder for save entset test." );
  iBase_EntitySetHandle seth;
  FBiGeom_createEntSet(geom, true, &seth, &err);
  CHECK( "Problems creating entity set for save entset test." );

    // add the entity
  FBiGeom_addEntToSet(geom, cyl, seth, &err);
  CHECK( "Problems adding entity to set for save entset test." );

    // save/restore the model, and see if the entity is there
  FBiGeom_save(geom, filename.c_str(), NULL, &err, filename.length(), 0);
  CHECK( "Problems saving file for save entset test." );

  FBiGeom_destroyEntSet(geom, seth, &err);
  CHECK("Failed to destroy entity set.");
  FBiGeom_deleteEnt(geom, cyl, &err);
  CHECK("Failed to destroy entity.");
  
    // read the file back in
  FBiGeom_load(geom, filename.c_str(), NULL, &err,
             filename.length(), 0);
  CHECK( "Problems reading file for save entset test." );

    // check number of sets and entities
  int num_ents_aft, num_sets_aft;
  FBiGeom_getNumEntSets(geom, root, 1, &num_sets_aft, &err);
  CHECK("Failed to get number of ent sets.");
  FBiGeom_getNumOfType(geom, root, iBase_REGION, &num_ents_aft, &err);
  CHECK("Failed to get number of entities.");
  bool success = true;
  if (num_ents_aft != 2*num_ents_bef + 1) {
    print_error("Failed to get the right number of entities.",
                iBase_FAILURE, geom, __FILE__, __LINE__);
    success = false;
  }
  else if (num_sets_aft != 2*num_sets_bef + 1) {
    print_error("Failed to get the right number of entity sets.",
                iBase_FAILURE, geom, __FILE__, __LINE__);
    success = false;
  }

    // otherwise, we succeeded
  return success;
}

