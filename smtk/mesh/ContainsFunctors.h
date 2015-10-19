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


#ifndef __smtk_mesh_ContainsFunctors_h
#define __smtk_mesh_ContainsFunctors_h

#include "smtk/mesh/Handle.h"

namespace smtk {
namespace mesh {

//these aren't exported as they are private class that only
//smtk::mesh should call ( currently )

class ContainsFunctor
{
public:
  virtual bool operator()(const smtk::mesh::HandleRange& points,
                          const smtk::mesh::Handle* connectivity,
                          const std::size_t num_nodes) const = 0;
};

class PartiallyContainedFunctor : public ContainsFunctor
{
public:
  bool operator()(const smtk::mesh::HandleRange& points,
                  const smtk::mesh::Handle* connectivity,
                  const std::size_t num_nodes) const
  {
  bool contains = false;
  for (std::size_t j = 0; j < num_nodes && contains == false; ++j)
    {
    contains = (points.find(connectivity[j]) != points.end());
    }
  return contains;
  }
};

class FullyContainedFunctor : public ContainsFunctor
{
public:
  bool operator()(const smtk::mesh::HandleRange& points,
                  const smtk::mesh::Handle* connectivity,
                  const std::size_t num_nodes) const
  {
  bool contains = true;
  for (std::size_t j = 0; j < num_nodes && contains == true; ++j)
    {
    contains = (points.find(connectivity[j]) != points.end());
    }
  return contains;
  }
};

}
}

#endif
