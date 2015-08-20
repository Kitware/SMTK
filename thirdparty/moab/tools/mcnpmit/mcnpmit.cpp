#include <iostream>
#include <fstream>
#include <cstdlib>
#include "mcnpmit.hpp"
#include "moab/CartVect.hpp"
#include "math.h"

moab::Interface* mb_instance();

// Parameters
//const double pi   = 3.141592653589793;
const double c2pi = 0.1591549430918954;
//const double cpi  = 0.3183098861837907;

MCNPError next_number(std::string, double&, int&);
int how_many_numbers(std::string);
MCNPError read_numbers(std::string, int, std::vector<double>&);

// Constructor
McnpData::McnpData() {

      // Default value for coordinate system
      coord_system = 0;

      // Default rotation matrix is identity matrix
      for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                  if (i == j)
                        rotation_matrix[4*i + j] = 1;
                  else
                        rotation_matrix[4*i + j] = 0;
            }
      }

}

// Destructor
McnpData::~McnpData() {

      // Vertices and elements
      MCNP_vertices.clear();

}


// Setting and retrieving coordinate sysem
MCNPError McnpData::set_coord_system(int k) {
      coord_system = k;
      return MCNP_SUCCESS;
}
int McnpData::get_coord_system() {
      return coord_system;
}

// Setting and retrieving roation matrix
MCNPError McnpData::set_rotation_matrix(double r[16]) {
      for (int i = 0; i < 16; i++) {
            rotation_matrix[i] = r[i];
      }
      return MCNP_SUCCESS;
}
double* McnpData::get_rotation_matrix() {
      return rotation_matrix;
}


// Set the filename
MCNPError McnpData::set_filename(std::string fname) {
      MCNP_filename = fname;
      return MCNP_SUCCESS;
}
std::string McnpData::get_filename() {
      return MCNP_filename;
}


