#include <iostream>
#include <fstream>
#include <math.h>
#include <time.h>
#include <cstdlib>
#include "assert.h"

#include "mcnpmit.hpp"

#include "moab/CartVect.hpp"
#include "moab/Core.hpp"
#include "MBTagConventions.hpp"
#include "moab/AdaptiveKDTree.hpp"
#include "moab/GeomUtil.hpp"
#include "moab/FileOptions.hpp"
#include "../tools/mbcoupler/ElemUtil.hpp"

#define MBI mb_instance()

McnpData* mc_instance();
moab::Interface* mb_instance();
MCNPError read_files(int, char**);
MCNPError next_double(std::string, double&, int&);
MCNPError next_int(std::string, int&, int&);

MCNPError result;
moab::Tag     coord_tag, rotation_tag, cfd_heating_tag, cfd_error_tag;

std::string h5m_filename;
std::string CAD_filename;
std::string output_filename;

bool skip_build = false;
bool read_qnv   = false;

clock_t start_time, load_time, build_time, interp_time;

int main(int argc, char **argv) {

  start_time = clock();

  // Read in file names from command line
  result = read_files(argc, argv);
  if (result == MCNP_FAILURE) return 1;

  result = MCNP->initialize_tags();

  // Parse the MCNP input file
  if (!skip_build) {

    result = MCNP -> read_mcnpfile(skip_build);
    if (result == MCNP_FAILURE) {
      std::cout << "Failure reading MCNP file!" << std::endl;
      return 1;
    }
  } 
    

  load_time = clock() - start_time;

  // Make the KD-Tree
  moab::ErrorCode                MBresult;
  moab::AdaptiveKDTree           kdtree(MBI);
  moab::EntityHandle             root;

  MBI->tag_get_handle("CoordTag", 1, moab::MB_TYPE_INTEGER, coord_tag, moab::MB_TAG_DENSE|moab::MB_TAG_CREAT);
  MBI->tag_get_handle("RotationTag", 16, moab::MB_TYPE_DOUBLE, rotation_tag, moab::MB_TAG_DENSE|moab::MB_TAG_CREAT );

  if (skip_build) {
    MBresult = MBI->load_mesh(h5m_filename.c_str());

    if (moab::MB_SUCCESS == MBresult) {
      std::cout << std::endl << "Read in mesh from h5m file." << std::endl << std::endl;
      std::cout << "Querying mesh file..." << std::endl;
    }
    else {
      std::cout << "Failure reading h5m file!" << std::endl;
      std::cerr  << "Error code: " << MBI->get_error_string(MBresult) << " (" << MBresult << ")" << std::endl;
      std::string message;
      if (moab::MB_SUCCESS == MBI->get_last_error(message) && !message.empty())
	std::cerr << "Error message: " << message << std::endl;
      return 1;
    }

    moab::Range tmprange;
    kdtree.find_all_trees( tmprange );
    root = tmprange[0];
    
  }
  else {
    std::cout << "Building KD-Tree..." << std::endl;
    moab::FileOptions opts("CANDIDATE_PLANE_SET=SUBDIVISION");
    MBresult = kdtree.build_tree( MCNP -> elem_handles, &root, &opts);
    if (MBresult == moab::MB_SUCCESS) {

      MBI->tag_set_data(coord_tag, &root, 1, &(MCNP->coord_system));
      MBI->tag_set_data(rotation_tag, &root, 1, &(MCNP->rotation_matrix));

      std::cout << "KD-Tree has been built successfully!" << std::endl << std::endl;
      MBresult = MBI->write_mesh( (MCNP->MCNP_filename + ".h5m").c_str() );

      std::cout << "Querying mesh file..." << std::endl;
    }
    else {
      std::cout << "Error building KD-Tree!" << std::endl << std::endl;
      std::cerr  << "Error code: " << MBI->get_error_string(MBresult) << " (" << MBresult << ")" << std::endl;
      std::string message;
      if (moab::MB_SUCCESS == MBI->get_last_error(message) && !message.empty())
	std::cerr << "Error message: " << message << std::endl;
      return 1;
    }
  }

  int    coord_sys;
  double rmatrix[16];

  MBresult = MBI->tag_get_data( coord_tag, &root, 1, &coord_sys);
  MBresult = MBI->tag_get_data( rotation_tag, &root, 1, &rmatrix);

  build_time = clock() - load_time;

  // Read the CAD mesh data and query the tree
  std::ifstream cadfile;
  std::ofstream outfile;

  outfile.open( output_filename.c_str() );

  int num_pts;
  int n;
  long int nl = 0;
  char* ctmp;
  int elems_read = 0;
  int p = 0;
  std::string s;
  char line[10000];

  // Used only when reading a mesh file to get vertex info
  double *cfd_coords = NULL;
  moab::Range::iterator cfd_iter;
  moab::EntityHandle meshset;

  if (read_qnv) {      
    cadfile.open( CAD_filename.c_str() );
    cadfile.getline(line, 10000);
    cadfile.getline(line, 10000);
    result = next_int(line, num_pts, p);
  }
  else {

    meshset = 0;
    MBresult = MBI->load_file( CAD_filename.c_str(), &meshset );
    assert( moab::MB_SUCCESS == MBresult );
    assert( 0 != meshset );

    moab::Range cfd_verts;
    MBresult = MBI->get_entities_by_type( meshset, moab::MBVERTEX, cfd_verts, true);
    num_pts = cfd_verts.size();

    cfd_coords = new double [ 3 * num_pts ];
    MBresult = MBI->get_coords( cfd_verts , cfd_coords );  

    cfd_iter = cfd_verts.begin();
    MBresult = MBI->tag_get_handle("heating_tag", 1, moab::MB_TYPE_DOUBLE, cfd_heating_tag, moab::MB_TAG_DENSE|moab::MB_TAG_CREAT); 
    MBresult = MBI->tag_get_handle("error_tag", 1, moab::MB_TYPE_DOUBLE, cfd_error_tag, moab::MB_TAG_DENSE|moab::MB_TAG_CREAT);

    std::cout << std::endl << "Read in mesh with query points." << std::endl << std::endl;

  }
  

  double     testpt[3];
  double     transformed_pt[3];
  double     taldata;
  double     errdata;

  moab::CartVect testvc;

  bool found = false;

  // MBRange verts;
  std::vector<moab::EntityHandle> verts;
  moab::Range range;
  moab::CartVect box_max, box_min;

  moab::CartVect hexverts[8];
  moab::CartVect tmp_cartvect;
  std::vector<double> coords;

  double tal_sum     = 0.0,
         err_sum     = 0.0,
         tal_sum_sqr = 0.0,
         err_sum_sqr = 0.0;

//  double davg = 0.0;
//  unsigned int    nmax = 0, nmin = 1000000000 ;

  for (unsigned int i = 0; i < (unsigned int) num_pts; i++) {

    // if (i%status_freq == 0)
    //	std::cerr << "Completed " << i/status_freq << "%" << std::endl;
	
    // Grab the coordinates to query
    if (read_qnv) { 
      cadfile.getline(line, 10000);

      nl = std::strtol(line, &ctmp, 10); n  = (unsigned int) nl;
      testpt[0] = std::strtod( ctmp+1, &ctmp );
      testpt[1] = std::strtod( ctmp+1, &ctmp );
      testpt[2] = std::strtod( ctmp+1, NULL );
    }
    else {    
      testpt[0] = cfd_coords[3*i  ];
      testpt[1] = cfd_coords[3*i+1];
      testpt[2] = cfd_coords[3*i+2];      
      n = i+1;
    }

    result = MCNP->transform_point( testpt, transformed_pt, coord_sys, rmatrix );

    testvc[0] = transformed_pt[0]; 
    testvc[1] = transformed_pt[1]; 
    testvc[2] = transformed_pt[2];

    // Find the leaf containing the point
    moab::EntityHandle tree_node;
    MBresult = kdtree.point_search(transformed_pt, tree_node);
    if (moab::MB_SUCCESS != MBresult) {
      double x, y, z;
      if (CARTESIAN == coord_sys) {
        x = testvc[0];
        y = testvc[1];
	z = testvc[2];
      }
      else if (CYLINDRICAL == coord_sys) {
        x = testvc[0]*cos(2*M_PI*testvc[2]);
        y = testvc[0]*sin(2*M_PI*testvc[2]);
        z = testvc[1];
      } 
      else {
        assert(moab::MB_SUCCESS == MBresult);
      }
      std::cout << "No leaf found, MCNP coord xyz=" << x << " " << y << " " << z << std::endl;
      cfd_iter++;
      continue;
    }

    range.clear();
    MBresult = MBI -> get_entities_by_type(tree_node, moab::MBHEX, range );
    assert(MBresult == moab::MB_SUCCESS);

    // davg += (double) range.size();
    // if (range.size() > nmax) nmax = range.size();
    // if (range.size() < nmin) nmin = range.size();

    for (moab::Range::iterator rit = range.begin(); rit != range.end(); rit++) {
      verts.clear();
      const moab::EntityHandle *connect;
      int num_connect;
      MBresult = MBI -> get_connectivity( *rit, connect, num_connect, true); 
      assert(MBresult == moab::MB_SUCCESS);

      coords.resize(3*num_connect);
      MBresult = MBI -> get_coords( connect, num_connect, &coords[0]);
      assert(MBresult == moab::MB_SUCCESS);

      for (unsigned int j = 0; j < (unsigned int) num_connect; j++) {
	  hexverts[j][0] = coords[3*j];
	  hexverts[j][1] = coords[3*j+1];
	  hexverts[j][2] = coords[3*j+2];
      }

      if (moab::ElemUtil::point_in_trilinear_hex(hexverts, testvc, 1.e-6)) {
    	MBresult = MBI -> tag_get_data( MCNP->tally_tag, &(*rit), 1, &taldata);
	MBresult = MBI -> tag_get_data( MCNP->relerr_tag, &(*rit), 1, &errdata);

	outfile <<   n         << ","
	            << testpt[0] << ","
	  	    << testpt[1] << ","
		    << testpt[2] << ","
		    << taldata   << ","
		    << errdata   << std::endl;

        if (!read_qnv) {
          MBresult = MBI->tag_set_data(cfd_heating_tag, &(*cfd_iter), 1, &taldata);
	  MBresult = MBI->tag_set_data(cfd_error_tag, &(*cfd_iter), 1, &errdata);
        }

        found = true;
        elems_read++;
	
	tal_sum     = tal_sum + taldata;
	err_sum     = err_sum + errdata;
	tal_sum_sqr = tal_sum_sqr + taldata*taldata;
	err_sum_sqr = err_sum_sqr + errdata*errdata;

	  break;
      }
    }

    if (!read_qnv) cfd_iter++;

    if (!found) {
      std::cout << n << " " << testvc << std::endl;
    }
    found = false;

  }

  cadfile.close();
  outfile.close();

  if (result == MCNP_SUCCESS) {
    std::cout << "Success! " << elems_read << " elements interpolated." << std::endl << std::endl;
  }
  else {
    std::cout << "Failure during query! " << elems_read << " elements interpolated." << std::endl << std::endl;
  }


  double tal_std_dev = sqrt( (1.0/elems_read)*(tal_sum_sqr - (1.0/elems_read)*tal_sum*tal_sum) );
  double err_std_dev = sqrt( (1.0/elems_read)*(err_sum_sqr - (1.0/elems_read)*err_sum*err_sum) );

  std::cout << "Tally Mean:               " << tal_sum / elems_read << std::endl;
  std::cout << "Tally Standard Deviation: " << tal_std_dev << std::endl;
  std::cout << "Error Mean:               " << err_sum / elems_read << std::endl;
  std::cout << "Error Standard Deviation: " << err_std_dev << std::endl;

  interp_time = clock() - build_time;

  if (!read_qnv) {
    std::string out_mesh_fname = output_filename;
    MBresult = MBI->write_mesh( (out_mesh_fname + ".h5m").c_str(), &meshset, 1);
    // MBresult = MBI->write_file( (cfd_mesh_fname + ".vtk").c_str(), "vtk", NULL, &meshset, 1, &cfd_heating_tag, 1);
  }

  std::cout << "Time to read in file:     " << (double) load_time / CLOCKS_PER_SEC << std::endl;
  std::cout << "Time to build kd-tree:    " << (double) build_time / CLOCKS_PER_SEC << std::endl;
  std::cout << "Time to interpolate data: " << (double) interp_time / CLOCKS_PER_SEC << std::endl;

  return 0;

}



