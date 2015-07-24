#include "GeometryQueryTool.hpp"
#include "InitCGMA.hpp"
#include "CGMApp.hpp"
#include "moab/CN.hpp"
#include "moab/Core.hpp"
#include "moab/CartVect.hpp"
#include "moab/FileOptions.hpp"
#include "moab/Skinner.hpp"
#include "quads_to_tris.hpp"
#include "DagMC.hpp"
#include <limits>
#include <cstdlib>
#include <sstream>
#include <time.h>

#define GF_CUBIT_FILE_TYPE    "CUBIT"
#define GF_STEP_FILE_TYPE     "STEP"
#define GF_IGES_FILE_TYPE     "IGES"
#define GF_ACIS_TXT_FILE_TYPE "ACIS_SAT"
#define GF_ACIS_BIN_FILE_TYPE "ACIS_SAB"
#define GF_OCC_BREP_FILE_TYPE "OCC"

using namespace moab;

void tokenize(const std::string& str, std::vector<std::string>& tokens,
    const char* delimiters)
{
  std::string::size_type last = str.find_first_not_of(delimiters, 0);
  std::string::size_type pos = str.find_first_of(delimiters, last);
  if (std::string::npos == pos)
    tokens.push_back(str);
  else
    while (std::string::npos != pos && std::string::npos != last)
    {
      tokens.push_back(str.substr(last, pos - last));
      last = str.find_first_not_of(delimiters, pos);
      pos = str.find_first_of(delimiters, last);
      if (std::string::npos == pos)
        pos = str.size();
    }
}

ErrorCode get_group_names(Interface* MBI, const EntityHandle group_set,
    const Tag nameTag, std::vector<std::string> &grp_names)
{
  // get names
  char name0[NAME_TAG_SIZE];
  std::fill(name0, name0 + NAME_TAG_SIZE, '\0');
  ErrorCode result = MBI->tag_get_data(nameTag, &group_set, 1, &name0);
  if (MB_SUCCESS != result && MB_TAG_NOT_FOUND != result)
    return MB_FAILURE;

  if (MB_TAG_NOT_FOUND != result)
    grp_names.push_back(std::string(name0));

  return MB_SUCCESS;
}

// For each material, sum the volume. If the coordinates were updated for 
// deformation, summarize the volume change.
ErrorCode summarize_cell_volume_change(Interface* MBI,
    const EntityHandle cgm_file_set, const Tag categoryTag, const Tag dimTag,
    const Tag sizeTag, const Tag nameTag, const Tag idTag,
    const bool conserve_mass, const bool debug)
{
  // get groups
  ErrorCode rval;
  const char group_category[CATEGORY_TAG_SIZE] = { "Group\0" };
  const void* const group_val[] = { &group_category };
  Range groups;
  rval = MBI->get_entities_by_type_and_tag(0, MBENTITYSET, &categoryTag,
      group_val, 1, groups);
  if (MB_SUCCESS != rval)
    return rval;

  // Get the maximum group id. This is so that new groups do not have
  // duplicate ids.
  int max_grp_id = -1;
  for (Range::const_iterator i = groups.begin(); i != groups.end(); ++i)
  {
    int grp_id;
    rval = MBI->tag_get_data(idTag, &(*i), 1, &grp_id);
    if (MB_SUCCESS != rval)
      return rval;
    if (max_grp_id < grp_id)
      max_grp_id = grp_id;
  }
  if (conserve_mass)
  {
    std::cout << "  Altering group densities to conserve mass for each volume."
        << std::endl;
    std::cout << "    maximum group id=" << max_grp_id << std::endl;
  }

  for (Range::const_iterator i = groups.begin(); i != groups.end(); ++i)
  {
    // get group names
    std::vector<std::string> grp_names;
    rval = get_group_names(MBI, *i, nameTag, grp_names);
    if (MB_SUCCESS != rval)
      return MB_FAILURE;

    // determine if it is a material group
    bool material_grp = false;
    int mat_id = -1;
    double rho = 0;
    for (std::vector<std::string>::const_iterator j = grp_names.begin();
        j != grp_names.end(); ++j)
    {
      if (std::string::npos != (*j).find("mat")
          && std::string::npos != (*j).find("rho"))
      {
        material_grp = true;
        std::cout << "  material group: " << *j << std::endl;

        // get the density and material id
        std::vector<std::string> tokens;
        tokenize(*j, tokens, "_");
        mat_id = atoi(tokens[1].c_str());
        rho = atof(tokens[3].c_str());
      }
    }
    if (!material_grp)
      continue;

    // get the volume sets of the material group
    const int three = 3;
    const void* const three_val[] = { &three };
    Range vols;
    rval = MBI->get_entities_by_type_and_tag(*i, MBENTITYSET, &dimTag,
        three_val, 1, vols);
    if (MB_SUCCESS != rval)
      return rval;

    // for each volume, sum predeformed and deformed volume
    double orig_grp_volume = 0, defo_grp_volume = 0;
    for (Range::const_iterator j = vols.begin(); j != vols.end(); ++j)
    {
      double defo_size = 0, orig_size = 0;
      moab::DagMC &dagmc = *moab::DagMC::instance(MBI);
      rval = dagmc.measure_volume(*j, defo_size);
      if (MB_SUCCESS != rval)
        return rval;
      defo_grp_volume += defo_size;
      rval = MBI->tag_get_data(sizeTag, &(*j), 1, &orig_size);
      if (MB_SUCCESS != rval)
        return rval;
      orig_grp_volume += orig_size;

      // calculate a new density to conserve mass through the deformation
      if (!conserve_mass)
        continue;
      double new_rho = rho * orig_size / defo_size;

      // create a group for the volume with modified density
      EntityHandle new_grp;
      rval = MBI->create_meshset(MESHSET_SET, new_grp);
      if (MB_SUCCESS != rval)
        return rval;
      std::stringstream new_name_ss;
      new_name_ss << "mat_" << mat_id << "_rho_" << new_rho << "\0";
      std::string new_name;
      new_name_ss >> new_name;
      rval = MBI->tag_set_data(nameTag, &new_grp, 1, new_name.c_str());
      if (MB_SUCCESS != rval)
        return rval;
      max_grp_id++;
      rval = MBI->tag_set_data(idTag, &new_grp, 1, &max_grp_id);
      if (MB_SUCCESS != rval)
        return rval;
      const char group_category2[CATEGORY_TAG_SIZE] = "Group\0";
      rval = MBI->tag_set_data(categoryTag, &new_grp, 1, group_category2);
      if (MB_SUCCESS != rval)
        return rval;

      // add the volume to the new group
      rval = MBI->add_entities(new_grp, &(*j), 1);
      if (MB_SUCCESS != rval)
        return rval;

      // add the new grp to the cgm_file_set
      rval = MBI->add_entities(cgm_file_set, &new_grp, 1);
      if (MB_SUCCESS != rval)
        return rval;

      // remove the volume from the old group
      rval = MBI->remove_entities(*i, &(*j), 1);
      if (MB_SUCCESS != rval)
        return rval;
      if (debug)
        std::cout << "    new group: " << new_name << " id=" << max_grp_id
            << std::endl;
    }

    std::cout << "    orig_volume=" << orig_grp_volume << " defo_volume="
        << defo_grp_volume << " defo/orig=" << defo_grp_volume / orig_grp_volume
        << std::endl;

  }

  return MB_SUCCESS;
}

