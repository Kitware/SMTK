//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/ExtractField.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/ExtractTessellation.h"
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
#include <cstring>
#include <limits>
#include <utility>

namespace smtk {
namespace mesh {

//----------------------------------------------------------------------------
void PreAllocatedField::determineAllocationLengths(
  const smtk::mesh::MeshSet& ms,
  boost::int64_t& numberOfCells,
  boost::int64_t& numberOfPoints)
{
  boost::int64_t connectivityLength;
  PreAllocatedTessellation::determineAllocationLengths(ms.cells(),
                                                       connectivityLength,
                                                       numberOfCells,
                                                       numberOfPoints);
}

//----------------------------------------------------------------------------
PreAllocatedField::PreAllocatedField(boost::int64_t* cellField,
                                     boost::int64_t* pointField):
  m_cellField(cellField),
  m_pointField(pointField)
{

}

//----------------------------------------------------------------------------
template <typename QueryTag>
void Field::extract( const smtk::mesh::MeshSet& ms,
                     const smtk::mesh::PointSet& ps )
{
  //determine the lengths
  boost::int64_t numberOfCells = -1;
  boost::int64_t numberOfPoints = -1;

  PreAllocatedField::determineAllocationLengths(ms,
                                                numberOfCells,
                                                numberOfPoints);
  this->m_cellData.resize( numberOfCells );
  //since the input PointSet can contain more points that the computed number,
  //set numberOfPoints to the PointSet size.
  numberOfPoints = ps.size();
  this->m_pointData.resize( numberOfPoints );

  smtk::mesh::PreAllocatedField field(&this->m_cellData[0],
                                      &this->m_pointData[0]);

  extractField<QueryTag>(ms,ps,field);
}

template void Field::extract<smtk::mesh::Dirichlet>(
  const smtk::mesh::MeshSet& ms, const smtk::mesh::PointSet& ps);
template void Field::extract<smtk::mesh::Neumann>(
  const smtk::mesh::MeshSet& ms, const smtk::mesh::PointSet& ps);
template void Field::extract<smtk::mesh::Domain>(
  const smtk::mesh::MeshSet& ms, const smtk::mesh::PointSet& ps);

//----------------------------------------------------------------------------
void Field::extractDirichlet( const smtk::mesh::MeshSet& ms )
{
  this->extract<smtk::mesh::Dirichlet>(ms, ms.points());
}

//----------------------------------------------------------------------------
void Field::extractNeumann( const smtk::mesh::MeshSet& ms )
{
  this->extract<smtk::mesh::Neumann>(ms, ms.points());
}

//----------------------------------------------------------------------------
void Field::extractDomain( const smtk::mesh::MeshSet& ms )
{
  this->extract<smtk::mesh::Domain>(ms, ms.points());
}

//----------------------------------------------------------------------------
void Field::extractDirichlet( const smtk::mesh::MeshSet& ms,
                              const smtk::mesh::PointSet& ps )
{
  this->extract<smtk::mesh::Dirichlet>(ms, ps);
}

//----------------------------------------------------------------------------
void Field::extractNeumann( const smtk::mesh::MeshSet& ms,
                            const smtk::mesh::PointSet& ps )
{
  this->extract<smtk::mesh::Neumann>(ms, ps);
}

//----------------------------------------------------------------------------
void Field::extractDomain( const smtk::mesh::MeshSet& ms,
                           const smtk::mesh::PointSet& ps )
{
  this->extract<smtk::mesh::Domain>(ms, ps);
}

//----------------------------------------------------------------------------
namespace
{
void QueryTags(const smtk::mesh::MeshSet& ms,
               std::vector<smtk::mesh::Dirichlet>& tags)
{
  tags = ms.dirichlets();
}

void QueryTags(const smtk::mesh::MeshSet& ms,
               std::vector<smtk::mesh::Neumann>& tags)
{
  tags = ms.neumanns();
}

void QueryTags(const smtk::mesh::MeshSet& ms,
               std::vector<smtk::mesh::Domain>& tags)
{
  tags = ms.domains();
}

template <typename QueryTag>
struct TaggedRangeForQuery
{
  TaggedRangeForQuery(QueryTag tag, std::pair<std::size_t, std::size_t> range) :
    m_tag(tag), m_range(range) {}

