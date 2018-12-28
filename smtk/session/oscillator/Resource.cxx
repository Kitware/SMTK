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
#include "smtk/session/oscillator/Resource.h"

#include "smtk/model/Volume.h"

namespace smtk
{
namespace session
{
namespace oscillator
{

Resource::Resource(const smtk::common::UUID& id, resource::Manager::Ptr manager)
  : smtk::resource::DerivedFrom<Resource, smtk::model::Resource>(id, manager)
{
}

Resource::Resource(resource::Manager::Ptr manager)
  : smtk::resource::DerivedFrom<Resource, smtk::model::Resource>(manager)
{
}

bool Resource::resetDomainTessellation(smtk::model::Volume& domain)
{
  auto origin = domain.floatProperty("origin");
  auto size = domain.floatProperty("size");
  int dimension = static_cast<int>(origin.size());
  if (dimension < 2 || dimension > 3)
  {
    return false;
  }

  if (static_cast<int>(size.size()) != dimension)
  {
    return false;
  }

  // Use the operation info to create or replace the tessellation.
  auto tess = domain.resetTessellation();
  double corner[3] = { 0, 0, 0 };
  for (int ii = 0; ii < (1 << dimension); ++ii)
  {
    for (int cc = 0; cc < dimension; ++cc)
    {
      corner[cc] = origin[cc] + ((ii & (1 << cc)) ? size[cc] : 0.0);
    }
    tess->addCoords(corner);
  }
  constexpr int edgeEndpoints[12][2] = { { 0, 1 }, { 1, 3 }, { 3, 2 }, { 2, 0 },

    { 4, 5 }, { 5, 7 }, { 7, 6 }, { 6, 4 },

    { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 } };
  int numEdges = (dimension == 2 ? 4 : 12);
  for (int ee = 0; ee < numEdges; ++ee)
  {
    tess->addLine(edgeEndpoints[ee][0], edgeEndpoints[ee][1]);
  }
  return true;
}
}
}
}
