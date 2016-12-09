//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_ExtractField_h
#define __smtk_mesh_ExtractField_h

#include <boost/cstdint.hpp>

#include "smtk/mesh/CellSet.h"
#include "smtk/mesh/MeshSet.h"

namespace smtk {
namespace model { class EntityRef; class Loop; }
namespace mesh {

class SMTKCORE_EXPORT PreAllocatedField
{

public:
  static void determineAllocationLengths(const smtk::mesh::MeshSet& ms,
                                         boost::int64_t& numberOfCells,
                                         boost::int64_t& numberOfPoints);

  PreAllocatedField( boost::int64_t* cellField,
                     boost::int64_t* pointField );

private:
template <typename QueryTag>
friend SMTKCORE_EXPORT void extractField( const smtk::mesh::MeshSet&,
                                          const smtk::mesh::PointSet&,
                                          PreAllocatedField& );

  boost::int64_t* m_cellField;
  boost::int64_t* m_pointField;
};

class SMTKCORE_EXPORT Field
{
public:
  Field() {}

  //This class self allocates all the memory needed to extract tessellation
  //and auto extract the tessellation based on the MeshSet you pass in
  void extractDirichlet( const smtk::mesh::MeshSet& ms );
  void extractNeumann( const smtk::mesh::MeshSet& ms );
  void extractDomain( const smtk::mesh::MeshSet& ms );

  void extractDirichlet( const smtk::mesh::MeshSet& cs,
                         const smtk::mesh::PointSet& ps );
  void extractNeumann( const smtk::mesh::MeshSet& cs,
                       const smtk::mesh::PointSet& ps );
  void extractDomain( const smtk::mesh::MeshSet& cs,
                      const smtk::mesh::PointSet& ps );

  //use these methods to gain access to the field after extraction
  const std::vector<boost::int64_t>& cellData() const
  { return this->m_cellData; }
  const std::vector<boost::int64_t>& pointData() const
  { return this->m_pointData; }

private:
  template <typename QueryTag>
    void extract( const smtk::mesh::MeshSet& ms,
                  const smtk::mesh::PointSet& ps );

  std::vector<boost::int64_t> m_cellData;
  std::vector<boost::int64_t> m_pointData;
};

//Don't wrap these for python, instead python should use the Field class and
//the extract method
#ifndef SHIBOKEN_SKIP

SMTKCORE_EXPORT void extractDirichletField( const smtk::mesh::MeshSet&,
                                            PreAllocatedField& );
SMTKCORE_EXPORT void extractNeumannField( const smtk::mesh::MeshSet&,
                                          PreAllocatedField& );
SMTKCORE_EXPORT void extractDomainField( const smtk::mesh::MeshSet&,
                                         PreAllocatedField& );

//Extract Field with respect to another PointSet instead of the PointSet
//contained by the meshset. This is useful if you are sharing a single
//PointSet among multiple Fields.
SMTKCORE_EXPORT void extractDirichletField( const smtk::mesh::MeshSet&,
                                            const smtk::mesh::PointSet&,
                                            PreAllocatedField& );
SMTKCORE_EXPORT void extractNeumannField( const smtk::mesh::MeshSet&,
                                          const smtk::mesh::PointSet&,
                                          PreAllocatedField& );
SMTKCORE_EXPORT void extractDomainField( const smtk::mesh::MeshSet&,
                                         const smtk::mesh::PointSet&,
                                         PreAllocatedField& );

template <typename QueryTag>
SMTKCORE_EXPORT void extractField( const smtk::mesh::MeshSet&,
                                   const smtk::mesh::PointSet&,
                                   PreAllocatedField& );


#endif //SHIBOKEN_SKIP

}
}

#endif