  friend bool operator< (const TaggedRangeForQuery &left,
                         const TaggedRangeForQuery &right)
  {
    return left.m_range.first < right.m_range.first;
  }

  QueryTag m_tag;
  std::pair<std::size_t, std::size_t> m_range;
};

}

//----------------------------------------------------------------------------
template <typename QueryTag>
void extractField( const smtk::mesh::MeshSet& ms,
                   const smtk::mesh::PointSet& ps,
                   PreAllocatedField& field)
{
  // We need to preserve the internal ordering of the cells and points in
  // MeshSet <ms>. So, we first separate <ms> into discrete submeshes for each
  // unique tag. We then add a "null" tag with a unique integer value to denote
  // the absense of a tag. Then, we loop over each submesh and extract
  // contiguous cell and point ranges. We then sort these contiguous ranges.
  // Finally, we assign tag values to our cell and point fields according to
  // each contiguous block.

  typedef TaggedRangeForQuery<QueryTag> TaggedRange;

  // <cellRanges> and <pointRanges> hold the tag for a contiguous cell/point
  // range, and the start and end indices (inclusive) of the range.
  std::vector<TaggedRange> cellRanges;
  std::vector<TaggedRange> pointRanges;

  // <meshes> holds the tag and discrete submesh for each unique tag.
  std::vector<std::pair<QueryTag, smtk::mesh::MeshSet> > meshes;

  // <tags> holds the unique tags
  std::vector<QueryTag> tags;
  QueryTags(ms, tags);

  // We add a "null" tag to denote cells/points that are not tagged. The null
  // tag is assigned to the mesh elements that are not associated with any tag,
  // so we loop over the existing subsets and remove them from the original
  // mesh.
  smtk::mesh::MeshSet nullSet = ms;
  meshes.reserve(tags.size());
  for (auto&& tag : tags)
    {
    smtk::mesh::MeshSet subset = ms.subset(tag);
    meshes.push_back(std::make_pair(tag, subset));
    nullSet = set_difference(nullSet, subset);
    }

  // We need an unused value to assign our null tag. We start with -1, and
  // increment until we find an unused value.
  int nullValue = -1;
  while (true)
    {
    bool foundUniqueValue = true;
    for (auto&& subset : meshes)
      {
      if (subset.first.value() == nullValue)
        {
        foundUniqueValue = false;
        break;
        }
      }
    if (foundUniqueValue)
      {
      break;
      }
    ++nullValue;
    }

  // Now that our null tag and null set are constructed, we add them as a pair
  // to our meshes array.
  meshes.push_back(std::make_pair(QueryTag(nullValue), nullSet));

  // From here, it is assumed that <meshes> forms a complete partition (i.e. no
  // overlaps, no missing elements) of our original meshset. To guard against
  // this assumption, we continuously extract our subcells/subpoints from the
  // original cells/points, ensuring there is no overlap. We loop over our
  // partition and extract contiguous cell/point ranges associated with each
  // tag. This data is used to populate <cellRanges> and <pointRanges>.

  // <cells> and <points> are kept intact because they are used as references
  // for indexing
  smtk::mesh::CellSet cells = ms.cells();
  const smtk::mesh::PointSet& points = ps;

  // <remainingCells> and <remainingPoints> are contininually diminished to
  // ensure that no overlapping cells or points are double-counted
  smtk::mesh::CellSet remainingCells = cells;
  smtk::mesh::PointSet remainingPoints = points;

  cellRanges.reserve(meshes.size());
  pointRanges.reserve(meshes.size());
  for (auto&& subset : meshes)
    {
    smtk::mesh::CellSet subcells = set_intersect(remainingCells,
                                                 subset.second.cells());
    smtk::mesh::PointSet subpoints = set_intersect(remainingPoints,
                                                   subset.second.points());
    for (auto range = subcells.range().const_pair_begin();
         range != subcells.range().const_pair_end(); ++range)
      {
      cellRanges.push_back(
        TaggedRange(subset.first,
                    std::make_pair(cells.range().index(range->first),
                                   cells.range().index(range->second))));
      }

    for (auto range = subpoints.range().const_pair_begin();
         range != subpoints.range().const_pair_end(); ++range)
      {
      pointRanges.push_back(
        TaggedRange(subset.first,
                    std::make_pair(points.range().index(range->first),
                                   points.range().index(range->second))));
      }

    remainingCells = set_difference(remainingCells, subcells);
    remainingPoints = set_difference(remainingPoints, subpoints);
    }

  // Since each range is contiguous and represents a partition of the underlying
  // cells/points of the input meshset, we use as a sort function a comparator
  // between the first indices of each range.
  std::sort(cellRanges.begin(), cellRanges.end());
  std::sort(pointRanges.begin(), pointRanges.end());

  // Finally, we assign our tag value to contiguous blocks of our cell/point
  // field arrays.
  std::size_t cellRangeStart = 0;
  for (auto&& range : cellRanges)
    {
      std::size_t span = range.m_range.second - range.m_range.first + 1;
      std::fill(&field.m_cellField[cellRangeStart],
                &field.m_cellField[cellRangeStart] + span,
                range.m_tag.value());
      cellRangeStart += span;
    }

  std::size_t pointRangeStart = 0;
  for (auto&& range : pointRanges)
    {
      std::size_t span = range.m_range.second - range.m_range.first + 1;
      std::fill(&field.m_pointField[pointRangeStart],
                &field.m_pointField[pointRangeStart] + span,
                range.m_tag.value());
      pointRangeStart += span;
    }
}

template void extractField<smtk::mesh::Dirichlet>(
  const smtk::mesh::MeshSet&, const smtk::mesh::PointSet&, PreAllocatedField&);
template void extractField<smtk::mesh::Neumann>(
  const smtk::mesh::MeshSet&, const smtk::mesh::PointSet&, PreAllocatedField&);
template void extractField<smtk::mesh::Domain>(
  const smtk::mesh::MeshSet&, const smtk::mesh::PointSet&, PreAllocatedField&);

//----------------------------------------------------------------------------
void extractDirichletField( const smtk::mesh::MeshSet& ms,
                            PreAllocatedField& field)
{
  extractField<smtk::mesh::Dirichlet>(ms, ms.points(), field);
}

//----------------------------------------------------------------------------
void extractNeumannField( const smtk::mesh::MeshSet& ms,
                          PreAllocatedField& field)
{
  extractField<smtk::mesh::Neumann>(ms, ms.points(), field);
}

//----------------------------------------------------------------------------
void extractDomainField( const smtk::mesh::MeshSet& ms,
                         PreAllocatedField& field)
{
  extractField<smtk::mesh::Domain>(ms, ms.points(), field);
}

//----------------------------------------------------------------------------
void extractDirichletField( const smtk::mesh::MeshSet& ms,
                            const smtk::mesh::PointSet& ps,
                            PreAllocatedField& field)
{
  extractField<smtk::mesh::Dirichlet>(ms, ps, field);
}

//----------------------------------------------------------------------------
void extractNeumannField( const smtk::mesh::MeshSet& ms,
                          const smtk::mesh::PointSet& ps,
                          PreAllocatedField& field)
{
  extractField<smtk::mesh::Neumann>(ms, ps, field);
}

//----------------------------------------------------------------------------
void extractDomainField( const smtk::mesh::MeshSet& ms,
                         const smtk::mesh::PointSet& ps,
                         PreAllocatedField& field)
{
  extractField<smtk::mesh::Domain>(ms, ps, field);
}


}
}
