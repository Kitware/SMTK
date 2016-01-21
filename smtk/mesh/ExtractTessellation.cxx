//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/ExtractTessellation.h"
#include "smtk/mesh/PointConnectivity.h"
#include "smtk/mesh/PointSet.h"

namespace smtk {
namespace mesh {

namespace detail
{

inline
unsigned char smtkToSMTKCell(smtk::mesh::CellType t)
  {
  return t;
  }

inline
unsigned char smtkToVTKCell(smtk::mesh::CellType t)
  {
  unsigned char ctype = 0; //VTK_EMPTY_CELL
  switch (t)
    {
    case smtk::mesh::Vertex:
      ctype = 1; //VTK_VERTEX
      break;
    case smtk::mesh::Line:
      ctype = 3; //VTK_LINE
      break;
    case smtk::mesh::Triangle:
      ctype = 5; //VTK_TRIANGLE
      break;
    case smtk::mesh::Quad:
      ctype = 9; //VTK_QUAD
      break;
    case smtk::mesh::Polygon:
      ctype = 7; //VTK_POLYGON
      break;
    case smtk::mesh::Tetrahedron:
      ctype = 10; //VTK_TETRA
      break;
    case smtk::mesh::Pyramid:
      ctype = 14; //VTK_PYRAMID
      break;
    case smtk::mesh::Wedge:
      ctype = 13; //VTK_WEDGE
      break;
    case smtk::mesh::Hexahedron:
      ctype = 12; //VTK_HEXAHEDRON
      break;
    default:
      ctype = 0; //VTK_EMPTY_CELL
      break;
    }
  return ctype;
  }

inline
std::size_t smtkToSMTKLocation(const std::size_t& location, int numPts)
{
  return location + numPts;
}

inline
std::size_t smtkToVTKLocation(const std::size_t& location, int numPts)
{
  return location + numPts + 1;
}

inline
std::size_t smtkToSMTKConn(boost::int64_t&, std::size_t index, int)
{
  //do nothing as we aren't storing the length of each cell in the connectivity
  //array
  return index;
}

inline
std::size_t smtkToVTKConn(boost::int64_t& conn, std::size_t index, int numPts)
{
  //store the length of the cell in the connectivity array
  conn = numPts;
  return index + 1;
}

} //namespace detail

//----------------------------------------------------------------------------
void PreAllocatedTessellation::determineAllocationLengths(const smtk::mesh::MeshSet& ms,
                                                          boost::int64_t& connectivityLength,
                                                          boost::int64_t& numberOfCells,
                                                          boost::int64_t& numberOfPoints)

{
  determineAllocationLengths(ms.cells(),
                             connectivityLength,
                             numberOfCells,
                             numberOfPoints);

}

//----------------------------------------------------------------------------
void PreAllocatedTessellation::determineAllocationLengths(const smtk::mesh::CellSet& cs,
                                                          boost::int64_t& connectivityLength,
                                                          boost::int64_t& numberOfCells,
                                                          boost::int64_t& numberOfPoints)

{
  connectivityLength = cs.pointConnectivity().size();
  numberOfCells = cs.size();
  numberOfPoints = cs.points().size();
}

//----------------------------------------------------------------------------
PreAllocatedTessellation::PreAllocatedTessellation( boost::int64_t* connectivity ):
  m_connectivity(connectivity),
  m_cellLocations(NULL),
  m_cellTypes(NULL),
  m_dpoints(NULL),
  m_fpoints(NULL),
  m_useVTKConnectivity(true),
  m_useVTKCellTypes(true)
{

}

//----------------------------------------------------------------------------
PreAllocatedTessellation::PreAllocatedTessellation(boost::int64_t* connectivity,
                                                   float* points):
  m_connectivity(connectivity),
  m_cellLocations(NULL),
  m_cellTypes(NULL),
  m_dpoints(NULL),
  m_fpoints(points),
  m_useVTKConnectivity(true),
  m_useVTKCellTypes(true)
{

}

//----------------------------------------------------------------------------
PreAllocatedTessellation::PreAllocatedTessellation(boost::int64_t* connectivity,
                                                   double* points):
  m_connectivity(connectivity),
  m_cellLocations(NULL),
  m_cellTypes(NULL),
  m_dpoints(points),
  m_fpoints(NULL),
  m_useVTKConnectivity(true),
  m_useVTKCellTypes(true)
{

}

//----------------------------------------------------------------------------
PreAllocatedTessellation::PreAllocatedTessellation(boost::int64_t* connectivity,
                                                   boost::int64_t* cellLocations,
                                                   unsigned char* cellTypes):
  m_connectivity(connectivity),
  m_cellLocations(cellLocations),
  m_cellTypes(cellTypes),
  m_dpoints(NULL),
  m_fpoints(NULL),
  m_useVTKConnectivity(true),
  m_useVTKCellTypes(true)
{

}

//----------------------------------------------------------------------------
PreAllocatedTessellation::PreAllocatedTessellation(boost::int64_t* connectivity,
                                                   boost::int64_t* cellLocations,
                                                   unsigned char* cellTypes,
                                                   float* points):
  m_connectivity(connectivity),
  m_cellLocations(cellLocations),
  m_cellTypes(cellTypes),
  m_dpoints(NULL),
  m_fpoints(points),
  m_useVTKConnectivity(true),
  m_useVTKCellTypes(true)
{

}

//----------------------------------------------------------------------------
PreAllocatedTessellation::PreAllocatedTessellation(boost::int64_t* connectivity,
                                                   boost::int64_t* cellLocations,
                                                   unsigned char* cellTypes,
                                                   double* points):
  m_connectivity(connectivity),
  m_cellLocations(cellLocations),
  m_cellTypes(cellTypes),
  m_dpoints(points),
  m_fpoints(NULL),
  m_useVTKConnectivity(true),
  m_useVTKCellTypes(true)
{

}

//----------------------------------------------------------------------------
Tessellation::Tessellation( ):
  m_connectivity(),
  m_cellLocations(),
  m_cellTypes(),
  m_points(),
  m_useVTKConnectivity(true),
  m_useVTKCellTypes(true)
{

}

//----------------------------------------------------------------------------
Tessellation::Tessellation( bool useVTKConnectivity,
                            bool useVTKCellTypes):
  m_connectivity(),
  m_cellLocations(),
  m_cellTypes(),
  m_points(),
  m_useVTKConnectivity(useVTKConnectivity),
  m_useVTKCellTypes(useVTKCellTypes)
{

}

//----------------------------------------------------------------------------
void Tessellation::extract( const smtk::mesh::MeshSet& ms )
{
  this->extract(ms.cells(), ms.points());
}

//----------------------------------------------------------------------------
void Tessellation::extract( const smtk::mesh::CellSet& cs )
{
  this->extract(cs, cs.points());
}

//----------------------------------------------------------------------------
void Tessellation::extract( const smtk::mesh::MeshSet& ms,
                            const smtk::mesh::PointSet& ps )
{
  this->extract(ms.cells(), ps);
}

//----------------------------------------------------------------------------
void Tessellation::extract( const smtk::mesh::CellSet& cs,
                            const smtk::mesh::PointSet& ps )
{
  //determine the lengths
  boost::int64_t connectivityLength= -1;
  boost::int64_t numberOfCells = -1;
  boost::int64_t numberOfPoints = -1;

  PreAllocatedTessellation::determineAllocationLengths(cs,
                                                       connectivityLength,
                                                       numberOfCells,
                                                       numberOfPoints);
  //handle vtk style connectivity
  if(this->useVTKConnectivity())
    {
    connectivityLength += numberOfCells;
    }

  this->m_connectivity.resize( connectivityLength );
  this->m_cellLocations.resize( numberOfCells );
  this->m_cellTypes.resize( numberOfCells );
  //since the input PointSet can contain more points that the computed number,
  //set numberOfPoints to the PointSet size.
  numberOfPoints = ps.size();
  this->m_points.resize( numberOfPoints * 3 );

  smtk::mesh::PreAllocatedTessellation tess(&this->m_connectivity[0],
                                            &this->m_cellLocations[0],
                                            &this->m_cellTypes[0],
                                            &this->m_points[0]);

  //set the vtk connectivity and cell types flags properly
  const bool disableVTKConn = !this->useVTKConnectivity();
  const bool disableVTKCellTypes = !this->useVTKCellTypes();
  tess.disableVTKStyleConnectivity(disableVTKConn);
  tess.disableVTKCellTypes(disableVTKCellTypes);

  extractTessellation(cs,ps,tess);
}


//----------------------------------------------------------------------------
void extractTessellation( const smtk::mesh::MeshSet& ms,
                          PreAllocatedTessellation& tess)
{
  extractTessellation(ms.cells(),ms.points(),tess);
}

//----------------------------------------------------------------------------
void extractTessellation( const smtk::mesh::CellSet& cs,
                          PreAllocatedTessellation& tess)
{
  extractTessellation(cs,cs.points(),tess);
}

//----------------------------------------------------------------------------
void extractTessellation( const smtk::mesh::MeshSet& ms,
                          const smtk::mesh::PointSet& ps,
                          PreAllocatedTessellation& tess)
{
  extractTessellation(ms.cells(),ps,tess);
}

//----------------------------------------------------------------------------
void extractTessellation( const smtk::mesh::CellSet& cs,
                          const smtk::mesh::PointSet& ps,
                          PreAllocatedTessellation& tess)
{
  //we need to detect what options of tesselation the user has enabled.
  const bool fetch_cellLocations = tess.m_cellLocations != NULL;
  const bool fetch_fPoints = tess.m_fpoints != NULL;
  const bool fetch_dPoints = tess.m_dpoints != NULL;

  //we need to determine what version of this function we are actually
  //using. This is fairly important as we don't want to branch within the
  //tight loop. Too complicate the matter we also have to determine if
  //we are using VTK style connectivity, or a compacted connectivity format

  //determine the function pointer to use for the connectivity array
  std::size_t (*addCellLen)(boost::int64_t& conn, std::size_t index, int numPts) = detail::smtkToSMTKConn;
  if(tess.m_useVTKConnectivity)
    {
    addCellLen = detail::smtkToVTKConn;
    }

  smtk::mesh::PointConnectivity pc = cs.pointConnectivity();
  int numPts = 0;
  const smtk::mesh::Handle* pointIds;
  std::size_t conn_index = 0;
  if(fetch_cellLocations)
    {
    //determine the function pointer to use for the cell type conversion
    unsigned char (*convertCellTypeFunction)(smtk::mesh::CellType t) = detail::smtkToSMTKCell;
    if(tess.m_useVTKCellTypes)
      {
      convertCellTypeFunction = detail::smtkToVTKCell;
      }

    //determine the function pointer to use for the cell location conversion
    std::size_t (*updateCellLocationFunction)(const std::size_t& location, int numPts) = detail::smtkToSMTKLocation;
    if(tess.m_useVTKConnectivity)
      { //needs to check for vtk connectivity as that is how we deduce where the location is
      updateCellLocationFunction = detail::smtkToVTKLocation;
      }

    //Issue we haven't handled the VTK syst
    smtk::mesh::CellType ctype;
    std::size_t index = 0;
    for(pc.initCellTraversal(); pc.fetchNextCell(ctype, numPts, pointIds); ++index, conn_index += numPts)
      {
      //first we mark the current cell location this is done before addCellLen as that
      //will modify conn_index
      tess.m_cellLocations[index] = conn_index;

      conn_index = addCellLen(*(tess.m_connectivity + conn_index),
                              conn_index,
                              numPts);

      for(int i=0;  i < numPts; ++i)
        {
        //call find on the pointset to determine the proper index for the
        //point id. the point id value is based off the global point id, and
        //we need to transform it to a relative id based of the pointset
        //that was past in
        tess.m_connectivity[conn_index + i] = ps.find( pointIds[i] );
        }

      tess.m_cellTypes[index] = convertCellTypeFunction(ctype);
      }
    }
  else
    {
    for(pc.initCellTraversal(); pc.fetchNextCell(numPts, pointIds); conn_index += numPts)
      {
      conn_index = addCellLen(*(tess.m_connectivity + conn_index),
                              conn_index,
                              numPts);

      for(int i=0;  i < numPts; ++i)
        {
        //call find on the pointset to determine the proper index for the
        //point id. the point id value is based off the global point id, and
        //we need to transform it to a relative id based of the pointset
        //that was past in
        tess.m_connectivity[conn_index + i] = ps.find( pointIds[i] );
        }
      }
    }

  //we now have to read in the points if requested
  if(fetch_dPoints)
    {
    ps.get(tess.m_dpoints);
    }
  else if( fetch_fPoints )
    {
    ps.get(tess.m_fpoints);
    }
}


}
}
