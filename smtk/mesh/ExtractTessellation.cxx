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
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/PointConnectivity.h"
#include "smtk/mesh/PointSet.h"

#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Loop.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Vertex.h"

#include <cmath>
#include <utility>
#include <deque>

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
std::size_t smtkToSMTKConn(std::int64_t&, std::size_t index, int)
{
  //do nothing as we aren't storing the length of each cell in the connectivity
  //array
  return index;
}

inline
std::size_t smtkToVTKConn(std::int64_t& conn, std::size_t index, int numPts)
{
  //store the length of the cell in the connectivity array
  conn = numPts;
  return index + 1;
}

} //namespace detail

//----------------------------------------------------------------------------
void PreAllocatedTessellation::determineAllocationLengths(const smtk::mesh::MeshSet& ms,
                                                          std::int64_t& connectivityLength,
                                                          std::int64_t& numberOfCells,
                                                          std::int64_t& numberOfPoints)

{
  determineAllocationLengths(ms.cells(),
                             connectivityLength,
                             numberOfCells,
                             numberOfPoints);
}

//----------------------------------------------------------------------------
void PreAllocatedTessellation::determineAllocationLengths(const smtk::mesh::CellSet& cs,
                                                          std::int64_t& connectivityLength,
                                                          std::int64_t& numberOfCells,
                                                          std::int64_t& numberOfPoints)

{
  connectivityLength = cs.pointConnectivity().size();
  numberOfCells = cs.size();
  numberOfPoints = cs.points().size();
}

//----------------------------------------------------------------------------
void PreAllocatedTessellation::determineAllocationLengths(const smtk::model::EntityRef& eRef,
                                                          const smtk::mesh::CollectionPtr& c,
                                                          std::int64_t& connectivityLength,
                                                          std::int64_t& numberOfCells,
                                                          std::int64_t& numberOfPoints)

{
  determineAllocationLengths(c->findAssociatedCells(eRef),
                             connectivityLength,
                             numberOfCells,
                             numberOfPoints);
}

//----------------------------------------------------------------------------
void PreAllocatedTessellation::determineAllocationLengths(const smtk::model::Loop& loop,
                                                          const smtk::mesh::CollectionPtr& c,
                                                          std::int64_t& connectivityLength,
                                                          std::int64_t& numberOfCells,
                                                          std::int64_t& numberOfPoints)

{
  smtk::mesh::HandleRange cellRange;
  smtk::mesh::CellSet cells(c,cellRange);

  // Grab the edge uses from the loop.
  smtk::model::EdgeUses euses = loop.edgeUses();

  // Loop over the edge uses
  for (auto eu=euses.crbegin(); eu!=euses.crend(); ++eu)
    {
    // Collect the cells associated with the edge connected to the edge use
    cells = set_union(cells, c->findAssociatedCells(eu->edge()));
    }

  determineAllocationLengths(cells,
                             connectivityLength,
                             numberOfCells,
                             numberOfPoints);
}

