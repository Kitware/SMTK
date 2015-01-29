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

#include "moab/Interface.hpp"

namespace smtk {
namespace mesh {
namespace moab {

//these aren't exported as they are private class that only
//smtk::mesh should call ( currently )

struct ContainsFunctor
{
  virtual bool operator()(const smtk::mesh::HandleRange& points,
                          const ::moab::EntityHandle* connectivity,
                          const std::size_t num_nodes) const = 0;
};

struct PartiallyContained : public ContainsFunctor
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

struct FullyContained : public ContainsFunctor
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

#endif