// DAGMC cannot build an OBB tree if all of a volume's surfaces have no facets.
// To prevent this, remove the cgm surface set if the cub surface set exists,
// but had its faced removed (due to dead elements). Remember that the cgm_file_set
// is not TRACKING.
ErrorCode remove_empty_cgm_surfs_and_vols(Interface *MBI,
    const EntityHandle cgm_file_set, const Tag idTag, const Tag dimTag,
    const bool /*debug */)
{

  ErrorCode result;
  const int two = 2;
  const void* const two_val[] = { &two };
  Range cgm_surfs;
  result = MBI->get_entities_by_type_and_tag(cgm_file_set, MBENTITYSET, &dimTag,
      two_val, 1, cgm_surfs);
  if (MB_SUCCESS != result)
    return result;

  for (Range::iterator i = cgm_surfs.begin(); i != cgm_surfs.end(); ++i)
  {
    int n_tris;
    result = MBI->get_number_entities_by_type(*i, MBTRI, n_tris);
    if (MB_SUCCESS != result)
      return result;

    if (0 == n_tris)
    {
      int surf_id;
      result = MBI->tag_get_data(idTag, &(*i), 1, &surf_id);
      assert(MB_SUCCESS == result);

      Range parent_vols;
      result = MBI->get_parent_meshsets(*i, parent_vols);
      assert(MB_SUCCESS == result);
      for (Range::iterator j = parent_vols.begin(); j != parent_vols.end(); ++j)
      {
        result = MBI->remove_parent_child(*j, *i);
        assert(MB_SUCCESS == result);
      }
      Range child_curves;
      result = MBI->get_child_meshsets(*i, child_curves);
      assert(MB_SUCCESS == result);
      for (Range::iterator j = child_curves.begin(); j != child_curves.end();
          ++j)
      {
        result = MBI->remove_parent_child(*i, *j);
        assert(MB_SUCCESS == result);
      }
      result = MBI->remove_entities(cgm_file_set, &(*i), 1);
      assert(MB_SUCCESS == result);

      // Is the set contained anywhere else? If the surface is in a CUBIT group, 
      // such as "unmerged_surfs" it will cause write_mesh to fail. This should 
      // be a MOAB bug.
      Range all_sets;
      result = MBI->get_entities_by_type(0, MBENTITYSET, all_sets);
      assert(MB_SUCCESS == result);
      for (Range::iterator j = all_sets.begin(); j != all_sets.end(); ++j)
      {
        if (MBI->contains_entities(*j, &(*i), 1))
        {
          result = MBI->remove_entities(*j, &(*i), 1);
          assert(MB_SUCCESS == result);
        }
      }

      result = MBI->delete_entities(&(*i), 1);
      assert(MB_SUCCESS == result);
      std::cout << "  Surface " << surf_id
          << ": removed because all of its mesh faces have been removed"
          << std::endl;
    }
  }

  // get all volumes
  const int three = 3;
  const void* const three_val[] = { &three };
  Range cgm_vols;
  result = MBI->get_entities_by_type_and_tag(cgm_file_set, MBENTITYSET, &dimTag,
      three_val, 1, cgm_vols);
  if (MB_SUCCESS != result)
    return result;

  for (Range::iterator i = cgm_vols.begin(); i != cgm_vols.end(); ++i)
  {
    // get the volume's number of surfaces
    int n_surfs;
    result = MBI->num_child_meshsets(*i, &n_surfs);
    assert(MB_SUCCESS == result);

    // if no surfaces remain, remove the volume
    if (0 == n_surfs)
    {
      int vol_id;
      result = MBI->tag_get_data(idTag, &(*i), 1, &vol_id);
      assert(MB_SUCCESS == result);
      // Is the set contained anywhere else? If the surface is in a CUBIT group, 
      // such as "unmerged_surfs" it will cause write_mesh to fail. This should 
      // be a MOAB bug.
      Range all_sets;
      result = MBI->get_entities_by_type(0, MBENTITYSET, all_sets);
      assert(MB_SUCCESS == result);
      for (Range::iterator j = all_sets.begin(); j != all_sets.end(); ++j)
      {
        if (MBI->contains_entities(*j, &(*i), 1))
        {
          result = MBI->remove_entities(*j, &(*i), 1);
          assert(MB_SUCCESS == result);
        }
      }
      result = MBI->delete_entities(&(*i), 1);
      assert(MB_SUCCESS == result);
      std::cout << "  Volume " << vol_id
          << ": removed because all of its surfaces have been removed"
          << std::endl;
    }
  }
  return MB_SUCCESS;
}

// Given parent volume senses, an id, and a set handle, this function creates a
// new surface set with dimension, geometry category, id, and sense tags.
ErrorCode build_new_surface(Interface *MBI, EntityHandle &new_surf,
    const EntityHandle forward_parent_vol,
    const EntityHandle reverse_parent_vol, const int new_surf_id,
    const Tag dimTag, const Tag idTag, const Tag categoryTag,
    const Tag senseTag)
{

  ErrorCode result;
  result = MBI->create_meshset(0, new_surf);
  if (MB_SUCCESS != result)
    return result;
  if (0 != forward_parent_vol)
  {
    result = MBI->add_parent_child(forward_parent_vol, new_surf);
    if (MB_SUCCESS != result)
      return result;
  }
  if (0 != reverse_parent_vol)
  {
    result = MBI->add_parent_child(reverse_parent_vol, new_surf);
    if (MB_SUCCESS != result)
      return result;
  }
  const int two = 2;
  result = MBI->tag_set_data(dimTag, &new_surf, 1, &two);
  if (MB_SUCCESS != result)
    return result;
  result = MBI->tag_set_data(idTag, &new_surf, 1, &new_surf_id);
  if (MB_SUCCESS != result)
    return result;
  const char geom_category[CATEGORY_TAG_SIZE] = { "Surface\0" };
  result = MBI->tag_set_data(categoryTag, &new_surf, 1, &geom_category);
  if (MB_SUCCESS != result)
    return result;
  EntityHandle vols[2] = { forward_parent_vol, reverse_parent_vol };
  result = MBI->tag_set_data(senseTag, &new_surf, 1, vols);
  if (MB_SUCCESS != result)
    return result;

  return MB_SUCCESS;
}

// Given a face, orient it outward wrt its adjacent mesh element.
// Each face must be adjacent to exactly one mesh element.
ErrorCode orient_faces_outward(Interface *MBI, const Range faces,
    const bool /*debug*/)
{

  ErrorCode result;
  for (Range::const_iterator i = faces.begin(); i != faces.end(); ++i)
  {
    Range adj_elem;
    result = MBI->get_adjacencies(&(*i), 1, 3, false, adj_elem);
    if (MB_SUCCESS != result)
      return result;
    if (1 != adj_elem.size())
      return MB_INVALID_SIZE;

    // get connectivity for element and face
    const EntityHandle *elem_conn;
    int elem_n_nodes;
    result = MBI->get_connectivity(adj_elem.front(), elem_conn, elem_n_nodes);
    if (MB_SUCCESS != result)
      return result;
    const EntityHandle *face_conn;
    int face_n_nodes;
    result = MBI->get_connectivity(*i, face_conn, face_n_nodes);
    if (MB_SUCCESS != result)
      return result;

    // Get the sense of the face wrt the element
    EntityType elem_type = MBI->type_from_handle(adj_elem.front());
    EntityType face_type = MBI->type_from_handle(*i);
    int face_num, offset;
    int sense = 0;
    const int face_dim = CN::Dimension(face_type);
    int rval = CN::SideNumber(elem_type, elem_conn, face_conn, face_n_nodes,
        face_dim, face_num, sense, offset);
    if (0 != rval)
      return MB_FAILURE;

    // If the face is not oriented outward wrt the element, reverse it
    if (-1 == sense)
    {
      EntityHandle new_face_conn[4] = { face_conn[3], face_conn[2],
          face_conn[1], face_conn[0] };
      result = MBI->set_connectivity(*i, new_face_conn, 4);
      if (MB_SUCCESS != result)
        return result;
    }
  }
  return MB_SUCCESS;
}

/* Isolate dead code in a preproc-killed block - sjackson 11/22/10 */
#if 0 

