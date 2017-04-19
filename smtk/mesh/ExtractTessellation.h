//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_ExtractTessellation_h
#define __smtk_mesh_ExtractTessellation_h

#include <cstdint>

#include "smtk/mesh/CellSet.h"
#include "smtk/mesh/MeshSet.h"

namespace smtk {
namespace model { class EntityRef; class Loop; }
namespace mesh {

class SMTKCORE_EXPORT PreAllocatedTessellation
{
public:
  // Todo: Document that connectivityLength is just pure length of connectivity
  // if you are enabling vtk length you need to allocate for connectivityLength
  // + numberOfCells
  static void determineAllocationLengths(const smtk::mesh::MeshSet& ms,
                                         std::int64_t& connectivityLength,
                                         std::int64_t& numberOfCells,
                                         std::int64_t& numberOfPoints);

  static void determineAllocationLengths(const smtk::mesh::CellSet& cs,
                                         std::int64_t& connectivityLength,
                                         std::int64_t& numberOfCells,
                                         std::int64_t& numberOfPoints);

  static void determineAllocationLengths(const smtk::model::EntityRef& eRef,
                                         const smtk::mesh::CollectionPtr& c,
                                         std::int64_t& connectivityLength,
                                         std::int64_t& numberOfCells,
                                         std::int64_t& numberOfPoints);

  static void determineAllocationLengths(const smtk::model::Loop& loop,
                                         const smtk::mesh::CollectionPtr& c,
                                         std::int64_t& connectivityLength,
                                         std::int64_t& numberOfCells,
                                         std::int64_t& numberOfPoints);

  //Only converts connectivity. The following properties will not be
  //converted: cellLocations, cellTypes, and Points
  PreAllocatedTessellation(  std::int64_t* connectivity );

  //Converts connectivity and store the points as floats. The following properties will not be
  //converted: cellLocations, and cellTypes.
  PreAllocatedTessellation(  std::int64_t* connectivity,
                             float* points);

  //Converts connectivityand store the points as doubles. The following properties will not be
  //converted: cellLocations, and cellTypes.
  PreAllocatedTessellation(  std::int64_t* connectivity,
                             double* points);

  //Converts everything but Points.
  PreAllocatedTessellation(  std::int64_t* connectivity,
                             std::int64_t* cellLocations,
                             unsigned char* cellTypes);

  //Converts everything and stores the points as floats
  PreAllocatedTessellation(  std::int64_t* connectivity,
                             std::int64_t* cellLocations,
                             unsigned char* cellTypes,
                             float* points);

  //Converts everything and stores the points as doubles
  PreAllocatedTessellation(  std::int64_t* connectivity,
                             std::int64_t* cellLocations,
                             unsigned char* cellTypes,
                             double* points);

  //determine if you want VTK 6.0 style connectivity array where each cell
  //is preceded with an entry that states the length of the cell. Passing
  //in True will disable this behavior. The default behavior of the
  //class is to use VTK style connectivity.
  void disableVTKStyleConnectivity(bool disable) { m_useVTKConnectivity  = !disable; }

  //determine if you want VTK cell enum values for the cell types array.
  //If this is disabled we use the smtk/mesh cell enum values.
  void disableVTKCellTypes(bool disable) { m_useVTKCellTypes  = !disable; }

  bool hasConnectivity() const { return this->m_connectivity != NULL; }
  bool hasCellLocations() const { return this->m_cellLocations != NULL; }
  bool hasCellTypes() const { return this->m_cellTypes != NULL; }

  bool hasDoublePoints() const { return this->m_dpoints != NULL; }
  bool hasFloatPoints() const { return this->m_fpoints != NULL; }

  bool useVTKConnectivity() const { return this->m_useVTKConnectivity; }
  bool useVTKCellTypes() const { return this->m_useVTKCellTypes; }


private:
  template <class PointConnectivity>
  friend SMTKCORE_EXPORT void
    extractTessellationInternal( PointConnectivity&,
                                 const smtk::mesh::PointSet&,
                                 PreAllocatedTessellation& );
  std::int64_t* m_connectivity;
  std::int64_t* m_cellLocations;
  unsigned char* m_cellTypes;