// Reading the MCNP file
MCNPError McnpData::read_mcnpfile(bool skip_mesh) {

      MCNPError result;
      moab::ErrorCode MBresult;
      moab::CartVect tvect;

      std::vector<double> xvec[3];

      // Open the file
      std::ifstream mcnpfile;
      mcnpfile.open( MCNP_filename.c_str() );
      if (!mcnpfile) {
            std::cout << "Unable to open MCNP data file." << std::endl;
            return MCNP_FAILURE;
      }
      std::cout << std::endl;
      std::cout << "Reading MCNP input file..." << std::endl;

      // Prepare for file reading ...
      char line[10000];  
      int mode = 0;         // Set the file reading mode to read proper data
      int nv[3];

      // Read in the file ...
      while (! mcnpfile.eof() ) {

            mcnpfile.getline(line, 10000);
            // std::cout << line << std::endl;

            switch(mode) {
            case 0:           // First line is a title
                  mode++;
            break;
            case 1:           // Coordinate system
                  mode++;
                  result = read_coord_system(line);
                  if (result == MCNP_FAILURE)
                        return MCNP_FAILURE;
            break;
            case 2:           // Rotation matrix
                  mode++;
                  for (int i = 0; i < 4; i++) {
                        mcnpfile.getline(line, 10000);
                        result = read_rotation_matrix(line, i);
                        if (result == MCNP_FAILURE)
                              return MCNP_FAILURE;
                  }
                  if (skip_mesh) return MCNP_SUCCESS;
            break;
            case 3:           // Read in vertices and build elements
                  mode++;

                  for (int i = 0; i < 3; i++) {
                        // How many points in the x[i]-direction
                        nv[i] = how_many_numbers(line);
                        if (nv[i] <= 0) return MCNP_FAILURE;

                        // Get space and read in these points
                        result = read_numbers(line , nv[i], xvec[i]);
                        if (result == MCNP_FAILURE) return MCNP_FAILURE;

                        // Update to the next line
                        mcnpfile.getline(line, 10000);
                  }

                  // Make the elements and vertices
                  result = make_elements(xvec, nv);
                  if (result == MCNP_FAILURE) return MCNP_FAILURE;
            break;
            case 4:           // Read in tally data, make, and tag elements
                  mode++;
                  moab::EntityHandle elemhandle;

                  moab::EntityHandle vstart, vijk;
                  moab::EntityHandle connect[8];
                  // double d[3];

                  // vstart = MCNP_vertices.front();
                  vstart = *(vert_handles.begin());

                      for (int i=0; i < nv[0]-1; i++) {
                    for (int j=0; j < nv[1]-1; j++) {
                  for (int k=0; k < nv[2]-1; k++) {
                        vijk = vstart + (i + j*nv[0] + k*nv[0]*nv[1]);

                        //std::cout << vijk << std::endl;                        

                        connect[0] = vijk;
                        connect[1] = vijk + 1;
                        connect[2] = vijk + 1 + nv[0];
                        connect[3] = vijk + nv[0];
                        connect[4] = vijk + nv[0]*nv[1];
                        connect[5] = vijk + 1 + nv[0]*nv[1];
                        connect[6] = vijk + 1 + nv[0] + nv[0]*nv[1];
                        connect[7] = vijk + nv[0] + nv[0]*nv[1];

                        MBresult = MBI->create_element(moab::MBHEX, connect, 8, elemhandle);
                        if (MBresult != moab::MB_SUCCESS) return MCNP_FAILURE;
                        elem_handles.insert(elemhandle);

                        mcnpfile.getline(line, 10000);
                        result = extract_tally_data(line, elemhandle);
                        if (result == MCNP_FAILURE) return MCNP_FAILURE;

                    }
                  }
                }

/*
                  for (MBRange::iterator rit=vert_handles.begin(); rit != vert_handles.end(); rit++) {
                        std::cout << *rit << std::endl; 
                  }


                  for (int i=0; i < nv[0]-1; i++) {
                    for (int j=0; j < nv[1]-1; j++) {
                      for (int k=0; k < nv[2]-1; k++) {
                        vijk = vstart + (i + j*nv[0] + k*nv[0]*nv[1]);
                        connect[0] = vijk;
                        connect[1] = vijk + 1;
                        connect[2] = vijk + 1 + nv[0];
                        connect[3] = vijk + nv[0];
                        connect[4] = vijk + nv[0]*nv[1];
                        connect[5] = vijk + 1 + nv[0]*nv[1];
                        connect[6] = vijk + 1 + nv[0] + nv[0]*nv[1];
                        connect[7] = vijk + nv[0] + nv[0]*nv[1];

                        MBresult = MBI->create_element(MBHEX, connect, 8, elemhandle);
                        if (MBresult != MB_SUCCESS) return MCNP_FAILURE;
                        elem_handles.insert(elemhandle);

                        mcnpfile.getline(line, 10000);
                        result = extract_tally_data(line, elemhandle);
                        if (result == MCNP_FAILURE) return MCNP_FAILURE;

                    }
                  }
                }
*/
            break;
            case 5:           // Ckeck for weirdness at end of file
                  if (! mcnpfile.eof() ) return MCNP_FAILURE;
            break;
            }

      }

      std::cout <<  "SUCCESS! Read in " << elem_handles.size() 
                << " elements!" << std::endl << std::endl;
      // MCNP_vertices.clear();
      vert_handles.clear();
      MCNP_elems.clear();
      return MCNP_SUCCESS;

}

MCNPError McnpData::read_coord_system(std::string s) {

      if ((s.find("Box") < 100) || (s.find("xyz") < 100))
            coord_system = CARTESIAN;     
      else if (s.find("Cyl") < 100)
            coord_system = CYLINDRICAL; 
      else if (s.find("Sph") < 100)
            coord_system = SPHERICAL; 
      else
            return MCNP_FAILURE;

      return MCNP_SUCCESS;
}

MCNPError McnpData::read_rotation_matrix(std::string s, int i) {

      int fpos = 0;
      MCNPError result;

      for (int j = 0; j < 4; j++) {
            result = next_number(s, rotation_matrix[4*i+j], fpos);
            if (result == MCNP_FAILURE) 
                  return MCNP_FAILURE;
      }

      return MCNP_SUCCESS;
}

MCNPError McnpData::make_elements(std::vector<double> x[3], int* n) {

      // double v[3];
      // MBEntityHandle dumhandle;
      // MBEntityHandle vstart, vijk;
      unsigned int num_verts = n[0]*n[1]*n[2];
      double       *coords;
      coords = new double [ 3 * num_verts ]; 

/*
      // Enter the vertices ...
      for (int k=0; k < n[2]; k++) {
            v[2] = x[2].at(k);
            for (int j=0; j < n[1]; j++) {
                  v[1] = x[1].at(j);
                  for (int i=0; i < n[0]; i++) {
                        v[0] = x[0].at(i);
                        MBresult = MBI->create_vertex(v, dumhandle);
                        if (MBresult != MB_SUCCESS) return MCNP_FAILURE;
                        MCNP_vertices.push_back(dumhandle);

                  }
            }
      }
*/

      // Enter the vertices ...
      for (int k=0; k < n[2]; k++) {
            for (int j=0; j < n[1]; j++) {
                  for (int i=0; i < n[0]; i++) {
                        unsigned int ijk = 3*(k*n[0]*n[1] + j*n[0] + i);
                        coords[ ijk   ] = x[0][i];
                        coords[ ijk+1 ] = x[1][j];
                        coords[ ijk+2 ] = x[2][k];

                        // std::cout << coords[ijk] << " " << coords[ijk+1] << " "
                        //          << coords[ijk+2] << std::endl;

                        // MCNP_vertices.push_back(dumhandle);
                  }
            }
      }

      MBI->create_vertices(coords, num_verts, vert_handles);
      

      delete[] coords;
      return MCNP_SUCCESS;
}