/* qsort int comparison function */
int handle_compare(const void *a, const void *b)
{
  const EntityHandle *ia = (const EntityHandle *)a; // casting pointer types
  const EntityHandle *ib = (const EntityHandle *)b;
  return *ia - *ib;
  /* integer comparison: returns negative if b > a 
   and positive if a > b */
}

// qsort face comparison function. assume each face has 4 nodes
int compare_face(const void *a, const void *b)
{
  EntityHandle *ia = (EntityHandle *)a;
  EntityHandle *ib = (EntityHandle *)b;
  if(*ia == *ib)
  {
    if(*(ia+1) == *(ib+1))
    {
      if(*(ia+2) == *(ib+2))
      {
        return (int)(*(ia+3) - *(ib+3));
      }
      else
      {
        return (int)(*(ia+2) - *(ib+2));
      }
    }
    else
    {
      return (int)(*(ia+1) - *(ib+1));
    }
  }
  else
  {
    return (int)(*ia - *ib);
  }
}

// Use this to get quad faces from hex elems.
ErrorCode skin_hex_elems(Interface *MBI, Range elems, const int dim,
    Range &faces )
{
  // get faces of each hex
  const int nodes_per_face = 4;
  const unsigned int faces_per_elem = 6;
  unsigned int n_faces = faces_per_elem*elems.size();
  EntityHandle f[n_faces][nodes_per_face];
  ErrorCode result;
  int counter = 0;
  for(Range::iterator i=elems.begin(); i!=elems.end(); ++i)
  {
    Range elem_faces;
    result = MBI->get_adjacencies( &(*i), 1, 2, true, elem_faces );
    if(MB_SUCCESS != result) return result;
    if(faces_per_elem != elem_faces.size()) return MB_INVALID_SIZE;
    for(Range::iterator j=elem_faces.begin(); j!=elem_faces.end(); ++j)
    {
      const EntityHandle *conn;
      int n_nodes;
      ErrorCode result = MBI->get_connectivity( *j, conn, n_nodes );
      if(MB_SUCCESS != result) return result;
      if(nodes_per_face != n_nodes) return MB_INVALID_SIZE;
      // Sort the node handles of the face
      for(int k=0; k<nodes_per_face; ++k) f[counter][k] = conn[k];
      qsort( &f[counter][0], nodes_per_face, sizeof(EntityHandle), handle_compare );
      ++counter;
    }
  }

  // Sort the faces by the first node handle, then second node, then third node...
  qsort( &f[0][0], n_faces, nodes_per_face*sizeof(EntityHandle), compare_face );

  // if a face has 1 or more duplicates, it is not on the skin
  faces.clear();
  for(unsigned int i=0; i<n_faces; ++i)
  {
    // if the last face is tested, it must be on the skin
    if(n_faces-1 == i)
    {
      Range face_handle;
      result = MBI->get_adjacencies( &(f[i][0]), nodes_per_face, 2, false, face_handle );
      if(MB_SUCCESS != result) return result;
      if(1 != face_handle.size()) return MB_INVALID_SIZE;
      faces.insert( face_handle.front() );
      // Due to sort, if a duplicate exists it must be next
    }
    else if( f[i][0]==f[i+1][0] && f[i][1]==f[i+1][1] &&
        f[i][2]==f[i+1][2] && f[i][3]==f[i+1][3] )
    {
      ++i;
      while( f[i][0]==f[i+1][0] && f[i][1]==f[i+1][1] &&
          f[i][2]==f[i+1][2] && f[i][3]==f[i+1][3] )
      {
        std::cout << "    skin WARNING: non-manifold face" << std::endl;
        ++i;
      }
      // otherwise it is on the skin
    }
    else
    {
      Range face_handle;
      result = MBI->get_adjacencies( &(f[i][0]), nodes_per_face, 2, false, face_handle );
      if(MB_SUCCESS != result) return result;
      if(1 != face_handle.size()) return MB_INVALID_SIZE;
      faces.insert( face_handle.front() );
    }
  }

  return MB_SUCCESS;
}

#endif //dead code isolation
// Given a 1D array of data, axis labels, title, and number of bins, create a
// histogram.
void plot_histogram(const std::string title, const std::string x_axis_label,
    const std::string y_axis_label, const int n_bins, const double data[],
    const int n_data)
{
  // find max and min
  double min = std::numeric_limits<double>::max();
  double max = -std::numeric_limits<double>::max();
  for (int i = 0; i < n_data; ++i)
  {
    if (min > data[i])
      min = data[i];
    if (max < data[i])
      max = data[i];

  }

  // make bins for histogram
  double bin_width = (max - min) / n_bins;
  std::vector<int> bins(n_bins);
  for (int i = 0; i < n_bins; ++i)
    bins[i] = 0;

  // fill the bins
  for (int i = 0; i < n_data; ++i)
  {
    double diff = data[i] - min;
    int bin = diff / bin_width;
    if (9 < bin)
      bin = 9; // cheap fix for numerical precision error
    if (0 > bin)
      bin = 0; // cheap fix for numerical precision error
    ++bins[bin];
  }

  // create bars
  int max_bin = 0;
  for (int i = 0; i < n_bins; ++i)
    if (max_bin < bins[i])
      max_bin = bins[i];
  int bar_height;
  int max_bar_chars = 72;
  std::vector<std::string> bars(n_bins);
  for (int i = 0; i < n_bins; ++i)
  {
    bar_height = (max_bar_chars * bins[i]) / max_bin;
    for (int j = 0; j < bar_height; ++j)
      bars[i] += "*";
  }

  // print histogram header
  std::cout << std::endl;
  std::cout << "                                 " << title << std::endl;

  // print results
  std::cout.width(15);
  std::cout << min << std::endl;
  for (int i = 0; i < n_bins; ++i)
  {
    std::cout.width(15);
    std::cout << min + ((i + 1) * bin_width);
    std::cout.width(max_bar_chars);
    std::cout << bars[i] << bins[i] << std::endl;
  }

  // print histogram footer
  std::cout.width(15);
  std::cout << y_axis_label;
  std::cout.width(max_bar_chars);
  std::cout << " " << x_axis_label << std::endl;
  std::cout << std::endl;
}

// This is a helper function that creates data and labels for histograms.
void generate_plots(const double orig[], const double defo[], const int n_elems,
    const std::string time_step)
{

  // find volume ratio then max and min
  double *ratio = new double[n_elems];
  for (int i = 0; i < n_elems; ++i)
    ratio[i] = (defo[i] - orig[i]) / orig[i];

  plot_histogram("Predeformed Element Volume", "Num_Elems", "Volume [cc]", 10,
      orig, n_elems);
  std::string title = "Element Volume Change Ratio at Time Step " + time_step;
  plot_histogram(title, "Num_Elems", "Volume Ratio", 10, ratio, n_elems);
  delete[] ratio;
}

// Given four nodes, calculate the tet volume.
inline static double tet_volume(const CartVect& v0, const CartVect& v1,
    const CartVect& v2, const CartVect& v3)
{
  return 1. / 6. * (((v1 - v0) * (v2 - v0)) % (v3 - v0));
}

