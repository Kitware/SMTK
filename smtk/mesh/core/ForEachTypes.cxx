//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/core/ForEachTypes.h"

namespace smtk
{
namespace mesh
{

MeshForEach::~MeshForEach() = default;

CellForEach::CellForEach(bool wantCoordinates)
  : m_pointIds(nullptr)
  , m_coords(nullptr)
  , m_wantsCoordinates(wantCoordinates)
{
}

CellForEach::~CellForEach() = default;

PointForEach::~PointForEach() = default;

} // namespace mesh
} // namespace smtk
