// A test file for Subset Normalization
#include "moab/ParallelComm.hpp"
#include "MBParallelConventions.h"
#include "moab/Core.hpp"
#include "moab/FileOptions.hpp"
#include "ReadParallel.hpp"
#include "Coupler.hpp"
#include "DebugOutput.hpp"
#include "ElemUtil.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <cstdlib>
extern "C" 
{
#include "moab/FindPtFuncs.h"
}
#include "moab/TupleList.hpp"
#include "moab/gs.hpp"
#include "moab/Types.hpp"
#ifndef IS_BUILDING_MB
#  define IS_BUILDING_MB
#  include "Internals.hpp"
#  undef IS_BUILDING_MB
#else
#  include "Internals.hpp"
#endif

using namespace moab;

bool debug = true;

// Forward declarations
void get_file_options(int argc, char **argv, 
                      std::vector<const char *> &filenames,
                      std::string &norm_tag,
                      std::vector<const char *> &tag_names,
                      std::vector<const char *> &tag_values,
                      std::string &file_opts,
                      int *err);

void print_tuples(TupleList *tlp);

int print_vertex_fields(Interface* mbi,
                        std::vector< std::vector<EntityHandle> > &groups,
                        Tag                               &norm_hdl,
                        Coupler::IntegType                             integ_type);

double const_field(double x, double y, double z);
double field_1(double x, double y, double z);
double field_2(double x, double y, double z);
double field_3(double x, double y, double z);
double physField(double x, double y, double z);

ErrorCode integrate_scalar_field_test();
int pack_tuples(TupleList *tl, void **ptr);
void unpack_tuples(void *ptr, TupleList** tlp);