// Measure and tet volume are taken from measure.cpp
double measure(Interface *MBI, const EntityHandle element)
{
  EntityType type = MBI->type_from_handle(element);

  const EntityHandle *conn;
  int num_vertices;
  ErrorCode result = MBI->get_connectivity(element, conn, num_vertices);
  if (MB_SUCCESS != result)
    return result;

  std::vector<CartVect> coords(num_vertices);
  result = MBI->get_coords(conn, num_vertices, coords[0].array());
  if (MB_SUCCESS != result)
    return result;

  switch (type)
  {
  case MBEDGE:
    return (coords[0] - coords[1]).length();
  case MBTRI:
    return 0.5 * ((coords[1] - coords[0]) * (coords[2] - coords[0])).length();
  case MBQUAD:
    num_vertices = 4;
  case MBPOLYGON:
  {
    CartVect mid(0, 0, 0);
    for (int i = 0; i < num_vertices; ++i)
      mid += coords[i];
    mid /= num_vertices;

    double sum = 0.0;
    for (int i = 0; i < num_vertices; ++i)
    {
      int j = (i + 1) % num_vertices;
      sum += ((mid - coords[i]) * (mid - coords[j])).length();
    }
    return 0.5 * sum;
  }
  case MBTET:
    return tet_volume(coords[0], coords[1], coords[2], coords[3]);
  case MBPYRAMID:
    return tet_volume(coords[0], coords[1], coords[2], coords[4])
        + tet_volume(coords[0], coords[2], coords[3], coords[4]);
  case MBPRISM:
    return tet_volume(coords[0], coords[1], coords[2], coords[5])
        + tet_volume(coords[3], coords[5], coords[4], coords[0])
        + tet_volume(coords[1], coords[4], coords[5], coords[0]);
  case MBHEX:
    return tet_volume(coords[0], coords[1], coords[3], coords[4])
        + tet_volume(coords[7], coords[3], coords[6], coords[4])
        + tet_volume(coords[4], coords[5], coords[1], coords[6])
        + tet_volume(coords[1], coords[6], coords[3], coords[4])
        + tet_volume(coords[2], coords[6], coords[3], coords[1]);
  default:
    return 0.0;
  }
}

/* Calculate the signed volumes beneath the surface (x 6.0). Use the triangle's
 cannonical sense. Do not take sense tags into account. Code taken from
 DagMC::measure_volume.

 Special Case: If the surface is planar, and the plane includes the origin,
 the signed volume will be ~0. If the signed volume is ~0 then offset everything
 by a random amount and try again. */
ErrorCode get_signed_volume(Interface *MBI, const EntityHandle surf_set,
    const CartVect offset, double &signed_volume)
{
  ErrorCode rval;
  Range tris;
  rval = MBI->get_entities_by_type(surf_set, MBTRI, tris);
  if (MB_SUCCESS != rval)
    return rval;
  signed_volume = 0.0;
  const EntityHandle *conn;
  int len;
  CartVect coords[3];
  for (Range::iterator j = tris.begin(); j != tris.end(); ++j)
  {
    rval = MBI->get_connectivity(*j, conn, len, true);
    if (MB_SUCCESS != rval)
      return rval;
    if (3 != len)
      return MB_INVALID_SIZE;
    rval = MBI->get_coords(conn, 3, coords[0].array());
    if (MB_SUCCESS != rval)
      return rval;

    // Apply offset to avoid calculating 0 for cases when the origin is in the
    // plane of the surface.
    for (unsigned int k = 0; k < 3; ++k)
    {
      coords[k][0] += offset[0];
      coords[k][1] += offset[1];
      coords[k][2] += offset[2];
    }

    coords[1] -= coords[0];
    coords[2] -= coords[0];
    signed_volume += (coords[0] % (coords[1] * coords[2]));
  }
  return MB_SUCCESS;
}

// The cgm and cub surfaces may not have the same sense. Create triangles that
// represent the quads in the cub surface. Calculate the signed volume of both
// the cgm and cub surface. If they are different, change the cgm sense so that
// it matches the sense of the cub surface.
ErrorCode fix_surface_senses(Interface *MBI, const EntityHandle cgm_file_set,
    const EntityHandle cub_file_set, const Tag idTag, const Tag dimTag,
    const Tag senseTag, const bool debug)
{
  ErrorCode result;
  const int two = 2;
  const void* const two_val[] = { &two };
  Range cgm_surfs;
  result = MBI->get_entities_by_type_and_tag(cgm_file_set, MBENTITYSET, &dimTag,
      two_val, 1, cgm_surfs);
  if (MB_SUCCESS != result)
    return result;
  for (Range::iterator i = cgm_surfs.begin(); i != cgm_surfs.end(); i++)
  {
    int surf_id;
    result = MBI->tag_get_data(idTag, &(*i), 1, &surf_id);
    if (MB_SUCCESS != result)
      return result;

    // Find the meshed surface set with the same id
    Range cub_surf;
    const Tag tags[] = { idTag, dimTag };
    const void* const tag_vals[] = { &surf_id, &two };
    result = MBI->get_entities_by_type_and_tag(cub_file_set, MBENTITYSET, tags,
        tag_vals, 2, cub_surf);
    if (MB_SUCCESS != result)
      return result;
    if (1 != cub_surf.size())
    {
      std::cout << "  Surface " << surf_id
          << ": no meshed representation found, using CAD representation instead"
          << std::endl;
      continue;
    }

    // Get tris that represent the quads of the cub surf
    Range quads;
    result = MBI->get_entities_by_type(cub_surf.front(), MBQUAD, quads);
    if (MB_SUCCESS != result)
      return result;
    Range cub_tris;
    result = make_tris_from_quads(MBI, quads, cub_tris);

    // Add the tris to the same surface meshset as the quads are inside.            
    result = MBI->add_entities(cub_surf.front(), cub_tris);
    if (MB_SUCCESS != result)
      return result;

    // get the signed volume for each surface representation. Keep trying until
    // The signed volumes are much greater than numerical precision. Planar
    // surfaces will have a signed volume of zero if the plane goes through the
    // origin, unless we apply an offset.
    const int n_attempts = 100;
    const int max_random = 10;
    const double min_signed_vol = 0.1;
    double cgm_signed_vol, cub_signed_vol;
    for (int j = 0; j < n_attempts; ++j)
    {
      cgm_signed_vol = 0;
      cub_signed_vol = 0;
      CartVect offset(std::rand() % max_random, std::rand() % max_random,
          std::rand() % max_random);
      result = get_signed_volume(MBI, *i, offset, cgm_signed_vol);
      if (MB_SUCCESS != result)
        return result;
      result = get_signed_volume(MBI, cub_surf.front(), offset, cub_signed_vol);
      if (MB_SUCCESS != result)
        return result;
      if (debug)
        std::cout << "  surf_id=" << surf_id << " cgm_signed_vol="
            << cgm_signed_vol << " cub_signed_vol=" << cub_signed_vol
            << std::endl;
      if (fabs(cgm_signed_vol) > min_signed_vol
          && fabs(cub_signed_vol) > min_signed_vol)
        break;
      if (n_attempts == j + 1)
      {
        std::cout
            << "error: signed volume could not be calculated unambiguously"
            << std::endl;
        return MB_FAILURE;
      }
    }

    // If the sign is different, reverse the cgm senses so that both
    // representations have the same signed volume.
    if ((cgm_signed_vol < 0 && cub_signed_vol > 0)
        || (cgm_signed_vol > 0 && cub_signed_vol < 0))
    {
      EntityHandle cgm_surf_volumes[2], reversed_cgm_surf_volumes[2];
      result = MBI->tag_get_data(senseTag, &(*i), 1, cgm_surf_volumes);
      if (MB_SUCCESS != result)
        return result;
      if (MB_SUCCESS != result)
        return result;

      reversed_cgm_surf_volumes[0] = cgm_surf_volumes[1];
      reversed_cgm_surf_volumes[1] = cgm_surf_volumes[0];

      result = MBI->tag_set_data(senseTag, &(*i), 1, reversed_cgm_surf_volumes);
      if (MB_SUCCESS != result)
        return result;
    }
  }

  return MB_SUCCESS;
}

