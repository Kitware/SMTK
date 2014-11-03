//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================


#ifndef __smtk_mesh_moab_Functors_h
#define __smtk_mesh_moab_Functors_h

#include "Interface.h"

namespace smtk {
namespace mesh {
namespace moab {
namespace functors {

struct PartiallyContained
{
  bool operator()(const smtk::mesh::HandleRange& points,
                  const ::moab::EntityHandle* connectivity,
                  const std::size_t num_nodes) const
  {
  bool contains = false;
  for(int j=0; j < num_nodes && contains == false; ++j)
    {
    contains = (points.find(connectivity[j]) != points.end());
    }
  return contains;
  }
};

struct FullyContained
{
  bool operator()(const smtk::mesh::HandleRange& points,
                  const ::moab::EntityHandle* connectivity,
                  const std::size_t num_nodes) const
  {
  bool contains = true;
  for(int j=0; j < num_nodes && contains == true; ++j)
    {
    contains = (points.find(connectivity[j]) != points.end());
    }
  return contains;
  }
};

}
}
}
}

#endif