MCNPError read_files(int argc, char **argv) {

  // Check to see if appropriate command lines specified
  if (argc < 3) {
    std::cout << "Source and Target mesh filenames NOT specified!";
    std::cout << std::endl;
    return MCNP_FAILURE;
  }

  // Set the MCNP or H5M filename
  std::string str;
  str = argv[1];

  unsigned int itmp = str.find(".h5m");
  if ((itmp > 0) && (itmp < str.length())) {
    skip_build = true;
    h5m_filename = str;
  }
  else {
    result = MCNP -> set_filename(str);
  }

  // Set the CAD filename
  str = argv[2];
  CAD_filename = str;

  itmp = str.find(".qnv");
  if ((itmp > 0) && (itmp < str.length()))
    read_qnv = true;

  // Set the output filename
  str = argv[3];
  output_filename = str;


  return result;
}


MCNPError next_double(std::string s, double &d, int &p) {

  unsigned int slen = s.length();
  unsigned int j;
  std::string sn;

  for (unsigned int i = p; i < slen; i++) {
    if ( ( (s[i] >= 48) && (s[i] <= 57) ) || (s[i] == 45) ) {

      j = s.find(",",i);
      if (j > slen)
	j = slen;

      d = std::atof(s.substr(i,j-i).c_str());
      p = j+1;

      return MCNP_SUCCESS;
    }
  }   

  return DONE;
}


MCNPError next_int(std::string s, int &k, int &p) {

  unsigned int slen = s.length();
  unsigned int j;
  std::string sn;

  for (unsigned int i = p; i < slen; i++) {
    if ( ( (s[i] >= 48) && (s[i] <= 57) ) || (s[i] == 45) ) {

      j = s.find(",",i);
      if (j > slen)
	j = slen;

      k = std::atoi(s.substr(i,j-i).c_str());
      p = j+1;

      return MCNP_SUCCESS;
    }
  }   

  return DONE;
}


McnpData* mc_instance()
{
  static McnpData inst;
  return &inst;
}

moab::Interface* mb_instance()
{
  static moab::Core inst;
  return &inst;
}