// The quads in the cub_file_set have been updated for dead elements. For each
// cgm_surf, if there exists a cub_surf with the same id, replace the cgm tris
// with cub_tris (created from the quads). Note the a surface that is not
// meshed (in cub file) will not be effected.
ErrorCode replace_faceted_cgm_surfs(Interface *MBI,
    const EntityHandle cgm_file_set, const EntityHandle cub_file_set,
    const Tag idTag, const Tag dimTag, const bool debug)
{
  ErrorCode result;
  const int two = 2;
  const void* const two_val[] = { &two };
  Range cgm_surfs;
  result = MBI->get_entities_by_type_and_tag(cgm_file_set, MBENTITYSET, &dimTag,
      two_val, 1, cgm_surfs);
  if (MB_SUCCESS != result)
    return result;

  for (Range::iterator i = cgm_surfs.begin(); i != cgm_surfs.end(); ++i)
  {
    int surf_id;
    result = MBI->tag_get_data(idTag, &(*i), 1, &surf_id);
    if (MB_SUCCESS != result)
      return result;
    if (debug)
      std::cout << "surf_id=" << surf_id << std::endl;

    // Find the meshed surface set with the same id
    Range cub_surf;
    const Tag tags[] = { idTag, dimTag };
    const void* const tag_vals[] = { &surf_id, &two };
    result = MBI->get_entities_by_type_and_tag(cub_file_set, MBENTITYSET, tags,
        tag_vals, 2, cub_surf);
    if (MB_SUCCESS != result)
      return result;
    if (1 != cub_surf.size())
    {
      std::cout << "  Surface " << surf_id
          << ": no meshed representation found, using CAD representation instead"
          << std::endl;
      continue;
    }

    // Get tris that represent the quads of the cub surf
    Range quads;
    result = MBI->get_entities_by_type(cub_surf.front(), MBQUAD, quads);
    if (MB_SUCCESS != result)
      return result;

    Range cub_tris;
    result = make_tris_from_quads(MBI, quads, cub_tris);
    if (MB_SUCCESS != result)
      return result;

    // Remove the tris from the cgm surf. Don't forget to remove them from the
    // cgm_file_set because it is not TRACKING.
    Range cgm_tris;
    result = MBI->get_entities_by_type(*i, MBTRI, cgm_tris);
    if (MB_SUCCESS != result)
      return result;
    result = MBI->remove_entities(*i, cgm_tris);
    if (MB_SUCCESS != result)
      return result;
    result = MBI->remove_entities(cgm_file_set, cgm_tris);
    if (MB_SUCCESS != result)
      return result;
    result = MBI->delete_entities(cgm_tris);
    if (MB_SUCCESS != result)
      return result;

    // Add the cub_tris to the cgm_surf
    result = MBI->add_entities(*i, cub_tris);
    if (MB_SUCCESS != result)
      return result;
  }

  return MB_SUCCESS;
}

