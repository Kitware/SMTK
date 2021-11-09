//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_io_MeshIO_h
#define smtk_io_MeshIO_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h"

#include "smtk/io/mesh/Format.h"

#include <memory>

namespace smtk
{
namespace io
{
/// Mesh IO
namespace mesh
{

/// Mesh subset types
enum class Subset : unsigned int
{
  EntireResource,
  OnlyDomain,
  OnlyDirichlet,
  OnlyNeumann,
};

/**\brief Base class for mesh IO types
  *
  * Rather than make the interface for this class pure virtual, the methods
  * default to doing nothing and are overridden in the derived classes. The
  * possible actions are then recorded in the mesh Format, which can be queried
  * at runtime.
  */

class MeshIO;
typedef std::unique_ptr<MeshIO> MeshIOPtr;

class SMTKCORE_EXPORT MeshIO
{
public:
  virtual ~MeshIO() = default;

  virtual smtk::mesh::ResourcePtr
  importMesh(const std::string&, const smtk::mesh::InterfacePtr&, const std::string&) const
  {
    return smtk::mesh::ResourcePtr();
  }
  virtual bool importMesh(const std::string&, smtk::mesh::ResourcePtr, const std::string&) const
  {
    return false;
  }

  virtual bool exportMesh(const std::string&, smtk::mesh::ResourcePtr) const { return false; }
  virtual bool exportMesh(
    const std::string&,
    smtk::mesh::ResourcePtr,
    smtk::model::ResourcePtr,
    const std::string&) const
  {
    return false;
  }

  virtual smtk::mesh::ResourcePtr read(const std::string&, const smtk::mesh::InterfacePtr&, Subset)
    const
  {
    return smtk::mesh::ResourcePtr();
  }
  virtual bool read(const std::string&, smtk::mesh::ResourcePtr, Subset) const { return false; }

  virtual bool write(const std::string&, smtk::mesh::ResourcePtr, Subset) const { return false; }
  virtual bool write(smtk::mesh::ResourcePtr, Subset) const { return false; }

  const std::vector<smtk::io::mesh::Format>& FileFormats() const { return this->Formats; }

protected:
  std::vector<smtk::io::mesh::Format> Formats;
};
} // namespace mesh
} // namespace io
} // namespace smtk

#endif