MCNPError McnpData::initialize_tags() {

      MBI->tag_get_handle(TALLY_TAG, 1, moab::MB_TYPE_DOUBLE, tally_tag, moab::MB_TAG_DENSE|moab::MB_TAG_CREAT);
      MBI->tag_get_handle(ERROR_TAG, 1, moab::MB_TYPE_DOUBLE, relerr_tag, moab::MB_TAG_DENSE|moab::MB_TAG_CREAT);

      return MCNP_SUCCESS;

}

MCNPError McnpData::extract_tally_data(std::string s, moab::EntityHandle handle) {

      int fpos = 0;
      double d = 0;

      MCNPError result;
      moab::ErrorCode MBresult;

      // Discard first three lines
      for (int i = 0; i < 3; i++) {
            result = next_number(s, d, fpos);
            if (result == MCNP_FAILURE) return MCNP_FAILURE;           
      }
      // Need to read in tally entry and tag ...
      result = next_number(s, d, fpos);
      if (result == MCNP_FAILURE) return MCNP_FAILURE;
      MBresult = MBI -> tag_set_data(tally_tag, &handle, 1, &d);
      if (MBresult != moab::MB_SUCCESS) return MCNP_FAILURE; 

      // Need to read in relative error entry and tag ...
      result = next_number(s, d, fpos);
      if (result == MCNP_FAILURE) return MCNP_FAILURE;
      MBresult = MBI -> tag_set_data(relerr_tag, &handle, 1, &d);
      if (MBresult != moab::MB_SUCCESS) return MCNP_FAILURE; 

      return MCNP_SUCCESS;
}

MCNPError next_number(std::string s, double &d, int &p) {

      unsigned int slen = s.length();
      unsigned int j;
      std::string sn;

      for (unsigned int i = p; i < slen; i++) {
            if ( ( (s[i] >= 48) && (s[i] <= 57) ) || (s[i] == 45) ) {

                  j = s.find(" ",i);

                  if (j > slen)
                        j = slen;

                  // Extract the number out of the string
                  d = std::atof(s.substr(i,j-i).c_str());
                  p = j+1;

                  return MCNP_SUCCESS;
            }

      }   

      return DONE;
}

int how_many_numbers(std::string s) {

      int n = -1;
      int fpos = 0;
      double d = 0;
      MCNPError result = MCNP_SUCCESS;

      while (result != DONE) {
            result = next_number(s, d, fpos);
            n++;
      }

      return n;

}

MCNPError read_numbers(std::string s, int n, std::vector<double> &x) {

      MCNPError result;
      int fpos = 0;
      double d;

      for (int i = 0; i < n; i++) {
            result = next_number(s, d, fpos);
            if (result == MCNP_FAILURE) return MCNP_FAILURE;
            x.push_back(d);
      }

      return MCNP_SUCCESS;
}

MCNPError McnpData::transform_point(double *p, double *r, int csys, double *rmat) {

      double q[3];

      // Apply the rotation matrix
      for (unsigned int i=0; i < 3; i++) {
        q[i] =  p[0] * rmat[4*i  ] + p[1] * rmat[4*i+1]
              + p[2] * rmat[4*i+2] +        rmat[4*i+3];
      }

      // Transform coordinate system
      switch( csys ) {
        case CARTESIAN :
          r[0] = q[0]; r[1] = q[1]; r[2] = q[2];  // x, y, z
        break;
        case CYLINDRICAL :
          r[0] = sqrt( q[0]*q[0] + q[1]*q[1] );   // r
          r[1] = q[2];                            // z
          r[2] = c2pi * ( atan2( q[1], q[0] ) );  // theta (in rotations)
        break;
        case SPHERICAL :
          return MCNP_FAILURE;
        //break;
        default :
          return MCNP_FAILURE;
        //break;
      }

      return MCNP_SUCCESS;

}