//----------------------------------------------------------------------------
PreAllocatedTessellation::PreAllocatedTessellation( std::int64_t* connectivity ):
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
PreAllocatedTessellation::PreAllocatedTessellation(std::int64_t* connectivity,
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
PreAllocatedTessellation::PreAllocatedTessellation(std::int64_t* connectivity,
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
PreAllocatedTessellation::PreAllocatedTessellation(std::int64_t* connectivity,
                                                   std::int64_t* cellLocations,
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
PreAllocatedTessellation::PreAllocatedTessellation(std::int64_t* connectivity,
                                                   std::int64_t* cellLocations,
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
PreAllocatedTessellation::PreAllocatedTessellation(std::int64_t* connectivity,
                                                   std::int64_t* cellLocations,
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
  std::int64_t connectivityLength= -1;
  std::int64_t numberOfCells = -1;
  std::int64_t numberOfPoints = -1;

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
void extractTessellation( const smtk::model::EntityRef& eRef,
                          const smtk::mesh::CollectionPtr& c,
                          PreAllocatedTessellation& tess)
{
  CellSet cs = c->findAssociatedCells(eRef);
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
  smtk::mesh::PointConnectivity pc = cs.pointConnectivity();
  extractTessellation(pc,ps,tess);
}

//----------------------------------------------------------------------------
template <class PointConnectivity>
void extractTessellationInternal( PointConnectivity& pc,
                                  const smtk::mesh::PointSet& ps,
                                  PreAllocatedTessellation& tess)
{
  //we need to detect what options of tesselation the user has enabled.
  const bool fetch_cellLocations = tess.m_cellLocations != NULL;
  const bool fetch_fPoints = tess.m_fpoints != NULL;
  const bool fetch_dPoints = tess.m_dpoints != NULL;

  //we need to determine what version of this function we are actually
  //using. This is fairly important as we don't want to branch within the
  //tight loop. To complicate the matter we also have to determine if
  //we are using VTK style connectivity or a compacted connectivity format.

  //determine the function pointer to use for the connectivity array
  std::size_t (*addCellLen)(std::int64_t& conn, std::size_t index, int numPts) = detail::smtkToSMTKConn;
  if(tess.m_useVTKConnectivity)
    {
    addCellLen = detail::smtkToVTKConn;
    }

  int numPts = 0;
  const smtk::mesh::Handle* pointIds;
  std::size_t conn_index = 0;
  smtk::mesh::CellType ctype;
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
    for(pc.initCellTraversal(); pc.fetchNextCell(ctype, numPts, pointIds); conn_index += numPts)
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

//----------------------------------------------------------------------------

void extractTessellation( smtk::mesh::PointConnectivity& pc,
                          const smtk::mesh::PointSet& ps,
                          PreAllocatedTessellation& tess)
{
  extractTessellationInternal<smtk::mesh::PointConnectivity>(pc,ps,tess);
}

//----------------------------------------------------------------------------
void extractTessellation( const smtk::model::EntityRef& eRef,
                          const smtk::mesh::CollectionPtr& c,
                          const smtk::mesh::PointSet& ps,
                          PreAllocatedTessellation& tess)
{
  extractTessellation(c->findAssociatedCells(eRef), ps, tess);
}

//----------------------------------------------------------------------------
namespace
{
  // A Link is simply a pair of vertex ids.
  struct Link
  {
    const smtk::mesh::Handle& first()  const { return this->Handles[0]; }
    const smtk::mesh::Handle& second() const { return this->Handles[1]; }

    smtk::mesh::Handle Handles[2];
  };

  // A Chain is a list of Links, allowing for O[1] prepending, appending and
  // joining.
  typedef std::deque<Link> Chain;

  // An OrderedEdge is a list of Chains that supports the addition of links and
  // the merging of chains.
  struct OrderedEdge
  {
    OrderedEdge() :
      MergeLimit(std::numeric_limits<std::size_t>::max()) {}

    // When a link is inserted, we check to see if it can be prepended or
    // appended to any extant chains. If it can, we add it to the appropriate
    // chain in the correct orientation. Otherwise, it seeds a new Chain. If
    // the number of Chains exceeds the user-defined Merge Limit, the Chains
    // are merged.
    void insert_link(Link& l)
    {
      if (this->Chains.size() >= this->MergeLimit)
        {
        this->merge_chains();
        }

      // link (a,b)
      for (auto& c : this->Chains)
        {
        if (l.second() == c.front().first())
          {
          // (a,b) -> (b,...)
          c.push_front(l);
          return;
          }
        else if (l.first() == c.back().second())
          {
          // (...,a) <- (a,b)
          c.push_back(l);
          return;
          }
        }
      Chain c(1,l);
      this->Chains.push_back(c);
    }

    // merge_chains consists of two loops over our Chains. For each Chain c1,
    // we cycle through the subsequent Chains in the list to see if they can be
    // appended or prepended to c1. Once all possible connections have been made
    // to c1, we move to the next chain. If all Links are present, the outer
    // loop will execute exactly one iteration. Otherwise, Chain fragments are
    // merged, ensuring the fewest possible number of Chains remain.
    void merge_chains()
    {
      for (auto c1 = this->Chains.begin(); c1 != Chains.end();)
        {
        const std::size_t c1_size = c1->size();
        auto c2 = c1;
        for (++c2; c2 != Chains.end(); ++c2)
          {
          if (c2->empty())
            {
            continue;
            }

          // chain c1 looks like (a,...,b)
          if (c1->front().first() == c2->back().second())
            {
            // (...,a) -> (a,...,b)
            c1->insert(c1->begin(),
                       std::make_move_iterator(c2->begin()),
                       std::make_move_iterator(c2->end()));
            c2->clear();
            }
          else if (c2->front().first() == c1->back().second())
            {
            // (a,...,b) <- (b,...)
            c1->insert(c1->end(),
                       std::make_move_iterator(c2->begin()),
                       std::make_move_iterator(c2->end()));
            c2->clear();
            }
          }
        if (c1->size() == c1_size)
          {
          ++c1;
          }
        }

      // Erase the empty chains.
      for (auto c1 = this->Chains.begin(); c1 != Chains.end();)
        {
        if (c1->empty())
          {
          c1 = this->Chains.erase(c1);
          }
        else
          {
          ++c1;
          }
        }
    }

    std::deque<Chain> Chains;
    std::size_t MergeLimit;
  };

  class OrderedConnectivity
  {
  public:
    OrderedConnectivity(const std::deque<Chain>& chains) :
      Chains(chains) {}

    void initCellTraversal()
    {
      this->WhichConnectivityVector = this->Chains.begin();
      if (this->WhichConnectivityVector != this->Chains.end())
        {
        this->WhichLink = (*this->WhichConnectivityVector).begin();
        }
    }

    bool fetchNextCell( smtk::mesh::CellType& cellType,
                        int& numPts,
                        const smtk::mesh::Handle* &points)
    {
      // IterationState tracks connectivity by indices. We have lists though,
      // so we keep their indices and our iterators in lockstep. We probably
      // don't need to update the state at all, since our storage is not
      // accessible externally.

      numPts = 0;

      // We start by ensuring our connectivity vector is valid.
      if (this->WhichConnectivityVector == this->Chains.end())
        {
        return false;
        }

      // We then increment our offset in the connectivity vector and check that
      // it is still valid
      if (++this->WhichLink == this->WhichConnectivityVector->end())
        {
        if (++this->WhichConnectivityVector == this->Chains.end())
          {
          return false;
          }
        this->WhichLink = this->WhichConnectivityVector->begin();
        }

      // Now our connectivity iterators and indices are all in the right place.
      cellType = smtk::mesh::Line;
      numPts = 2;
      points = this->WhichLink->Handles;

      return true;
    }

  private:
    const std::deque<Chain>& Chains;
    std::deque<Chain>::const_iterator WhichConnectivityVector;
    Chain::const_iterator WhichLink;
  };

//----------------------------------------------------------------------------
template <class OneDimensionalEntities>
void extractOrderedTessellation( const OneDimensionalEntities& oneDimEntities,
                                 const smtk::mesh::CollectionPtr& c,
                                 const smtk::mesh::PointSet& ps,
                                 PreAllocatedTessellation& tess)
{
  // Assuming that the lines that make up a loop are oriented but not ordered,
  // extractOrderedTessellation() takes an iterable collection of 1-dimensional
  // model entities and orders the output lines before extracting the
  // tessellation.

  OrderedEdge orderedEdge;
  Link link;

  for (auto ent=oneDimEntities.cbegin(); ent != oneDimEntities.cend(); ++ent)
    {
    // Collect the cells associated with the edge connected to the edge use
    CellSet cs = c->findAssociatedCells(*ent);

    // Retrieve its point connectivity array
    smtk::mesh::PointConnectivity pc = cs.pointConnectivity();

    int numPts = 0;
    const smtk::mesh::Handle* pointIds;
    // For large lists of cells, have the loop collapse its chain fragments
    // periodically during insertion.
    if (pc.numberOfCells() > 20)
      {
      orderedEdge.MergeLimit = std::sqrt(pc.numberOfCells());
      }
    // For each line segment in our edge...
    for (pc.initCellTraversal(); pc.fetchNextCell(numPts, pointIds);)
      {
      // ... we construct a link and add it to our ordered loop.
      link.Handles[0]  = ps.find( pointIds[0] );
      link.Handles[1] = ps.find( pointIds[1] );
      orderedEdge.insert_link(link);
      }
    orderedEdge.merge_chains();
    }

  // Create our own PointConnectivity using our ordered edge
  OrderedConnectivity orderedPC(orderedEdge.Chains);

  extractTessellationInternal<OrderedConnectivity>(orderedPC,ps,tess);
}
}

//----------------------------------------------------------------------------
void extractOrderedTessellation( const smtk::model::Edge& edge,
                                 const smtk::mesh::CollectionPtr& c,
                                 PreAllocatedTessellation& tess)
{
  // Collect the cells associated with the edge
  smtk::mesh::CellSet cells = c->findAssociatedCells(edge);
  extractOrderedTessellation(edge,c,cells.points(),tess);
}

//----------------------------------------------------------------------------
void extractOrderedTessellation( const smtk::model::Loop& loop,
                                 const smtk::mesh::CollectionPtr& c,
                                 PreAllocatedTessellation& tess)
{
  smtk::mesh::HandleRange cellRange;
  smtk::mesh::CellSet cells(c,cellRange);

  // Grab the edge uses from the loop.
  smtk::model::EdgeUses euses = loop.edgeUses();

  // Loop over the edge uses
  for (auto eu=euses.crbegin(); eu!=euses.crend(); ++eu)
    {
    // Collect the cells associated with the edge connected to the edge use
    cells = set_union(cells, c->findAssociatedCells(eu->edge()));
    }

  extractOrderedTessellation(loop,c,cells.points(),tess);
}

//----------------------------------------------------------------------------
void extractOrderedTessellation( const smtk::model::Edge& edge,
                                 const smtk::mesh::CollectionPtr& c,
                                 const smtk::mesh::PointSet& ps,
                                 PreAllocatedTessellation& tess)
{
  smtk::model::Edges edges;
  edges.push_back(edge);
  extractOrderedTessellation<smtk::model::Edges>(edges,c,ps,tess);
}

//----------------------------------------------------------------------------
void extractOrderedTessellation( const smtk::model::Loop& loop,
                                 const smtk::mesh::CollectionPtr& c,
                                 const smtk::mesh::PointSet& ps,
                                 PreAllocatedTessellation& tess)
{
  smtk::model::EdgeUses euses = loop.edgeUses();
  smtk::model::Edges edges;
  for (auto eu=euses.cbegin(); eu!=euses.cend(); ++eu)
    {
    edges.push_back(eu->edge());
    }

  extractOrderedTessellation<smtk::model::Edges>(edges,c,ps,tess);
}

}
}
