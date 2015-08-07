//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

class SMTKCORE_EXPORT PreAllocatedTessellation
{

public:
  static void determineAllocationLengths(smtk::mesh::MeshSet,
                                         boost::int64& connectivityLength,
                                         boost::int64& numberOfCells,
                                         boost::int64& numberOfPoints);

  static void determineAllocationLengths(smtk::mesh::CellSet,
                                         boost::int64& connectivityLength,
                                         boost::int64& numberOfCells,
                                         boost::int64& numberOfPoints);

  //Only converts connectivity. The following properties will not be
  //converted: cellLocations, cellTypes, and Points
  PreAllocatedTessellation(  boost::int64* connectivity );

  //Converts everything but Points.
  PreAllocatedTessellation(  boost::int64* connectivity,
                             boost::int64* cellLocations,
                             unsigned char* cellTypes);

  //Converts everything and stores the points as floats
  PreAllocatedTessellation(  boost::int64* connectivity,
                             boost::int64* cellLocations,
                             unsigned char* cellTypes,
                             float* points);

  //Converts everything and stores the points as doubles
  PreAllocatedTessellation(  boost::int64* connectivity,
                             boost::int64* cellLocations,
                             unsigned char* cellTypes,
                             double* points);

  //determine if you want VTK 6.0 style connectivity array where each cell
  //is preceded with an entry that states the length of the cell. Passing
  //in True will disable this behavior. The default behavior of the
  //class is to use VTK style connectivity.
  void disableVTKStyleConnectivity(bool);

  boost::int64* m_connectivity;
  boost::int64* m_cellLocations;
  unsigned char* m_cellTypes;

  double* m_dpoints;
  float* m_fpoints;

  bool m_useVTKStyle;
};

void extractTessellation( smtk::mesh::MeshSet, PreAllocatedTessellation );
void extractTessellation( smtk::mesh::CellSet, PreAllocatedTessellation );

//Extract Tessellation in respect to another PointSet instead of the PointSet
//contained by the meshset. This is useful if you are sharing a single
//PointSet among multiple Tessellations.
void extractTessellation( smtk::mesh::MeshSet, smtk::mesh::PointSet, PreAllocatedTessellation );
void extractTessellation( smtk::mesh::CellSet, smtk::mesh::PointSet , PreAllocatedTessellation );
