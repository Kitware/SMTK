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
#ifndef smtk_extension_delaunay_io_ImportDelaunayMesh_h
#define smtk_extension_delaunay_io_ImportDelaunayMesh_h

#include "smtk/extension/delaunay/Exports.h"
//forward declarers for Manager and Meshresource
#include "smtk/PublicPointerDefs.h"

namespace Delaunay
{
namespace Mesh
{
class Mesh;
}
} // namespace Delaunay

namespace smtk
{
namespace mesh
{
class MeshSet;
}
} // namespace smtk

namespace smtk
{
namespace model
{
class EntityRef;
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

/**\brief Import a Delaunay mesh into smtk.
  *
  * This functor converts Delaunay Meshes into smtk::mesh::MeshSets or into
  * smtk::model::Tessellations.
  */
class SMTKDELAUNAYEXT_EXPORT ImportDelaunayMesh
{
public:
  ImportDelaunayMesh() = default;
  ImportDelaunayMesh(const ImportDelaunayMesh&) = delete;
  ImportDelaunayMesh& operator=(const ImportDelaunayMesh&) = delete;

  //Import a Delaunay mesh into an existing meshresource.
  smtk::mesh::MeshSet operator()(const Delaunay::Mesh::Mesh&, smtk::mesh::ResourcePtr) const;

  //Import a Delaunay mesh as a tessellation for an entity.
  bool operator()(const Delaunay::Mesh::Mesh&, smtk::model::EntityRef&) const;
};
} // namespace io
} // namespace delaunay
} // namespace extension
} // namespace smtk

#endif
