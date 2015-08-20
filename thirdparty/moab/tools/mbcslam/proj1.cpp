/*
 * proj1.cpp
 *
 *  project on a sphere of radius R, delete sets if needed, and delete edges between parts
 *  (created by resolve shared ents)
 */

#include "moab/Core.hpp"
#include "moab/Interface.hpp"
#include <iostream>
#include <math.h>

#include "CslamUtils.hpp"
#include <assert.h>
using namespace moab;

double radius = 1.;// in m:  6371220.

int main(int argc, char **argv)
{

  bool delete_partition_sets = false;

  if (argc < 3)
    return 1;

  int index = 1;
  char * input_mesh1 = argv[1];
  char * output = argv[2];
  while (index < argc)
  {
    if (!strcmp(argv[index], "-R")) // this is for radius to project
    {
      radius = atof(argv[++index]);
    }
    if (!strcmp(argv[index], "-DS")) // delete partition sets
    {
      delete_partition_sets = true;
    }

    if (!strcmp(argv[index], "-h"))
    {
      std::cout << " usage: proj1 <input> <output> -R <value>  -DS (delete partition sets)\n";
      return 1;
    }
    index++;
  }

  Core moab;
  Interface & mb = moab;

  ErrorCode rval;

  rval = mb.load_mesh(input_mesh1);

  std::cout  << " -R " << radius << " input: " << input_mesh1 <<
      "  output: " << output << "\n";

  Range verts;
  rval = mb.get_entities_by_dimension(0, 0, verts);
  if (MB_SUCCESS != rval)
    return 1;

  double *x_ptr, *y_ptr, *z_ptr;
  int count;
  rval = mb.coords_iterate(verts.begin(), verts.end(), x_ptr, y_ptr, z_ptr, count);
  if (MB_SUCCESS != rval)
      return 1;
  assert(count == (int) verts.size()); // should end up with just one contiguous chunk of vertices

  for (int v = 0; v < count; v++) {
     //EntityHandle v = verts[v];
     CartVect pos( x_ptr[v], y_ptr[v] , z_ptr[v]);
     pos = pos/pos.length();
     pos = radius*pos;
     x_ptr[v] = pos[0];
     y_ptr[v] = pos[1];
     z_ptr[v] = pos[2];
  }

  Range edges;
  rval = mb.get_entities_by_dimension(0, 1, edges);
  if (MB_SUCCESS != rval)
    return 1;
  // write edges to a new set, and after that, write the set, delete the edges and the set
  EntityHandle sf1;
  rval = mb.create_meshset(MESHSET_SET, sf1);
  if (MB_SUCCESS != rval)
    return 1;
  rval = mb.add_entities(sf1, edges);
  if (MB_SUCCESS != rval)
    return 1;
  rval = mb.write_mesh("edgesOnly.h5m", &sf1, 1);
  if (MB_SUCCESS != rval)
    return 1;
  rval = mb.delete_entities(&sf1, 1);
  if (MB_SUCCESS != rval)
    return 1;
  mb.delete_entities(edges);
  mb.write_file(output);

  if (delete_partition_sets)
  {
    Tag par_tag;
    rval = mb.tag_get_handle("PARALLEL_PARTITION", par_tag);
    if (MB_SUCCESS == rval)

    {
      Range par_sets;
      rval =  mb.get_entities_by_type_and_tag(0, MBENTITYSET, &par_tag, NULL, 1, par_sets,
         moab::Interface::UNION);
      if (!par_sets.empty())
        mb.delete_entities(par_sets);
      mb.tag_delete(par_tag);
    }
  }


  return 0;
}