// Dead elements need removed from the simulation. This is done by removing them
// from their volume set and adding them to the implicit complement. New surfaces
// must be created for this. 
// IF MODIFYING THIS CODE, BE AWARE THAT DEAD ELEMENTS CAN BE ADJACENT TO MORE
// THAN ONE SURFACE, MAKING THE ASSOCIATION BETWEEN NEWLY EXPOSED AND EXISTING
// SURFACES AMBIGUOUS.
ErrorCode add_dead_elems_to_impl_compl(Interface *MBI,
    const EntityHandle cgm_file_set, const EntityHandle cub_file_set,
    const Tag idTag, const Tag dimTag, const Tag categoryTag,
    const Tag senseTag, const bool debug)
{

  // Get the cgm surfaces
  ErrorCode result;
  const int two = 2;
  const void* const two_val[] = { &two };
  Range cgm_surfs;
  result = MBI->get_entities_by_type_and_tag(cgm_file_set, MBENTITYSET, &dimTag,
      two_val, 1, cgm_surfs);
  if (MB_SUCCESS != result)
    return result;

  // Get the maximum surface id. This is so that new surfaces do not have
  // duplicate ids.
  int max_surf_id = -1;
  for (Range::const_iterator i = cgm_surfs.begin(); i != cgm_surfs.end(); ++i)
  {
    int surf_id;
    result = MBI->tag_get_data(idTag, &(*i), 1, &surf_id);
    if (MB_SUCCESS != result)
      return result;
    if (max_surf_id < surf_id)
      max_surf_id = surf_id;
  }
  std::cout << "  Maximum surface id=" << max_surf_id << std::endl;

  // For each cgm volume, does a cub volume with the same id exist?
  const int three = 3;
  const void* const three_val[] = { &three };
  Range cgm_vols;
  result = MBI->get_entities_by_type_and_tag(cgm_file_set, MBENTITYSET, &dimTag,
      three_val, 1, cgm_vols);
  if (MB_SUCCESS != result)
    return result;

  // get the corresponding cub volume
  for (Range::iterator i = cgm_vols.begin(); i != cgm_vols.end(); i++)
  {
    int vol_id;
    result = MBI->tag_get_data(idTag, &(*i), 1, &vol_id);
    assert(MB_SUCCESS == result);
    if (MB_SUCCESS != result)
      return result;
    std::cout << "  Volume " << vol_id;

    // Find the meshed vol set with the same id
    Range cub_vol;
    const Tag tags[] = { idTag, dimTag };
    const void* const tag_vals[] = { &vol_id, &three };
    result = MBI->get_entities_by_type_and_tag(cub_file_set, MBENTITYSET, tags,
        tag_vals, 2, cub_vol);
    assert(MB_SUCCESS == result);
    if (MB_SUCCESS != result)
      return result;
    if (1 != cub_vol.size())
    {
      std::cout << ": no meshed representation found" << std::endl;
      continue;
    }
    else
    {
      std::cout << std::endl;
    }

    // get the mesh elements of the volume.
    Range elems;
    result = MBI->get_entities_by_type(cub_vol.front(), MBHEX, elems);
    assert(MB_SUCCESS == result);
    if (MB_SUCCESS != result)
      return result;
    if (debug)
      std::cout << "    found " << elems.size() << " hex elems" << std::endl;

    // skin the volumes
    Skinner tool(MBI);
    Range skin_faces;
    result = tool.find_skin(0, elems, 2, skin_faces, true);
    assert(MB_SUCCESS == result);
    if (MB_SUCCESS != result)
      return result;

    // Reconcile the difference between faces of the cub file surfaces and skin
    // faces of the 3D mesh. The difference exists because dead elements have been
    // removed. Faces are divided into:
    //   cub_faces    (in) - the faces in the cub file surface
    //   skin_faces   (in) - the faces of the 3D mesh elements
    //   common_faces (out)- the faces common to both the cub file surface and 3D mesh
    //                       they are still adjacent to this volume
    //   old_faces    (out)- the faces of the cub surface not on the 3D mesh skin
    //                       they are no longer adjacent to this vol
    //   new_faces    (out)- the faces of the 3D mesh skin not on the cub surface
    //                       they are now adjacent to this volume

    // get cub child surfaces.
    Range cub_surfs;
    result = MBI->get_child_meshsets(cub_vol.front(), cub_surfs);
    assert(MB_SUCCESS == result);
    if (MB_SUCCESS != result)
      return result;
    for (Range::iterator j = cub_surfs.begin(); j != cub_surfs.end(); ++j)
    {
      int surf_id;
      result = MBI->tag_get_data(idTag, &(*j), 1, &surf_id);
      assert(MB_SUCCESS == result);
      if (MB_SUCCESS != result)
        return result;
      // get the quads on each surface
      Range cub_faces;
      result = MBI->get_entities_by_type(*j, MBQUAD, cub_faces);
      assert(MB_SUCCESS == result);
      if (MB_SUCCESS != result)
        return result;
      // Meshed volumes must have meshed surfaces
      if (cub_faces.empty())
      {
        std::cout << "    Surface " << surf_id << ": contains no meshed faces"
            << std::endl;
        // return MB_ENTITY_NOT_FOUND;
      }
      // get the faces common to both the skin and this surface
      Range common_faces = intersect(cub_faces, skin_faces);
      // find the surface faces not on the skin - these are old and need removed
      Range old_faces = subtract(cub_faces, common_faces);
      result = MBI->remove_entities(*j, old_faces);
      assert(MB_SUCCESS == result);
      if (MB_SUCCESS != result)
        return result;

      // remove the common faces from the skin faces
      skin_faces = subtract(skin_faces, common_faces);
      // If no old faces exist we are done
      if (old_faces.empty())
        continue;
      std::cout << "    Surface " << surf_id << ": " << old_faces.size()
          << " old faces removed" << std::endl;
      // Place the old faces in a new surface, because they may still be adjacent
      // to 3D mesh in another volume. Get the parent vols of the surface.
      Range cgm_surf;
      const Tag tags2[] = { idTag, dimTag };
      const void* const tag_vals2[] = { &surf_id, &two };
      result = MBI->get_entities_by_type_and_tag(cgm_file_set, MBENTITYSET,
          tags2, tag_vals2, 2, cgm_surf);
      assert(MB_SUCCESS == result);
      if (MB_SUCCESS != result)
        return result;
      if (1 != cgm_surf.size())
      {
        std::cout << "invalid size" << std::endl;
        return MB_INVALID_SIZE;
      }
      EntityHandle cgm_vols2[2], cub_vols[2];
      result = MBI->tag_get_data(senseTag, cgm_surf, &cgm_vols2);
      assert(MB_SUCCESS == result);
      if (MB_SUCCESS != result)
        return result;
      result = MBI->tag_get_data(senseTag, &(*j), 1, &cub_vols);
      assert(MB_SUCCESS == result);
      if (MB_SUCCESS != result)
        return result;
      // for the new surf, replace the current volume with the impl compl vol.
      // This is because the faces that no longer exist will become adjacent to
      // the impl compl
      if (*i == cgm_vols2[0])
      {
        cgm_vols2[0] = 0;
        cub_vols[0] = 0;
      }
      if (*i == cgm_vols2[1])
      {
        cgm_vols2[1] = 0;
        cub_vols[1] = 0;
      }
      // If both sides of the surface are the impl comp, do not create the surface.
      if (0 == cgm_vols2[0] && 0 == cgm_vols2[1])
      {
        std::cout
            << "    New surface was not created for old faces because both parents are impl_compl volume "
            << std::endl;
        continue;
      }
      // build the new surface.
      EntityHandle new_cgm_surf, new_cub_surf;
      ++max_surf_id;
      result = build_new_surface(MBI, new_cgm_surf, cgm_vols2[0], cgm_vols2[1],
          max_surf_id, dimTag, idTag, categoryTag, senseTag);
      assert(MB_SUCCESS == result);
      if (MB_SUCCESS != result)
        return result;
      result = build_new_surface(MBI, new_cub_surf, cub_vols[0], cub_vols[1],
          max_surf_id, dimTag, idTag, categoryTag, senseTag);
      assert(MB_SUCCESS == result);
      if (MB_SUCCESS != result)
        return result;
      // add the new surface to the file set and populate it with the old faces
      result = MBI->add_entities(cgm_file_set, &new_cgm_surf, 1);
      assert(MB_SUCCESS == result);
      if (MB_SUCCESS != result)
        return result;
      assert(MB_SUCCESS == result);
      result = MBI->add_entities(cub_file_set, &new_cub_surf, 1);
      if (MB_SUCCESS != result)
        return result;
      assert(MB_SUCCESS == result);
      result = MBI->add_entities(new_cub_surf, old_faces);
      assert(MB_SUCCESS == result);
      if (MB_SUCCESS != result)
        return result;

      std::cout << "    Surface " << max_surf_id << ": created for "
          << old_faces.size() << " old faces" << std::endl;
    }

    // the remaining skin faces are newly exposed faces
    Range new_faces = skin_faces;

    // new skin faces must be assigned to a surface
    if (new_faces.empty())
      continue;
    std::cout << "    Surface " << max_surf_id + 1 << ": created for "
        << new_faces.size() << " new faces" << std::endl;

    // Ensure that faces are oriented outwards
    result = orient_faces_outward(MBI, new_faces, debug);
    assert(MB_SUCCESS == result);
    if (MB_SUCCESS != result)
      return result;

    // Create the new surface.
    EntityHandle new_cgm_surf, new_cub_surf;
    ++max_surf_id;
    result = build_new_surface(MBI, new_cgm_surf, *i, 0, max_surf_id, dimTag,
        idTag, categoryTag, senseTag);
    assert(MB_SUCCESS == result);
    if (MB_SUCCESS != result)
      return result;
    result = build_new_surface(MBI, new_cub_surf, cub_vol.front(), 0,
        max_surf_id, dimTag, idTag, categoryTag, senseTag);
    assert(MB_SUCCESS == result);
    if (MB_SUCCESS != result)
      return result;
    // Insert the new surf into file sets and populate it with faces.
    result = MBI->add_entities(cgm_file_set, &new_cgm_surf, 1);
    assert(MB_SUCCESS == result);
    if (MB_SUCCESS != result)
      return result;
    result = MBI->add_entities(cub_file_set, &new_cub_surf, 1);
    assert(MB_SUCCESS == result);
    if (MB_SUCCESS != result)
      return result;
    result = MBI->add_entities(new_cub_surf, new_faces);
    assert(MB_SUCCESS == result);
    if (MB_SUCCESS != result)
      return result;
  }

  return MB_SUCCESS;
}

/* Get the type of a file.
 Return value is one of the above constants
 */
const char* get_geom_file_type(const char* filename);
const char* get_geom_fptr_type(FILE* file);

int is_cubit_file(FILE* file);
int is_step_file(FILE* file);
int is_iges_file(FILE* file);
int is_acis_txt_file(FILE* file);
int is_acis_bin_file(FILE* file);
int is_occ_brep_file(FILE* file);

double DEFAULT_DISTANCE = 0.001;
double DEFAULT_LEN = 0.0;
int DEFAULT_NORM = 5;

