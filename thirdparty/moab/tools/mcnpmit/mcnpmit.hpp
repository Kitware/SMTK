#include "moab/Core.hpp"
#include "moab/Range.hpp"
#include <iostream>
#define MCNP mc_instance()
#define BOXMIN_TAG "BOXMIN_TAG"
#define BOXMAX_TAG "BOXMAX_TAG"
#define TALLY_TAG  "TALLY_TAG"
#define ERROR_TAG  "ERROR_TAG"

#define MBI mb_instance()

enum MCNPError { MCNP_SUCCESS, MCNP_FAILURE, DONE };
enum { NOSYS, CARTESIAN, CYLINDRICAL, SPHERICAL };

class McnpData {
      
      public:
            // Constructor and Destructor
            McnpData ();
            ~McnpData ();

            // Coordinate system and rotation matrix
            int coord_system;
            double rotation_matrix[16];

            // Vertices and elements
            std::vector<moab::EntityHandle> MCNP_vertices;
            std::vector<moab::EntityHandle> MCNP_elems;
            moab::Range                 vert_handles;
            moab::Range                 elem_handles;

            // Tally data
            moab::Tag box_min_tag, box_max_tag;
            moab::Tag tally_tag;
            moab::Tag relerr_tag;

            // MCNP Meshtal file name
            std::string MCNP_filename;

            // Setting and retrieving coordinate sysem
            MCNPError set_coord_system(int);
            int get_coord_system();

            // Setting and retrieving roation matrix
            MCNPError set_rotation_matrix(double[16]); 
            double* get_rotation_matrix();

            // Set the filename
            MCNPError set_filename(std::string);
            std::string get_filename();

            // MCNP reading routines
            MCNPError read_mcnpfile(bool);
            MCNPError read_coord_system(std::string);
            MCNPError read_rotation_matrix(std::string, int);
            MCNPError make_elements(std::vector<double> [3], int*);
            MCNPError make_adjacencies(int*);
            MCNPError initialize_tags();
            MCNPError extract_tally_data(std::string, moab::EntityHandle);

            // Transformation routine
            MCNPError transform_point(double*, double*, int, double*);
};
