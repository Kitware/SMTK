//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/multiscale/operators/PartitionBoundariesOperator.h"

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/bridge/multiscale/Session.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/model/Vertex.h"

using namespace smtk::model;
using namespace smtk::common;

namespace
{
void extent(smtk::mesh::MeshSet ms, double ext[6])
{
  class Extent : public smtk::mesh::PointForEach
  {
  public:
    Extent() : smtk::mesh::PointForEach()
    {
      m_values[0] = m_values[2] = m_values[4] =
        std::numeric_limits<double>::max();
      m_values[1] = m_values[3] = m_values[5] =
        std::numeric_limits<double>::lowest();
    }

    void forPoints(const smtk::mesh::HandleRange&,
                   std::vector<double>& xyz,
                   bool&)
    {
      for(std::size_t i = 0; i < xyz.size(); i += 3)
        {
        for (std::size_t j = 0; j < 3; j++)
          {
          if (xyz[i + j] < this->m_values[2*j])
            {
            this->m_values[2*j] = xyz[i + j];
            }
          if (xyz[i + j] > this->m_values[2*j + 1])
            {
            this->m_values[2*j + 1] = xyz[i + j];
            }
          }
        }
    }

    double m_values[6];
  };

  Extent extent;
  smtk::mesh::for_each( ms.points(), extent );
  for (std::size_t i=0;i<6;i++)
    {
    ext[i] = extent.m_values[i];
    }
}

class Filter : public smtk::mesh::CellForEach
{
public:
  Filter() :
    smtk::mesh::CellForEach(true) // needs coordinates
  { }

  smtk::mesh::HandleRange validPoints;
};

class CoolingPlateFilter : public Filter
{
  double yvalue;
  double rvalue;
  double origin[3];
  bool lessThan;
public:

  CoolingPlateFilter(double yval, double rval, const double* o,
                     bool less=true)
    : Filter(),
      yvalue( yval ),
      rvalue( rval ),
      lessThan( less )
    {
      for ( int i=0; i < 3; ++i)
      {
        this->origin[i] = o[i];
      }
    }

  void forCell(const smtk::mesh::Handle&,
               smtk::mesh::CellType,
               int numPts)
  {
    const std::vector<double>& coords = this->coordinates();
    const smtk::mesh::Handle* const ptIds = this->pointIds();
    for( int i=0; i < numPts; ++i)
    {
      const double r = sqrt((coords[(i*3)] - this->origin[0]) *
                            (coords[(i*3)] - this->origin[0]) +
                            (coords[(i*3)+2] - this->origin[2]) *
                            (coords[(i*3)+2] - this->origin[2]));
      const double currValue[2] = {coords[(i*3)+1], r};
      //add in a small tolerance
      if(currValue[0] >= (this->yvalue-0.002) &&
         currValue[0] <= (this->yvalue+0.002))
      {
        if ((this->lessThan && (currValue[1]<this->rvalue)) ||
            ((!this->lessThan) && (currValue[1]>=this->rvalue)))
        {
          this->validPoints.insert(ptIds[i]);
        }
      }
    }
  }
};

class OuterEdgeFilter : public Filter
{
  double origin[3];
  double rmin;
public:
  OuterEdgeFilter( const double o[3], double r ) :
    Filter(),
    rmin(r)
    {
      for (int i=0;i<3;i++)
      {
        this->origin[i] = o[i];
      }
    }