// load cub file 
// load cgm file
// for each surface
//   convert cub surf quads to tris
//   get signed volume from cgm and cub surf         MUST COME BEFORE COORD UPDATE, NEEDS MBTRIS
//   reverse cgm surface sense if needed
//   replace cgm surf tris with cub surf tris
// measure volume of predeformed cub elements
// convert cub volumes sets to tracking so that dead elems are removed from vol sets
// update coordinates and delete dead elems
// measure volume of deformed cub elems
// print histogram of volume change
// for each cub volume
//   skin volume elems to get faces
//   for each child cub surface
//     assign old skin faces to a new surface in case they are adjacent to another volume
//   orient each skin face outward
//   assign new skin faces to a new surface
// for each surface
//   remove existing tris (from before the update)
//   convert quads to tris
// remove empty surfaces and volumes due to dead elements
int main(int argc, char* argv[])
{
  clock_t start_time = clock();

  const bool debug = false;
  const char *file_type = NULL;

  const char* cub_name = 0;
  const char* exo_name = 0;
  const char* out_name = 0;
  const char* time_step = 0;
  const char* sat_name = 0;
  double dist_tol = 0.001, len_tol = 0.0;
  int norm_tol = 5;

  if (6 != argc && 9 != argc)
  {
    std::cerr << "To read meshed geometry for DagMC:" << std::endl;
    std::cerr
        << "$> <cub_file.cub> <acis_file.sat> <facet_tol> <output_file.h5m> conserve_mass<bool>"
        << std::endl;
    std::cerr
        << "To read meshed geometry for DagMC and update node coordinates:"
        << std::endl;
    std::cerr
        << "$> <cub_file.cub> <acis_file.sat> <facet_tol> <output_file.h5m> <deformed_exo_file.e> time_step<int> check_vol_change<bool> conserve_mass<bool>"
        << std::endl;
    exit(4);
  }

  // check filenames for proper suffix
  std::string temp;
  cub_name = argv[1];
  temp.assign(cub_name);
  if (std::string::npos == temp.find(".cub"))
  {
    std::cerr << "cub_file does not have correct suffix" << std::endl;
    return 1;
  }
  sat_name = argv[2]; // needed because the cub file's embedded sat file does not have groups
  temp.assign(sat_name);
  if (std::string::npos == temp.find(".sat"))
  {
    std::cerr << "sat_file does not have correct suffix" << std::endl;
    return 1;
  }
  out_name = argv[4];
  temp.assign(out_name);
  if (std::string::npos == temp.find(".h5m"))
  {
    std::cerr << "out_file does not have correct suffix" << std::endl;
    return 1;
  }

  // get the facet tolerance
  dist_tol = atof(argv[3]);
  if (0 > dist_tol || 1 < dist_tol)
  {
    std::cout << "error: facet_tolerance=" << dist_tol << std::endl;
    return 1;
  }

  // Should the nodes be updated?
  bool update_coords = false;
  if (9 == argc)
  {
    exo_name = argv[5];
    temp.assign(exo_name);
    if (std::string::npos == temp.find(".e"))
    {
      std::cerr << "e_file does not have correct suffix" << std::endl;
      return 1;
    }
    time_step = argv[6];
    update_coords = true;
  }

  // Should the volume change be determined?
  bool determine_volume_change = false;
  if (9 == argc)
  {
    temp.assign(argv[7]);
    if (std::string::npos != temp.find("true"))
      determine_volume_change = true;
  }

  // Should densities be changed to conserve mass?
  bool conserve_mass = false;
  if (9 == argc)
  {
    temp.assign(argv[8]);
    if (std::string::npos != temp.find("true"))
      conserve_mass = true;
  }
  else if (6 == argc)
  {
    temp.assign(argv[5]);
    if (std::string::npos != temp.find("true"))
      conserve_mass = true;
  }

  // Get CGM file type
  if (!file_type)
  {
    file_type = get_geom_file_type(cub_name);
    if (!file_type)
    {
      std::cerr << cub_name << " : unknown file type, try '-t'" << std::endl;
      exit(1);
    }
  }

  // Read the mesh from the cub file with Tqcdfr 
  Core *MBI = new Core();
  ErrorCode result;
  EntityHandle cub_file_set;
  result = MBI->create_meshset(0, cub_file_set);
  if (MB_SUCCESS != result)
    return result;
  // Do not ignore the Cubit file version. In testing, a cub file from Cubit12 
  // did not work.
  //char cub_options[256] = "120";
  //char cub_options[256] = "IGNORE_VERSION";
  //result = MBI->load_file(cub_name, &cub_file_set, cub_options, NULL, 0, 0);
  result = MBI->load_file(cub_name, &cub_file_set, 0, NULL, 0, 0);
  if (MB_SUCCESS != result)
  {
    std::cout << "error: problem reading cub file" << std::endl;
    return result;
  }
  std::cout << "Mesh file read." << std::endl;

  // Read the ACIS file with ReadCGM
  char cgm_options[256];
  std::cout << "  facet tolerance=" << dist_tol << std::endl;
  sprintf(cgm_options,
      "CGM_ATTRIBS=yes;FACET_DISTANCE_TOLERANCE=%g;FACET_NORMAL_TOLERANCE=%d;MAX_FACET_EDGE_LENGTH=%g;",
      dist_tol, norm_tol, len_tol);
  EntityHandle cgm_file_set;
  result = MBI->create_meshset(0, cgm_file_set);
  if (MB_SUCCESS != result)
    return result;
  result = MBI->load_file(sat_name, &cgm_file_set, cgm_options, NULL, 0, 0);
  if (MB_SUCCESS != result)
  {
    std::cout << "error: problem reading sat file" << std::endl;
    return result;
  }
  std::cout << "CAD file read." << std::endl;

  // Create tags
  Tag dimTag, idTag, categoryTag, senseTag, sizeTag, nameTag;
  result = MBI->tag_get_handle(GEOM_DIMENSION_TAG_NAME, 1, MB_TYPE_INTEGER,
      dimTag, MB_TAG_SPARSE | MB_TAG_CREAT);
  if (MB_SUCCESS != result)
    return result;
  result = MBI->tag_get_handle(GLOBAL_ID_TAG_NAME, 1, MB_TYPE_INTEGER, idTag,
      MB_TAG_DENSE | MB_TAG_CREAT);
  if (MB_SUCCESS != result)
    return result;
  result = MBI->tag_get_handle(CATEGORY_TAG_NAME, CATEGORY_TAG_SIZE,
      MB_TYPE_OPAQUE, categoryTag, MB_TAG_SPARSE | MB_TAG_CREAT);
  if (MB_SUCCESS != result)
    return result;
  result = MBI->tag_get_handle("GEOM_SENSE_2", 2, MB_TYPE_HANDLE, senseTag,
      MB_TAG_SPARSE | MB_TAG_CREAT);
  if (MB_SUCCESS != result)
    return result;
  result = MBI->tag_get_handle("GEOM_SIZE", 1, MB_TYPE_DOUBLE, sizeTag,
      MB_TAG_DENSE | MB_TAG_CREAT);
  if (MB_SUCCESS != result)
    return result;
  result = MBI->tag_get_handle(NAME_TAG_NAME, NAME_TAG_SIZE, MB_TYPE_OPAQUE,
      nameTag, MB_TAG_SPARSE | MB_TAG_CREAT);
  if (MB_SUCCESS != result)
    return result;

  // Create triangles from the quads of the cub surface sets and add them to the
  // cub surface sets. Get the signed volume of each surface for both cgm and 
  // cub representations. Change the sense of the cgm representation to match 
  // the cub representation.
  result = fix_surface_senses(MBI, cgm_file_set, cub_file_set, idTag, dimTag,
      senseTag, debug);
  if (MB_SUCCESS != result)
    return result;
  std::cout << "Fixed CAD surface senses to match meshed surface senses."
      << std::endl;

  // Get the 3D elements in the cub file and measure their volume.
  Range orig_elems;
  std::vector<double> orig_size;
  if (determine_volume_change)
  {
    result = MBI->get_entities_by_dimension(0, 3, orig_elems);
    if (MB_SUCCESS != result)
      return result;
    orig_size.resize(orig_elems.size());
    for (unsigned int i = 0; i < orig_elems.size(); ++i)
    {
      orig_size[i] = measure(MBI, orig_elems[i]);
    }
  }

  // Before updating the nodes and removing dead elements, force the cub volume
  // sets to track ownership so that dead elements will be deleted from the sets.
  const int three = 3;
  const void* const three_val[] = { &three };
  Range cub_vols;
  result = MBI->get_entities_by_type_and_tag(cub_file_set, MBENTITYSET, &dimTag,
      three_val, 1, cub_vols);
  if (MB_SUCCESS != result)
    return result;
  for (Range::const_iterator i = cub_vols.begin(); i != cub_vols.end(); ++i)
  {
    result = MBI->set_meshset_options(*i, MESHSET_TRACK_OWNER);
    if (MB_SUCCESS != result)
      return result;
  }

  // Tag volume sets with the undeformed size of each volume.
  Range vol_sets;
  result = MBI->get_entities_by_type_and_tag(cgm_file_set, MBENTITYSET, &dimTag,
      three_val, 1, vol_sets);
  if (MB_SUCCESS != result)
    return result;
  for (Range::const_iterator i = vol_sets.begin(); i != vol_sets.end(); ++i)
  {
    double size;
    moab::DagMC &dagmc = *moab::DagMC::instance(MBI);
    result = dagmc.measure_volume(*i, size);
    if (MB_SUCCESS != result)
      return result;
    result = MBI->tag_set_data(sizeTag, &(*i), 1, &size);
    if (MB_SUCCESS != result)
      return result;
  }

  // Update the coordinates if needed. Do not do this before checking surface
  // sense, because the coordinate update could deform the surfaces too much
  // to make an accurate comparison.
  // The cub node ids are unique because cgm vertex ids are tagged on the vertex
  // meshset and not the vertex itself.
  //result = MBI->delete_entities( &cub_file_set, 1 );
  //if(MB_SUCCESS != result) return result;
  // Assume dead elements exist until I think of something better.
  bool dead_elements_exist = true;
  if (update_coords)
  {
    //ReadNCDF my_ex_reader(MBI);
    char exo_options[120] = "tdata=coord,";
    strcat(exo_options, time_step);
    strcat(exo_options, ",set");
    //FileOptions exo_opts(exo_options)  ;
    //opts = "tdata=coord, 100, sum, temp.exo";
    //result =  my_ex_reader.load_file(exo_name, cgm_file_set, exo_opts, NULL, 0 , 0);
    //result =  my_ex_reader.load_file(exo_name, cub_file_set, exo_opts, NULL, 0 , 0);
    //result = my_ex_reader.load_file(exo_name, &cub_file_set, exo_opts, NULL, 0 , 0);
    MBI->load_file(exo_name, &cub_file_set, exo_options);
    if (MB_SUCCESS != result)
    {
      std::string last_error;
      MBI->get_last_error(last_error);
      std::cout << "coordinate update failed, " << last_error << std::endl;
      return result;
    }
    std::cout
        << "Updated mesh nodes with deformed coordinates from exodus file."
        << std::endl;
  }

  if (determine_volume_change)
  {
    // Dead elements have been removed by the deformation. Get the elements that 
    // still exist.
    Range defo_elems;
    result = MBI->get_entities_by_dimension(0, 3, defo_elems);
    if (MB_SUCCESS != result)
      return result;

    // Determine the volume of the elements now that a deformation has been
    // applied. Condense the original array by removing dead elements.
    double *orig_size_condensed = new double[defo_elems.size()];
    double *defo_size_condensed = new double[defo_elems.size()];
    int j = 0;
    for (unsigned int i = 0; i < orig_elems.size(); ++i)
    {
      if (orig_elems[i] == defo_elems[j])
      {
        orig_size_condensed[j] = orig_size[i];
        defo_size_condensed[j] = measure(MBI, defo_elems[j]);
        ++j;
      }
    }
    generate_plots(orig_size_condensed, defo_size_condensed, defo_elems.size(),
        std::string(time_step));
    delete[] orig_size_condensed; // can't use the same delete[] for both
    delete[] defo_size_condensed;
  }

  // Deal with dead elements. For now, add them to the impl_compl volume.
  // Extra surfaces are created to do this.
  if (update_coords && dead_elements_exist)
  {
    result = add_dead_elems_to_impl_compl(MBI, cgm_file_set, cub_file_set,
        idTag, dimTag, categoryTag, senseTag, debug);
    if (MB_SUCCESS != result)
      return result;
    std::cout
        << "Placed dead elements in implicit complement volume and added required surfaces."
        << std::endl;
  }

  // The quads in the cub_file_set have been updated for dead elements. For each
  // cgm_surf, if there exists a cub_surf with the same id, replace the cgm tris
  // with cub_tris (created from the quads). Note the a surface that is not 
  // meshed (in cub file) will not be effected.
  result = replace_faceted_cgm_surfs(MBI, cgm_file_set, cub_file_set, idTag,
      dimTag, debug);
  if (MB_SUCCESS != result)
    return result;
  std::cout
      << "Replaced faceted CAD surfaces with meshed surfaces of triangles."
      << std::endl;

  result = remove_empty_cgm_surfs_and_vols(MBI, cgm_file_set, idTag, dimTag,
      debug);
  if (MB_SUCCESS != result)
    return result;
  std::cout << "Removed surfaces and volumes that no longer have any triangles."
      << std::endl;

  // For each material, sum the volume. If the coordinates were updated for 
  // deformation, summarize the volume change.
  result = summarize_cell_volume_change(MBI, cgm_file_set, categoryTag, dimTag,
      sizeTag, nameTag, idTag, conserve_mass, debug);
  if (MB_SUCCESS != result)
    return result;
  std::cout
      << "Summarized the volume change of each material, with respect to the solid model."
      << std::endl;

  result = MBI->write_mesh(out_name, &cgm_file_set, 1);
  if (MB_SUCCESS != result)
  {
    std::cout << "write mesh failed" << std::endl;
    return result;
  }
  std::cout << "Saved output file for mesh-based analysis." << std::endl;

  clock_t end_time = clock();
  std::cout << "  " << (double) (end_time - start_time) / CLOCKS_PER_SEC
      << " seconds" << std::endl;
  std::cout << std::endl;

  return 0;
}

