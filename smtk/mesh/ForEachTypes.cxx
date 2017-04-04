//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/ForEachTypes.h"

namespace smtk {
  namespace mesh {



MeshForEach::~MeshForEach()
{

}

CellForEach::CellForEach(bool wantCoordinates):
  m_pointIds(NULL),
  m_coords(NULL),
  m_wantsCoordinates(wantCoordinates)
{

}

CellForEach::~CellForEach()
{

}

PointForEach::~PointForEach()
{

}

  } // namespace mesh
} // namespace smtk