  void forCell(const smtk::mesh::Handle&,
               smtk::mesh::CellType,
               int numPts)
  {
    const std::vector<double>& coords = this->coordinates();
    const smtk::mesh::Handle* const ptIds = this->pointIds();

    for ( int i=0; i < numPts; ++i)
    {
      // reject any cells whose first coordinate is less than a distance <rmin>
      // from the axis of rotation
      const double r = sqrt((coords[(i*3)] - this->origin[0]) *
                            (coords[(i*3)] - this->origin[0]) +
                            (coords[(i*3)+2] - this->origin[2]) *
                            (coords[(i*3)+2] - this->origin[2]));
      if (r > rmin)
      {
        this->validPoints.insert(ptIds[i]);
      }
    }
  }
};

bool labelIntersection(const smtk::mesh::CollectionPtr& collection,
                       const smtk::mesh::MeshSet& shell,
                       Filter& filter,
                       smtk::model::EntityRefArray& created,
                       smtk::bridge::mesh::Topology& topology)
{
  typedef smtk::bridge::mesh::Topology Topology;

  //need to remove the verts cells from the query for now
  //todo: filter needs to support vert cells
  smtk::mesh::CellSet shellCells = shell.cells( );

  //extract the top cells
  smtk::mesh::for_each(shellCells, filter);
  smtk::mesh::CellSet filteredCells = smtk::mesh::CellSet(collection,
                                                          filter.validPoints);

  //intersect the material and verts to find the verts of a given
  //material that passed the filter.
  //This verts than become a dirichlet set
  for(auto&& dom : collection->domains())
    {
    smtk::mesh::MeshSet domainMeshes = collection->meshes(dom);

    //find all cells on the top of shell that share a vert in common
    //with material volume
    smtk::mesh::CellSet domainCells = domainMeshes.cells();
    smtk::mesh::CellSet contactCells =
      smtk::mesh::point_intersect(domainCells,
                                  filteredCells,
                                  smtk::mesh::FullyContained);
    if(!contactCells.is_empty())
      {
      smtk::mesh::MeshSet contactD = collection->createMesh(contactCells);
      collection->setDirichletOnMeshes(contactD,
                                       smtk::mesh::Dirichlet(created.size()));

      // construct a new uuid
      smtk::common::UUID id = collection->modelManager()->unusedUUID();
      // construct a topology element for the vertex set (dimension 0)
      Topology::Element element(0);
      // insert the element into the topology under the parent level
      // (designating it as a "free" element)
      topology.m_elements.insert(std::make_pair(id, element));
      // store an entity ref associated with the vertex set id
      created.push_back(smtk::model::Vertex(collection->modelManager(), id));
      }
    }

  return true;
}

}

namespace smtk {
  namespace bridge {
    namespace multiscale {

smtk::model::OperatorResult PartitionBoundariesOperator::operateInternal()
{
  smtk::model::OperatorResult result;

  // Grab the datasets associated with the operator
  smtk::model::Models datasets =
    this->specification()->associatedModelEntities<smtk::model::Models>();
  if (datasets.empty())
    {
    smtkErrorMacro(this->log(), "No models on which to partition boundaries.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }
  smtk::model::Model dataset = datasets[0];

  // The collection for this model has the same UUID as the model, so we can
  // access it using the model's UUID
  smtk::mesh::CollectionPtr collection =
    this->activeSession()->meshManager()
    ->findCollection( dataset.entity() )->second;

  if ( !collection->isValid() )
    {
    smtkErrorMacro(this->log(), "No collection associated with this model.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::bridge::mesh::Topology* topology =
    this->activeSession()->topology(dataset);

  if ( !topology )
    {
    smtkErrorMacro(this->log(), "No topology associated with this model.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  // Set the origin from the values held in the specification
  double origin[3];
  smtk::attribute::DoubleItemPtr originItem =
    this->specification()->findDouble("origin");
  for (int i=0; i<3; i++)
    {
    origin[i] = originItem->value(i);
    }

  // Set the radius from the values held in the specification
  double radius = this->specification()->findDouble("radius")->value();

  //extract the exterior-shell for all meshes.
  smtk::mesh::MeshSet shell = collection->meshes().extractShell();

  // compute the shell's bounds
  double bounds[6];
  extent(shell, bounds);

  // we're going to generate vertices, so we need to keep track of them
  smtk::model::EntityRefArray created;

  const double ymin = bounds[2];
    {
    CoolingPlateFilter filter(ymin, radius, origin);
    labelIntersection(collection, shell, filter, created, *topology);
    }

  const double ymax = bounds[3];
    {
    CoolingPlateFilter filter(ymax, radius, origin);
    labelIntersection(collection, shell, filter, created, *topology);
    }

    {
    OuterEdgeFilter filter(origin, radius*2.);
    labelIntersection(collection, shell, filter, created, *topology);
    }

  result = this->createResult(smtk::model::OPERATION_SUCCEEDED);
  this->addEntitiesToResult(result, created, CREATED);

  smtk::bridge::multiscale::Session* sess = this->activeSession();
  if (sess)
    {
    smtk::attribute::ModelEntityItem::Ptr modelItem =
      this->specification()->associations();
    smtk::model::Model model = modelItem->value(0);
    model.addCells(created);
    this->addEntityToResult(result, model, MODIFIED);
    }

  return result;
}

    } // namespace multiscale
  } //namespace bridge
} // namespace smtk

#include "smtk/bridge/multiscale/Exports.h"
#include "smtk/bridge/multiscale/PartitionBoundariesOperator_xml.h"

smtkImplementsModelOperator(
  SMTKMULTISCALESESSION_EXPORT,
  smtk::bridge::multiscale::PartitionBoundariesOperator,
  multiscale_partition_boundaries,
  "partition boundaries",
  PartitionBoundariesOperator_xml,
  smtk::bridge::multiscale::Session);