  double* m_dpoints;
  float* m_fpoints;

  bool m_useVTKConnectivity;
  bool m_useVTKCellTypes;
};

class SMTKCORE_EXPORT Tessellation
{
public:
  //Default construction of Tessellation, enables vtk connectivity and cell types
  Tessellation();

  Tessellation(bool useVTKConnectivity,
               bool useVTKCellTypes);

  bool useVTKConnectivity() const { return this->m_useVTKConnectivity; }
  bool useVTKCellTypes() const { return this->m_useVTKCellTypes; }

  //This class self allocates all the memory needed to extract tessellation
  //and auto extract the tessellation based on the MeshSet or CellSet you
  //pass in
  void extract( const smtk::mesh::MeshSet& ms );
  void extract( const smtk::mesh::CellSet& cs );

  void extract( const smtk::mesh::MeshSet& cs, const smtk::mesh::PointSet& ps );
  void extract( const smtk::mesh::CellSet& cs, const smtk::mesh::PointSet& ps );

  //use these methods to gain access to the tessellation after
  const std::vector<std::int64_t>& connectivity() const {return this->m_connectivity;}
  const std::vector<std::int64_t>& cellLocations() const {return this->m_cellLocations;}
  const std::vector<unsigned char>& cellTypes() const {return this->m_cellTypes;}
  const std::vector<double>& points() const {return this->m_points;}

private:
  std::vector<std::int64_t> m_connectivity;
  std::vector<std::int64_t> m_cellLocations;
  std::vector<unsigned char> m_cellTypes;

  std::vector<double> m_points;

  bool m_useVTKConnectivity;
  bool m_useVTKCellTypes;
};

//Don't wrap these for python, instead python should use the Tessellation class
//and the extract method
#ifndef SHIBOKEN_SKIP

SMTKCORE_EXPORT void extractTessellation( const smtk::mesh::MeshSet&,
                                          PreAllocatedTessellation& );
SMTKCORE_EXPORT void extractTessellation( const smtk::mesh::CellSet&,
                                          PreAllocatedTessellation& );
SMTKCORE_EXPORT void extractTessellation( const smtk::model::EntityRef&,
                                          const smtk::mesh::CollectionPtr&,
                                          PreAllocatedTessellation& );
SMTKCORE_EXPORT void extractOrderedTessellation(
  const smtk::model::EdgeUse&,
  const smtk::mesh::CollectionPtr&,
  PreAllocatedTessellation& );
SMTKCORE_EXPORT void extractOrderedTessellation(
  const smtk::model::Loop&,
  const smtk::mesh::CollectionPtr&,
  PreAllocatedTessellation& );

//Extract Tessellation in respect to another PointSet instead of the PointSet
//contained by the meshset. This is useful if you are sharing a single
//PointSet among multiple Tessellations.
SMTKCORE_EXPORT void extractTessellation( const smtk::mesh::MeshSet&,
                                          const smtk::mesh::PointSet&,
                                          PreAllocatedTessellation& );
SMTKCORE_EXPORT void extractTessellation( const smtk::mesh::CellSet&,
                                          const smtk::mesh::PointSet&,
                                          PreAllocatedTessellation& );
SMTKCORE_EXPORT void extractTessellation( smtk::mesh::PointConnectivity&,
                                          const smtk::mesh::PointSet&,
                                          PreAllocatedTessellation& );
SMTKCORE_EXPORT void extractTessellation( const smtk::model::EntityRef&,
                                          const smtk::mesh::CollectionPtr&,
                                          const smtk::mesh::PointSet&,
                                          PreAllocatedTessellation& );
SMTKCORE_EXPORT void extractOrderedTessellation(
  const smtk::model::EdgeUse&,
  const smtk::mesh::CollectionPtr&,
  const smtk::mesh::PointSet&,
  PreAllocatedTessellation& );
SMTKCORE_EXPORT void extractOrderedTessellation(
  const smtk::model::Loop&,
  const smtk::mesh::CollectionPtr&,
  const smtk::mesh::PointSet&,
  PreAllocatedTessellation& );

#endif //SHIBOKEN_SKIP

}
}

#endif