const char* get_geom_file_type(const char* name)
{
  FILE* file;
  const char* result = 0;

  file = fopen(name, "r");
  if (file)
  {
    result = get_geom_fptr_type(file);
    fclose(file);
  }

  return result;
}

const char* get_geom_fptr_type(FILE* file)
{
  static const char* CUBIT_NAME = GF_CUBIT_FILE_TYPE;
  static const char* STEP_NAME = GF_STEP_FILE_TYPE;
  static const char* IGES_NAME = GF_IGES_FILE_TYPE;
  static const char* SAT_NAME = GF_ACIS_TXT_FILE_TYPE;
  static const char* SAB_NAME = GF_ACIS_BIN_FILE_TYPE;
  static const char* BREP_NAME = GF_OCC_BREP_FILE_TYPE;

  if (is_cubit_file(file))
    return CUBIT_NAME;
  else if (is_step_file(file))
    return STEP_NAME;
  else if (is_iges_file(file))
    return IGES_NAME;
  else if (is_acis_bin_file(file))
    return SAB_NAME;
  else if (is_acis_txt_file(file))
    return SAT_NAME;
  else if (is_occ_brep_file(file))
    return BREP_NAME;
  else
    return 0;
}

int is_cubit_file(FILE* file)
{
  unsigned char buffer[4];
  return !fseek(file, 0, SEEK_SET) && fread(buffer, 4, 1, file)
      && !memcmp(buffer, "CUBE", 4);
}

int is_step_file(FILE* file)
{
  unsigned char buffer[9];
  return !fseek(file, 0, SEEK_SET) && fread(buffer, 9, 1, file)
      && !memcmp(buffer, "ISO-10303", 9);
}

int is_iges_file(FILE* file)
{
  unsigned char buffer[10];
  return !fseek(file, 72, SEEK_SET) && fread(buffer, 10, 1, file)
      && !memcmp(buffer, "S      1\r\n", 10);
}

int is_acis_bin_file(FILE* file)
{
  char buffer[15];
  return !fseek(file, 0, SEEK_SET) && fread(buffer, 15, 1, file)
      && !memcmp(buffer, "ACIS BinaryFile", 9);
}

int is_acis_txt_file(FILE* file)
{
  char buffer[5];
  int version, length;

  if (fseek(file, 0, SEEK_SET)
      || 2 != fscanf(file, "%d %*d %*d %*d %d ", &version, &length))
    return 0;

  if (version < 1 || version > 0xFFFF)
    return 0;

  // Skip appliation name
  if (fseek(file, length, SEEK_CUR))
    return 0;

  // Read length of version string followed by first 5 characters
  if (2 != fscanf(file, "%d %4s", &length, buffer))
    return 0;

  return !strcmp(buffer, "ACIS");
}

int is_occ_brep_file(FILE* file)
{
  unsigned char buffer[6];
  return !fseek(file, 0, SEEK_SET) && fread(buffer, 6, 1, file)
      && !memcmp(buffer, "DBRep_", 6);
}