//
// Start of main test program
//
int main(int argc, char **argv) {
  // need to init MPI first, to tell how many procs and rank
  // Used since Coupler is a parallel code.  The Coupler may be run
  // in parallel or serial mode but will need MPI either way.
  int err = MPI_Init(&argc, &argv);

  // Print usage if not enough arguments
  if (argc < 3) {
    std::cerr << "Usage: ";
    std::cerr << argv[0] << " <nfiles> <fname1> ... <fnamen> <norm_tag> <tag_select_opts> <file_opts>" << std::endl;
    std::cerr << "nfiles          : number of mesh files" << std::endl;
    std::cerr << "fname1...fnamen : mesh files" << std::endl;
    std::cerr << "norm_tag        : name of tag to normalize across meshes" << std::endl;
    std::cerr << "tag_select_opts : quoted string of tags and values for subset selection, e.g. \"TAG1=VAL1;TAG2=VAL2;TAG3;TAG4\"" << std::endl;
    std::cerr << "file_opts       : quoted string of parallel file read options, e.g. \"OPTION1=VALUE1;OPTION2;OPTION3=VALUE3\"" << std::endl;

    err = integrate_scalar_field_test();
    MB_CHK_SET_ERR( (ErrorCode)err, "Integrate scalar field test failed");

    err = MPI_Finalize();
    
    return err;
  }

  int nprocs, rank;
  err = MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  err = MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Create an ofstream to write output.  One file each for each proc.
  std::stringstream fname;
  fname << argv[0] << rank << ".out";
  if (!std::freopen(fname.str().c_str(), "a", stdout)) return false;
  if (!std::freopen(fname.str().c_str(), "a", stderr)) return false;

  // Create the moab instance
  Interface *mbi = new Core();
  if (NULL == mbi) {
    std::cerr << "MOAB constructor failed" << std::endl;
    return 1;
  }

  // Get the input options
  std::cout << "Getting options..." << std::endl;
  std::vector<const char *> filenames;
  std::vector<const char *> tagNames;
  std::vector<const char *> tagValues;
  std::string normTag, fileOpts;
  get_file_options(argc, argv, filenames, normTag, tagNames, tagValues, fileOpts, &err);
  MB_CHK_SET_ERR( (ErrorCode)err, "get_file_options failed");

  // Print out the input parameters
  std::cout << "    Input Parameters - " << std::endl;
  std::cout << "      Filenames: ";
  for (std::vector<const char *>::iterator it = filenames.begin(); it != filenames.end(); it++)
    std::cout << *it << " ";
  std::cout << std::endl;
  std::cout << "      Norm Tag: " << normTag << std::endl;
  std::cout << "      Selection Data: NumNames=" << tagNames.size() << " NumValues=" << tagValues.size() << std::endl;
  std::cout << "                      TagNames             TagValues           " << std::endl;
  std::cout << "                      -------------------- --------------------" << std::endl;
  std::vector<const char *>::iterator nameIt = tagNames.begin();
  std::vector<const char *>::iterator valIt = tagValues.begin();
  std::cout << std::setiosflags(std::ios::left);
  for (; nameIt != tagNames.end(); nameIt++) {
    std::cout << "                      " << std::setw(20) << *nameIt;
    if (*valIt != 0) {
      std::cout << " " << std::setw(20) << *((int*)(*valIt)) << std::endl;
      valIt++;
    }
    else
      std::cout << " NULL                " << std::endl;
  }
  std::cout << std::resetiosflags(std::ios::left);
  std::cout << "      File Options: " << fileOpts << std::endl;

  // Read in mesh(es)
  std::cout << "Reading mesh file(s)..." << std::endl;
  std::vector<ParallelComm *> pcs(filenames.size()); 
  std::vector<ReadParallel *> rps(filenames.size()); 

  // allocate root sets for each mesh for moab
  std::vector<EntityHandle> roots(filenames.size());

  ErrorCode result;
  for (unsigned int i = 0; i < filenames.size(); i++) {
    pcs[i] = new ParallelComm(mbi, MPI_COMM_WORLD);
    rps[i] = new ReadParallel(mbi, pcs[i]);
    
    result = mbi->create_meshset(MESHSET_SET, roots[i]);

    MB_CHK_SET_ERR(result, "Creating root set failed");
    result = rps[i]->load_file(filenames[i], &roots[i], FileOptions(fileOpts.c_str()));
    MB_CHK_SET_ERR(result, "load_file failed");
  }

  // Initialize the debug object for Range printing
  DebugOutput debugOut("ssn_test-", std::cerr);
  debugOut.set_rank(rank);
  debugOut.set_verbosity(10);

  // Output what is in root sets
  for (unsigned int k = 0; k < filenames.size(); k++) {

    Range rootRg;
    result = mbi->get_entities_by_handle(roots[k], rootRg);
    MB_CHK_SET_ERR(result, "can't get entities");
    debugOut.print(2, "Root set entities: ", rootRg);
    rootRg.clear();

    Range partRg;
    pcs[k]->get_part_entities(partRg);
    debugOut.print(2, "Partition entities: ", partRg);
    partRg.clear();
  }

  // source is 1st mesh, target is 2nd mesh
  Range src_elems, targ_elems;

  // ******************************
  std::cout << "********** Create Coupler **********" << std::endl;
  // Create a coupler
  std::cout << "Creating Coupler..." << std::endl;
  Coupler mbc(mbi, pcs[0], src_elems, 0);

  // Get tag handles for passed in tags
  std::cout << "Getting tag handles..." << std::endl;
  int numTagNames = tagNames.size();

  std::vector<Tag> tagHandles(numTagNames);
  int iTags = 0;
  while (iTags < numTagNames) {
    std::cout << "Getting handle for " << tagNames[iTags] << std::endl;
    result = mbi->tag_get_handle( tagNames[iTags],tagHandles[iTags]);
    MB_CHK_SET_ERR(result, "Retrieving tag handles failed");
    iTags++;
  }

  // ******************************
  std::cout << "********** Test create_tuples **********" << std::endl;
  // First get some EntitySets for Mesh 1 and Mesh 2
  {

    Range entsets1, entsets2;
    result = mbi->get_entities_by_type_and_tag(roots[0], MBENTITYSET, &tagHandles[0],
        (const void* const*)&tagValues[0], tagHandles.size(), entsets1, Interface::INTERSECT); // recursive is false
    MB_CHK_SET_ERR(result, "sets: get_entities_by_type_and_tag failed on Mesh 1.");

    // Create tuple_list for each mesh's
    std::cout << "Creating tuples for mesh 1..." << std::endl;
    TupleList *m1TagTuples = NULL;
    err = mbc.create_tuples(entsets1, &tagHandles[0], tagHandles.size(), &m1TagTuples);
    MB_CHK_SET_ERR((ErrorCode)err, "create_tuples failed");

    std::cout << "   create_tuples returned" << std::endl;
    print_tuples(m1TagTuples);

    result = mbi->get_entities_by_type_and_tag(roots[1], MBENTITYSET, &tagHandles[0],
            (const void* const*)&tagValues[0], tagHandles.size(), entsets2, Interface::INTERSECT); // recursive is false
    MB_CHK_SET_ERR(result, "sets: get_entities_by_type_and_tag failed on Mesh 2.");

    std::cout << "Creating tuples for mesh 2..." << std::endl;
    TupleList *m2TagTuples = NULL;
    err = mbc.create_tuples(entsets2,
                            (Tag*)(&tagHandles[0]), tagHandles.size(), &m2TagTuples);
    MB_CHK_SET_ERR((ErrorCode)err, "create_tuples failed");

    std::cout << "   create_tuples returned" << std::endl;
    print_tuples(m2TagTuples);

    // ******************************
    std::cout << "********** Test consolidate_tuples **********" << std::endl;
    // In this serial version we only have the tuples from Mesh 1 and Mesh 2.
    // Just consolidate those for the test.
    std::cout << "Consolidating tuple_lists for Mesh 1 and Mesh 2..." << std::endl;
    TupleList **tplp_arr = (TupleList**) malloc(2*sizeof(TupleList*));
    TupleList *unique_tpl = NULL;
    tplp_arr[0] = m1TagTuples;
    tplp_arr[1] = m2TagTuples;

    err = mbc.consolidate_tuples(tplp_arr, 2, &unique_tpl);
    MB_CHK_SET_ERR((ErrorCode)err,"consolidate_tuples failed");
    std::cout << "    consolidate_tuples returned" << std::endl;
    print_tuples(unique_tpl);

  }

  // ******************************
  std::cout << "********** Test get_matching_entities **********" << std::endl;
  std::vector< std::vector<EntityHandle> > m1EntitySets;
  std::vector< std::vector<EntityHandle> > m1EntityGroups;
  std::vector< std::vector<EntityHandle> > m2EntitySets;
  std::vector< std::vector<EntityHandle> > m2EntityGroups;

  // Get matching entities for Mesh 1
  std::cout << "Get matching entities for mesh 1..." << std::endl;
  err = mbc.get_matching_entities( roots[0], &tagHandles[0], &tagValues[0], tagHandles.size(),
                                    &m1EntitySets, &m1EntityGroups);
  MB_CHK_SET_ERR((ErrorCode)err, "get_matching_entities failed");

  std::cout << "    get_matching_entities returned " << m1EntityGroups.size() << " entity groups" << std::endl;
  
  // Print out the data in the vector of vectors
  std::vector< std::vector<EntityHandle> >::iterator iter_esi;
  std::vector< std::vector<EntityHandle> >::iterator iter_egi;
  std::vector<EntityHandle>::iterator iter_esj;
  std::vector<EntityHandle>::iterator iter_egj;
  Range entSetRg;
  int icnt;
  for (iter_egi = m1EntityGroups.begin(), iter_esi = m1EntitySets.begin(), icnt = 1; 
       (iter_egi != m1EntityGroups.end()) && (iter_esi != m1EntitySets.end()); 
       iter_egi++, iter_esi++, icnt++) {
    std::cout << "      EntityGroup(" << icnt << ") = ";
    std::cout.flush();
    entSetRg.clear();
    for (iter_egj = (*iter_egi).begin(); iter_egj != (*iter_egi).end(); iter_egj++)
      entSetRg.insert((EntityHandle) *iter_egj);
    debugOut.print(2, "Mesh1 matching Entities: ", entSetRg);
    std::cout.flush();

    std::cout << "      EntitySet(" << icnt << ") = ";
    std::cout.flush();
    entSetRg.clear();
    for (iter_esj = (*iter_esi).begin(); iter_esj != (*iter_esi).end(); iter_esj++)
      entSetRg.insert((EntityHandle) *iter_esj);
    debugOut.print(2, "Mesh1 matching EntitySets: ", entSetRg);
    std::cout.flush();
  }

  // Get matching entities for Mesh 2
  std::cout << "Get matching entities for mesh 2..." << std::endl;
  err = mbc.get_matching_entities(roots[1], &tagHandles[0], &tagValues[0], tagHandles.size(),
                                  &m2EntitySets, &m2EntityGroups);
  MB_CHK_SET_ERR((ErrorCode)err, "get_matching_entities failed");

  std::cout << "    get_matching_entities returned " << m2EntityGroups.size() << " entity groups" << std::endl;
  for (iter_egi = m2EntityGroups.begin(), iter_esi = m2EntitySets.begin(), icnt = 1; 
       (iter_egi != m2EntityGroups.end()) && (iter_esi != m2EntitySets.end()); 
       iter_egi++, iter_esi++, icnt++) {
    std::cout << "      EntityGroup(" << icnt << ") = ";
    std::cout.flush();
    entSetRg.clear();
    for (iter_egj = (*iter_egi).begin(); iter_egj != (*iter_egi).end(); iter_egj++)
      entSetRg.insert((EntityHandle) *iter_egj);
    debugOut.print(2, "Mesh2 matching Entities: ", entSetRg);
    std::cout.flush();

    std::cout << "      EntitySet(" << icnt << ") = ";
    std::cout.flush();
    entSetRg.clear();
    for (iter_esj = (*iter_esi).begin(); iter_esj != (*iter_esi).end(); iter_esj++)
      entSetRg.insert((EntityHandle) *iter_esj);
    debugOut.print(2, "Mesh2 matching EntitySets: ", entSetRg);
    std::cout.flush();
  }

  if (debug) {
    // ******************************
    std::cout << "********** Test print_tuples **********" << std::endl;
    // temporary test funtion
    std::cout << "Testing print_tuples..." << std::endl;

    TupleList test_tuple;
    int num_ints=3, num_longs=2, num_ulongs=4, num_reals=6, num_rows=10;

    std::cout << "    print of test_tuples zero init..." << std::endl;
    test_tuple.initialize(0, 0, 0, 0, 0);

    test_tuple.enableWriteAccess();

    print_tuples(&test_tuple);

    std::cout << "    print of test_tuples after setting n to 10..." << std::endl;
    test_tuple.set_n( 10 );
    print_tuples(&test_tuple);

    test_tuple.initialize(num_ints, num_longs, num_ulongs, num_reals, num_rows);
    std::cout << "    print of test_tuples after init..." << std::endl;
    print_tuples(&test_tuple);

    std::cout << "    print of test_tuples after setting n to 10..." << std::endl;
    test_tuple.set_n( 10 );
    print_tuples(&test_tuple);

    
    for (int i = 0; i < num_rows; i++) {
      int j;
      for (j = 0; j < num_ints; j++)
        test_tuple.vi_wr[i*num_ints + j] = (int) ((j+1)*(i+1));

      for (j = 0; j < num_longs; j++)
        test_tuple.vl_wr[i*num_longs + j] = (int) ((j+1)*(i+1));

      for (j = 0; j < num_ulongs; j++)
        test_tuple.vul_wr[i*num_ulongs + j] = (int) ((j+1)*(i+1));

      for (j = 0; j < num_reals; j++)
        test_tuple.vr_wr[i*num_reals + j] = (int) ((j+1)*(i+1)+(j*0.01));
    }
    std::cout << "    print of test_tuples after filling with data..." << std::endl;
    print_tuples(&test_tuple);

    // ******************************
    std::cout << "********** Test pack_tuples and unpack_tuples **********" << std::endl;
    void *mp_buf;
    int buf_sz;
    if (rank == 0) {
      buf_sz = pack_tuples(&test_tuple, &mp_buf);
    }

    // Send buffer size
    err = MPI_Bcast(&buf_sz, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (err != MPI_SUCCESS) {
      std::cerr << "MPI_Bcast of buffer size failed" << std::endl;
      return -1;
    }

    // Allocate a buffer in the other procs
    if (rank != 0) {
      mp_buf = malloc(buf_sz*sizeof(uint));
    }

    err = MPI_Bcast(mp_buf, buf_sz*sizeof(uint), MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    if (err != MPI_SUCCESS) {
      std::cerr << "MPI_Bcast of buffer failed" << std::endl;
      return -1;
    }

    TupleList *rcv_tuples;
    unpack_tuples(mp_buf, &rcv_tuples);

    std::cout << "    print of rcv_tuples after unpacking from MPI_Bcast..." << std::endl;
    print_tuples(rcv_tuples);
  }

  err = integrate_scalar_field_test();
  MB_CHK_SET_ERR((ErrorCode)err, "Failure in integrating a scalar_field");

  // ******************************
  std::cout << "********** Test get_group_integ_vals **********" << std::endl;
  std::cout << "Get group integrated field values..." << std::endl;

  // print the field values at the vertices before change.
  std::cout << "    print vertex field values first:" << std::endl;
  Tag norm_hdl;
  result = mbi->tag_get_handle( normTag.c_str(),norm_hdl);
  MB_CHK_SET_ERR((ErrorCode)err, "Failed to get tag handle.");

  Coupler::IntegType integ_type = Coupler::VOLUME;
  // Mesh 1 field values
  std::cout << "  Original entity vertex field values (mesh 1): " << std::endl;
  print_vertex_fields(mbi, m1EntityGroups, norm_hdl, integ_type);

  // Mesh 2 field values
  std::cout << "  Original entity vertex field values (mesh 2): " << std::endl;
  print_vertex_fields(mbi, m2EntityGroups, norm_hdl, integ_type);

  // Get the field values
  std::vector<double>::iterator iter_ivals;

  std::cout << "Get group integrated field values for mesh 1..." << std::endl;
  std::vector<double> m1IntegVals(m1EntityGroups.size());
  err = mbc.get_group_integ_vals(m1EntityGroups, m1IntegVals, normTag.c_str(), 4, integ_type);
  MB_CHK_SET_ERR((ErrorCode)err, "Failed to get the Mesh 1 group integration values.");
  std::cout << "Mesh 1 integrated field values(" << m1IntegVals.size() << "): ";
  for (iter_ivals = m1IntegVals.begin(); iter_ivals != m1IntegVals.end(); iter_ivals++) {
    std::cout << (*iter_ivals) << " ";
  }
  std::cout << std::endl;

  std::cout << "Get group integrated field values for mesh 2..." << std::endl;
  std::vector<double> m2IntegVals(m2EntityGroups.size());
  err = mbc.get_group_integ_vals(m2EntityGroups, m2IntegVals, normTag.c_str(), 4, integ_type);
  MB_CHK_SET_ERR((ErrorCode)err, "Failed to get the Mesh 2 group integration values.");
  std::cout << "Mesh 2 integrated field values(" << m2IntegVals.size() << "): ";
  for (iter_ivals = m2IntegVals.begin(); iter_ivals != m2IntegVals.end(); iter_ivals++) {
    std::cout << (*iter_ivals) << " ";
  }
  std::cout << std::endl;

  // ******************************
  std::cout << "********** Test apply_group_norm_factors **********" << std::endl;
  // Make the norm factors by inverting the integration values.
  double val;
  for (unsigned int i = 0; i < m1IntegVals.size(); i++) {
    val = m1IntegVals[i];
    m1IntegVals[i] = 1/val;
  }

  for (unsigned int i = 0; i < m2IntegVals.size(); i++) {
    val = m2IntegVals[i];
    m2IntegVals[i] = 1/val;
  }

  std::cout << "Mesh 1 norm factors(" << m1IntegVals.size() << "): ";
  for (iter_ivals = m1IntegVals.begin(); iter_ivals != m1IntegVals.end(); iter_ivals++) {
    std::cout << (*iter_ivals) << " ";
  }
  std::cout << std::endl;

  std::cout << "Mesh 2 norm factors(" << m2IntegVals.size() << "): ";
  for (iter_ivals = m2IntegVals.begin(); iter_ivals != m2IntegVals.end(); iter_ivals++) {
    std::cout << (*iter_ivals) << " ";
  }
  std::cout << std::endl;

  // Apply the factors and reprint the vertices
  err = mbc.apply_group_norm_factor(m1EntitySets, m1IntegVals, normTag.c_str(), integ_type);
  MB_CHK_SET_ERR((ErrorCode)err, "Failed to apply norm factors to Mesh 1.");

  err = mbc.apply_group_norm_factor(m2EntitySets, m2IntegVals, normTag.c_str(), integ_type);
  MB_CHK_SET_ERR((ErrorCode)err, "Failed to apply norm factors to Mesh 2.");

  // Get the norm_tag_factor on the EntitySets
  // Get the handle for the norm factor tag
  Tag norm_factor_hdl;
  std::string normFactor = normTag + "_normf";
  result = mbi->tag_get_handle( normFactor.c_str(), norm_factor_hdl);
  MB_CHK_SET_ERR( result, "Failed to get norm factor tag handle.");
  
  // Mesh 1 values
  std::cout << "Mesh 1 norm factors per EntitySet...";
  for (iter_esi = m1EntitySets.begin(); iter_esi != m1EntitySets.end(); iter_esi++) {
    for (iter_esj = (*iter_esi).begin(); iter_esj != (*iter_esi).end(); iter_esj++) {
      double data = 0;
      EntityHandle eh=* iter_esj;
      result = mbi->tag_get_data(norm_factor_hdl, &eh, 1, &data);
      MB_CHK_SET_ERR( result, "Failed to get tag data.");
      std::cout << data << ", ";
    }
  }
  std::cout << std::endl;

  // Mesh 2 values
  std::cout << "Mesh 2 norm factors per EntitySet...";
  for (iter_esi = m2EntitySets.begin(); iter_esi != m2EntitySets.end(); iter_esi++) {
    for (iter_esj = (*iter_esi).begin(); iter_esj != (*iter_esi).end(); iter_esj++) {
      double data = 0;
      EntityHandle eh = *iter_esj;
      result = mbi->tag_get_data(norm_factor_hdl, &eh, 1, &data);
      MB_CHK_SET_ERR( result, "Failed to get tag data.");
      std::cout << data << ", ";
    }
  }
  std::cout << std::endl;
  
  // ******************************
  std::cout << "********** Test normalize_subset **********" << std::endl;
  // Now call the Coupler::normalize_subset routine and see if we get an error.
  std::cout << "Running Coupler::normalize_subset() on mesh 1" << std::endl;
  err = mbc.normalize_subset((EntityHandle)roots[0], 
                             normTag.c_str(), 
                             &tagNames[0], 
                             numTagNames, 
                             &tagValues[0], 
                             Coupler::VOLUME, 
                             4);
  MB_CHK_SET_ERR((ErrorCode)err, "Failure in call to Coupler::normalize_subset() on mesh 1");

  // Print the normFactor on each EntitySet after the above call.
  // Mesh 1 values
  std::cout << "Mesh 1 norm factors per EntitySet...";
  for (iter_esi = m1EntitySets.begin(); iter_esi != m1EntitySets.end(); iter_esi++) {
    for (iter_esj = (*iter_esi).begin(); iter_esj != (*iter_esi).end(); iter_esj++) {
      double data = 0;
      EntityHandle eh = *iter_esj;
      result = mbi->tag_get_data(norm_factor_hdl, &eh, 1, &data);
      MB_CHK_SET_ERR( result, "Failed to get tag data.");
      std::cout << data << ", ";
    }
  }
  std::cout << std::endl;

  std::cout << "Running Coupler::normalize_subset() on mesh 2" << std::endl;
  err = mbc.normalize_subset((EntityHandle)roots[1], 
                             normTag.c_str(), 
                             &tagNames[0], 
                             numTagNames, 
                             &tagValues[0], 
                             Coupler::VOLUME, 
                             4);
  MB_CHK_SET_ERR((ErrorCode)err, "Failure in call to Coupler::normalize_subset() on mesh 2");

  // Mesh 2 values
  std::cout << "Mesh 2 norm factors per EntitySet...";
  for (iter_esi = m2EntitySets.begin(); iter_esi != m2EntitySets.end(); iter_esi++) {
    for (iter_esj = (*iter_esi).begin(); iter_esj != (*iter_esi).end(); iter_esj++) {
      double data = 0;
      EntityHandle eh = *iter_esj;
      result = mbi->tag_get_data(norm_factor_hdl, &eh, 1, &data);
      MB_CHK_SET_ERR( result, "Failed to get tag data.");

      std::cout << data << ", ";
    }
  }
  std::cout << std::endl;

  // Done, cleanup
  std::cout << "********** ssn_test DONE! **********" << std::endl;
  MPI_Finalize();
  return 0;
}

ErrorCode integrate_scalar_field_test()
{
  // ******************************
  std::cout << "********** Test moab::Element::Map::integrate_scalar_field **********" << std::endl;
  // Create a simple hex centered at 0,0,0 with sides of length 2.
  std::vector<CartVect> biunit_cube(8);
  biunit_cube[0] = CartVect( -1, -1, -1 );
  biunit_cube[1] = CartVect(  1, -1, -1 );
  biunit_cube[2] = CartVect(  1,  1, -1 );
  biunit_cube[3] = CartVect( -1,  1, -1 );
  biunit_cube[4] = CartVect( -1, -1,  1 );
  biunit_cube[5] = CartVect(  1, -1,  1 );
  biunit_cube[6] = CartVect(  1,  1,  1 );
  biunit_cube[7] = CartVect( -1,  1,  1 );

  std::vector<CartVect> zerobase_cube(8);
  zerobase_cube[0] = CartVect( 0, 0, 0 );
  zerobase_cube[1] = CartVect( 2, 0, 0 );
  zerobase_cube[2] = CartVect( 2, 2, 0 );
  zerobase_cube[3] = CartVect( 0, 2, 0 );
  zerobase_cube[4] = CartVect( 0, 0, 2 );
  zerobase_cube[5] = CartVect( 2, 0, 2 );
  zerobase_cube[6] = CartVect( 2, 2, 2 );
  zerobase_cube[7] = CartVect( 0, 2, 2 );

  // Calculate field values at the corners of both cubes
  double bcf[8], bf1[8], bf2[8], bf3[8], zcf[8], zf1[8], zf2[8], zf3[8];
  for (int i = 0; i < 8; i++) {
    bcf[i] = const_field(biunit_cube[i][0], biunit_cube[i][1], biunit_cube[i][2]);
    bf1[i] = field_1(biunit_cube[i][0], biunit_cube[i][1], biunit_cube[i][2]);
    bf2[i] = field_2(biunit_cube[i][0], biunit_cube[i][1], biunit_cube[i][2]);
    bf3[i] = field_3(biunit_cube[i][0], biunit_cube[i][1], biunit_cube[i][2]);

    zcf[i] = const_field(zerobase_cube[i][0], zerobase_cube[i][1], zerobase_cube[i][2]);
    zf1[i] = field_1(zerobase_cube[i][0], zerobase_cube[i][1], zerobase_cube[i][2]);
    zf2[i] = field_2(zerobase_cube[i][0], zerobase_cube[i][1], zerobase_cube[i][2]);
    zf3[i] = field_3(zerobase_cube[i][0], zerobase_cube[i][1], zerobase_cube[i][2]);
  }

  std::cout << "Integrated values:" << std::endl;

  try {
    double field_const1, field_const2;
    double field_linear1, field_linear2;
    double field_quad1, field_quad2;
    double field_cubic1, field_cubic2;

    int ipoints=0;
    Element::LinearHex biunit_hexMap(biunit_cube);
    Element::LinearHex zerobase_hexMap(zerobase_cube);

    field_const1 = biunit_hexMap.integrate_scalar_field(bcf);
    field_const2 = zerobase_hexMap.integrate_scalar_field(zcf);
    std::cout << "    binunit_cube, const_field(num_pts=" << ipoints << "): field_val=" << field_const1 << std::endl;
    std::cout << "    zerobase_cube, const_field(num_pts=" << ipoints << "): field_val=" << field_const2 << std::endl;

    field_linear1 = biunit_hexMap.integrate_scalar_field(bf1);
    field_linear2 = zerobase_hexMap.integrate_scalar_field(zf1);
    std::cout << "    binunit_cube, field_1(num_pts=" << ipoints << "): field_val=" << field_linear1 << std::endl;
    std::cout << "    zerobase_cube, field_1(num_pts=" << ipoints << "): field_val=" << field_linear2 << std::endl;

    field_quad1 = biunit_hexMap.integrate_scalar_field(bf2);
    field_quad2 = zerobase_hexMap.integrate_scalar_field(zf2);
    std::cout << "    binunit_cube, field_2(num_pts=" << ipoints << "): field_val=" << field_quad1 << std::endl;
    std::cout << "    zerobase_cube, field_2(num_pts=" << ipoints << "): field_val=" << field_quad2 << std::endl;

    field_cubic1 = biunit_hexMap.integrate_scalar_field(bf3);
    field_cubic2 = zerobase_hexMap.integrate_scalar_field(zf3);
    std::cout << "    binunit_cube, field_3(num_pts=" << ipoints << "): field_val=" << field_cubic1 << std::endl;
    std::cout << "    zerobase_cube, field_3(num_pts=" << ipoints << "): field_val=" << field_cubic2 << std::endl;
  }
  catch (moab::Element::Map::ArgError) {
    MB_CHK_SET_ERR(MB_FAILURE, "Failed to set vertices on Element::Map.");
  }
  catch (moab::Element::Map::EvaluationError) {
    MB_CHK_SET_ERR(MB_FAILURE, "Failed to get inverse evaluation of coordinate on Element::Map.");
  }
  return MB_SUCCESS;
}

// Function to parse input parameters
void get_file_options(int argc, char **argv, 
                      std::vector<const char *> &filenames,
                      std::string &normTag,
                      std::vector<const char *> &tagNames,
                      std::vector<const char *> &tagValues,
                      std::string &fileOpts,
                      int *err)
{
  int npos = 1;

  // get number of files
  int nfiles = atoi(argv[npos++]);
  
  // get mesh filenames
  filenames.resize(nfiles);
  for (int i = 0; i < nfiles; i++) filenames[i] = argv[npos++];

  // get normTag
  if (npos < argc) 
    normTag = argv[npos++];
  else {
    std::cerr << "Insufficient parameters:  norm_tag missing" << std::endl;
    *err = 1;
    return;
  }

  // get tag selection options
  if (npos < argc) {
    char* opts = argv[npos++];
    //char sep1[1] = {';'};
    //char sep2[1] = {'='};
    bool end_vals_seen = false;
    std::vector<char *> tmpTagOpts;

    // first get the options
    for (char* i = strtok(opts, ";"); i; i = strtok(0, ";")) {
      if (debug) std::cout << "get_file_options:  i=" << i << std::endl;
      tmpTagOpts.push_back(i);
    }

    // parse out the name and val or just name.
    for (unsigned int j = 0; j < tmpTagOpts.size(); j++) {
      char* e = strtok(tmpTagOpts[j], "=");
      if (debug) std::cout << "get_file_options:    name=" << e << std::endl;
      tagNames.push_back(e);
      e = strtok(0, "=");
      if (e != NULL) {
        if (debug) std::cout << "get_file_options:     val=" << e << std::endl;
        // We have a value
        if (end_vals_seen) {
          // ERROR we should not have a value after none are seen
          std::cerr << "Incorrect parameters:  new value seen after end of values" << std::endl;
          *err = 1;
          return;
        }
        // Otherwise get the value string from e and convert it to an int
        int *valp = new int;
        *valp = atoi(e);
        tagValues.push_back((const char *) valp);
      }
      else {
        // Otherwise there is no '=' so push a null on the list
        end_vals_seen = true;
        tagValues.push_back((const char *) 0);
      }
    }
  }
  else {
    std::cerr << "Insufficient parameters:  tag_select_opts missing" << std::endl;
    *err = 1;
    return;
  }

  // get fileOpts
  if (npos < argc) 
    fileOpts = argv[npos++];
  else {
    std::cerr << "Insufficient parameters:  file_opts missing" << std::endl;
    *err = 1;
    return;
  }
}

// Function to print out a tuple_list.
void print_tuples(TupleList *tlp)
{
  uint mi, ml, mul, mr;
  tlp->getTupleSize(mi, ml, mul, mr);
  std::cout << "    tuple data:  (n=" << tlp->get_n() << ")" << std::endl;
  std::cout << "      mi:" << mi
            << " ml:" << ml
            << " mul:" << mul
            << " mr:" << mr << std::endl;
  std::cout << "      ["
            << std::setw(11*mi)  << " int data"   << " |"
            << std::setw(11*ml)  << " long data"  << " |"
            << std::setw(11*mul) << " ulong data" << " |"
            << std::setw(11*mr)  << " real data"  << " "
            << std::endl << "        ";
  for (unsigned int i = 0; i < tlp->get_n(); i++) {
    if (mi >0) {
      for (unsigned int j = 0; j < mi; j++) {
        std::cout << std::setw(10) << tlp->vi_rd[i*mi + j] << " ";
      }
    }
    else {
      std::cout << "         ";
    }
    std::cout << "| ";

    if (ml >0) {
      for (unsigned int j = 0; j < ml; j++) {
        std::cout << std::setw(10) << tlp->vl_rd[i*ml + j] << " ";
      }
    }
    else {
      std::cout << "          ";
    }
    std::cout << "| ";

    if (mul >0) {
      for (unsigned int j = 0; j < mul; j++) {
        std::cout << std::setw(10) << tlp->vul_rd[i*mul + j] << " ";
      }
    }
    else {
      std::cout << "           ";
    }
    std::cout << "| ";

    if (mr >0) {
      for (unsigned int j = 0; j < mr; j++) {
        std::cout << std::setw(10) << tlp->vr_rd[i*mr + j] << " ";
      }
    }
    else {
      std::cout << "          ";
    }

    if (i+1 < tlp->get_n())
      std::cout << std::endl << "        ";
  }
  std::cout << "]" << std::endl;
}

// Function to print vertex field values
int print_vertex_fields(Interface* mbi,
                        std::vector< std::vector<EntityHandle> > &groups,
                        Tag                           &norm_hdl,
                        Coupler::IntegType                             integ_type)
{
  int err = 0;
  ErrorCode result;
  std::vector<EntityHandle>::iterator iter_j;

  for (unsigned int i = 0; i < groups.size(); i++) {
    std::cout << "    Group - " << std::endl << "        ";
    for (iter_j = groups[i].begin(); iter_j != groups[i].end(); iter_j++) {
      EntityHandle ehandle = (*iter_j);
      // Check that the entity in iter_j is of the same dimension as the 
      // integ_type we are performing
      int j_type = mbi->dimension_from_handle(ehandle);

      if ((integ_type == Coupler::VOLUME) && (j_type != 3))
        continue;

      // Retrieve the vertices from the element
      const EntityHandle * conn = NULL;
      int num_verts=0;
      result = mbi->get_connectivity(ehandle, conn, num_verts );
      if (MB_SUCCESS!=result) return 1;
      std::cout << std::fixed;
      for (int iv = 0; iv < num_verts; iv++) {
        double data = 0;
        result = mbi->tag_get_data(norm_hdl, &conn[iv], 1, &data);
        if (MB_SUCCESS!=result) return 1;
        std::cout << std::setprecision(8) << data << ", ";
      }
      std::cout << std::endl << "        ";
    }
    std::cout << std::endl;
    std::cout.unsetf(std::ios_base::floatfield);  // turn off fixed notation
  }

  return err;
}

// Function for a constant field value
double const_field(double /*x*/, double /*y*/, double /*z*/)
{
  //  return 5.0/40.0;
  return 5.0;
}

// Functions for a some field values
double field_1(double x, double y, double z)
{
  double f = fabs(x) + fabs(y) + fabs(z);
  //  return f/24.0;
  return f;
}

double field_2(double x, double y, double z)
{
  double f = x*x + y*y + z*z;
  //  return f/32.0;
  return f;
}

double field_3(double x, double y, double z)
{
  double f = 2*x + 2*y + 2*z;
  //  return f/48.0;
  return f;
}

// Function used to create field on mesh for testing.
double physField(double x, double y, double z)
{
  double out;

  // 1/r^2 decay from {0,0,0}

  out = x*x + y*y + z*z;
  out += 1e-1; // clamp
  out = 1/out;

  return out;
}

#define UINT_PER_X(X) ((sizeof(X)+sizeof(uint)-1)/sizeof(uint))
#define UINT_PER_REAL UINT_PER_X(real)
#define UINT_PER_LONG UINT_PER_X(slong)
#define UINT_PER_UNSIGNED UINT_PER_X(unsigned)

// Function for packing tuple_list
int pack_tuples(TupleList* tl, void **ptr)
{
  uint mi, ml, mul, mr;
  tl->getTupleSize(mi, ml, mul, mr);

  uint n = tl->get_n();

  int sz_buf = 1 + 4*UINT_PER_UNSIGNED +
    tl->get_n() * (mi + 
		   ml*UINT_PER_LONG + 
		   mul*UINT_PER_LONG + 
		   mr*UINT_PER_REAL);
  
  uint *buf = (uint*) malloc(sz_buf*sizeof(uint));
  *ptr = (void*) buf;

  // copy n
  memcpy(buf, &n,   sizeof(uint)),                buf+=1;
  // copy mi
  memcpy(buf, &mi,  sizeof(unsigned)),            buf+=UINT_PER_UNSIGNED;
  // copy ml
  memcpy(buf, &ml,  sizeof(unsigned)),            buf+=UINT_PER_UNSIGNED;
  // copy mul
  memcpy(buf, &mul, sizeof(unsigned)),            buf+=UINT_PER_UNSIGNED;
  // copy mr
  memcpy(buf, &mr,  sizeof(unsigned)),            buf+=UINT_PER_UNSIGNED;
  // copy vi_rd
  memcpy(buf, tl->vi_rd,     tl->get_n()*mi*sizeof(sint)),   buf+=tl->get_n()*mi;
  // copy vl_rd
  memcpy(buf, tl->vl_rd,     tl->get_n()*ml*sizeof(slong)),  buf+=tl->get_n()*ml*UINT_PER_LONG;
  // copy vul_rd
  memcpy(buf, tl->vul_rd,    tl->get_n()*mul*sizeof(ulong)), buf+=tl->get_n()*mul*UINT_PER_LONG;
  // copy vr_rd
  memcpy(buf, tl->vr_rd,     tl->get_n()*mr*sizeof(real)),   buf+=tl->get_n()*mr*UINT_PER_REAL;

  return sz_buf;
}

// Function for packing tuple_list
void unpack_tuples(void *ptr, TupleList** tlp)
{
  TupleList *tl = new TupleList();
  *tlp = tl;

  uint nt;
  unsigned mit, mlt, mult, mrt;
  uint *buf = (uint*)ptr;

  // get n
  memcpy(&nt,   buf, sizeof(uint)),          buf+=1;
  // get mi
  memcpy(&mit,  buf, sizeof(unsigned)),      buf+=UINT_PER_UNSIGNED;
  // get ml
  memcpy(&mlt,  buf, sizeof(unsigned)),      buf+=UINT_PER_UNSIGNED;
  // get mul
  memcpy(&mult, buf, sizeof(unsigned)),      buf+=UINT_PER_UNSIGNED;
  // get mr
  memcpy(&mrt,  buf, sizeof(unsigned)),      buf+=UINT_PER_UNSIGNED;

  // initalize tl
  tl->initialize(mit, mlt, mult, mrt, nt);
  tl->enableWriteAccess();
  tl->set_n( nt );

  uint mi, ml, mul, mr;
  tl->getTupleSize(mi, ml, mul, mr);

  // get vi_wr
  memcpy(tl->vi_wr,     buf, tl->get_n()*mi*sizeof(sint)),   buf+=tl->get_n()*mi;
  // get vl_wr
  memcpy(tl->vl_wr,     buf, tl->get_n()*ml*sizeof(slong)),  buf+=tl->get_n()*ml*UINT_PER_LONG;
  // get vul_wr
  memcpy(tl->vul_wr,    buf, tl->get_n()*mul*sizeof(ulong)), buf+=tl->get_n()*mul*UINT_PER_LONG;
  // get vr_wr
  memcpy(tl->vr_wr,     buf, tl->get_n()*mr*sizeof(real)),   buf+=tl->get_n()*mr*UINT_PER_REAL;

  tl->disableWriteAccess();

  return;
}

