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
#ifndef __smtk_extension_delaunay_io_ExportDelaunayMesh_h
#define __smtk_extension_delaunay_io_ExportDelaunayMesh_h

#include "smtk/extension/delaunay/Exports.h"
//forward declarers for Manager and Resource
#include "smtk/PublicPointerDefs.h"

namespace Delaunay
{
namespace Shape
{
class Point;
}
} // namespace Delaunay

namespace smtk
{
namespace model
{
class Loop;
}
} // namespace smtk

namespace smtk
{
namespace extension
{
namespace delaunay
{
namespace io
{

/**\brief Export from smtk into a Delaunay mesh
  *
  */
class SMTKDELAUNAYEXT_EXPORT ExportDelaunayMesh
{
public:
  ExportDelaunayMesh() = default;
  ExportDelaunayMesh(const ExportDelaunayMesh&) = delete;
  ExportDelaunayMesh& operator=(const ExportDelaunayMesh&) = delete;

  //Export a model loop with a tessellation into a vector of Delaunay points.
  std::vector<Delaunay::Shape::Point> operator()(const smtk::model::Loop&) const;

  //Export a model loop with a mesh representation stored in the given
  // resource into a vector of Delaunay points.
  std::vector<Delaunay::Shape::Point> operator()(const smtk::model::Loop&, smtk::mesh::ResourcePtr&)
    const;
};
} // namespace io
} // namespace delaunay
} // namespace extension
} // namespace smtk

#endif
