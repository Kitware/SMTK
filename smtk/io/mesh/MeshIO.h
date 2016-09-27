//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_io_MeshIO_h
#define __smtk_io_MeshIO_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h"

#include "smtk/io/mesh/Format.h"

namespace smtk {
  namespace io {
namespace mesh {

enum class Subset: unsigned int
  {
  EntireCollection,
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

class SMTKCORE_EXPORT MeshIO
{
public:
  virtual ~MeshIO() {}

  virtual smtk::mesh::CollectionPtr
    importMesh( const std::string&,
                smtk::mesh::ManagerPtr& ) const
  { return smtk::mesh::CollectionPtr(); }
  virtual bool
    importMesh( const std::string&,
                smtk::mesh::CollectionPtr ) const { return false; }

  virtual bool exportMesh( const std::string&,
                           smtk::mesh::CollectionPtr ) const { return false; }
  virtual bool exportMesh( const std::string&,
                           smtk::mesh::CollectionPtr,
                           smtk::model::ManagerPtr,
                           const std::string& ) const { return false; }

  virtual smtk::mesh::CollectionPtr
    read( const std::string&,
          smtk::mesh::ManagerPtr&,
          Subset) const { return smtk::mesh::CollectionPtr(); }
  virtual bool
    read( const std::string&,
          smtk::mesh::CollectionPtr,
          Subset ) const { return false; }

  virtual bool write( const std::string&,
                      smtk::mesh::CollectionPtr,
                      Subset ) const { return false; }
  virtual bool write( smtk::mesh::CollectionPtr,
                      Subset ) const { return false; }

  const std::vector<smtk::io::mesh::Format>& FileFormats() const
    {
      return this->Formats;
    }

protected:
  std::vector<smtk::io::mesh::Format> Formats;
};

}
}
}

#endif
