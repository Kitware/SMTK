//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/UnstructuredData.h"

#include "smtk/markup/IndirectAssignedIds.h"
#include "smtk/markup/Resource.h"
#include "smtk/markup/SequentialAssignedIds.h"

#include "smtk/common/Paths.h"
#include "smtk/resource/json/Helper.h"

#include "vtkDataObject.h"
#include "vtkDataSetAttributes.h"
#include "vtkDataSetReader.h"
#include "vtkImageData.h"
#include "vtkNew.h"
#include "vtkPolyDataReader.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkXMLImageDataReader.h"

using namespace smtk::string::literals; // for ""_token

namespace smtk
{
namespace markup
{

namespace
{

int maxDimension(vtkSmartPointer<vtkDataObject> data)
{
  int result = -1;
  auto* dset = vtkDataSet::SafeDownCast(data);
  if (!dset)
  {
    return result;
  }

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 0)
  // This is stupid. In VTK 9.2+, we are not allowed to call the
  // vtkDataSet::GetCellTypes() method on unstructured grids without
  // a warning but polydata and other subclasses of dataset provide
  // no alternative API. vtkDataSet should provide a new API common
  // to *all* subclasses if the unstructured-grid implementation is
  // deprecated.
  if (auto* ugrid = vtkUnstructuredGrid::SafeDownCast(dset))
  {
    auto* cellTypes = ugrid->GetDistinctCellTypesArray();
    for (vtkIdType ii = 0; ii < cellTypes->GetNumberOfTuples(); ++ii)
    {
      int cellType = cellTypes->GetTuple1(ii);
      int dim = vtkCellTypes::GetDimension(cellType);
      if (dim > result)
      {
        result = dim;
      }
    }
    return result;
  }
#endif
  if (auto* image = vtkImageData::SafeDownCast(dset))
  {
    result = image->GetDataDimension();
  }
  else // NOLINT(readability-misleading-indentation)
  {
    // Use deprecated API.
    vtkNew<vtkCellTypes> cellTypes;
    dset->GetCellTypes(cellTypes);
    for (vtkIdType ii = 0; ii < cellTypes->GetNumberOfTypes(); ++ii)
    {
      int cellType = cellTypes->GetCellType(ii);
      int dim = cellTypes->GetDimension(cellType);
      if (dim > result)
      {
        result = dim;
      }
    }
  }

  return result;
}

} // anonymous namespace

UnstructuredData::~UnstructuredData() = default;

void UnstructuredData::initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper)
{
  auto resource = helper.resourceAs<smtk::markup::Resource>();
  auto pointSpace = std::dynamic_pointer_cast<IdSpace>(resource->domains().find("points"_token));
  auto cellSpace = std::dynamic_pointer_cast<IdSpace>(resource->domains().find("cells"_token));
  auto jpid = data["point_ids"];
  auto jcid = data["cell_ids"];
  auto jprr = jpid["range"].get<AssignedIds::IdRange>();
  auto jcrr = jcid["range"].get<AssignedIds::IdRange>();
  auto jpnn = jpid["nature"].get<std::string>();
  auto jcnn = jcid["nature"].get<std::string>();
  IdNature pnat = natureEnumerant(jpnn);
  IdNature cnat = natureEnumerant(jcnn);

  m_pointIds =
    std::make_shared<smtk::markup::AssignedIds>(pointSpace, pnat, jprr[0], jprr[1], this);
  m_cellIds = std::make_shared<smtk::markup::AssignedIds>(cellSpace, cnat, jcrr[0], jcrr[1], this);

  // Fetch data from shape URL.
  this->incoming<arcs::URLsToData>().visit([this, &helper](const URL* url) {
    std::string location = url->location().data();
    if (smtk::common::Paths::isRelative(location))
    {
      // Relative paths must be relative to the resource's directory.
      location = smtk::common::Paths::canonical(
        location, smtk::common::Paths::directory(url->parentResource()->location()));
    }
    smtk::string::Token mimeType = url->type();
    helper.futures().emplace_back(
      smtk::resource::json::Helper::threadPool()([this, &helper, location, mimeType]() {
        switch (mimeType.id())
        {
          case "vtk/polydata"_hash:
          {
            // vtkNew<vtkPolyDataReader> reader;
            vtkNew<vtkDataSetReader> reader;
            reader->SetFileName(location.c_str());
            reader->Update();
            // Do not call this->setShapeData(...) here since that could create nodes
            // that have already been read from the file. This could happen because
            // the order in which nodes are initialized is unspecified; field metadata
            // may not have been initialized.
            m_mesh = reader->GetOutputDataObject(0);
          }
          break;
          case "vtk/unstructured-grid"_hash:
          {
            // vtkNew<vtkUnstructuredGridReader> reader;
            vtkNew<vtkDataSetReader> reader;
            reader->SetFileName(location.c_str());
            reader->Update();
            // Do not call this->setShapeData(...) here since that could create nodes
            // that have already been read from the file. This could happen because
            // the order in which nodes are initialized is unspecified; Field metadata
            // may not have been initialized.
            m_mesh = reader->GetOutputDataObject(0);
          }
          break;
          case "vtk/image"_hash:
          {
            vtkNew<vtkXMLImageDataReader> reader;
            reader->SetFileName(location.c_str());
            reader->Update();
            m_mesh = reader->GetOutputDataObject(0);
          }
          break;
          default:
            smtkErrorMacro(
              smtk::io::Logger::instance(),
              "Unsupported shape format \"" << mimeType.data() << "\"");
            break;
        }
      }));
  });
}

void UnstructuredData::assignedIds(std::vector<AssignedIds*>& assignments) const
{
  assignments.clear();
  assignments.push_back(m_pointIds.get());
  assignments.push_back(m_cellIds.get());
}

bool UnstructuredData::setShapeData(vtkSmartPointer<vtkDataObject> mesh, ShapeOptions& options)
{
  auto& sharedPointIds = options.sharedPointIds;

  bool didChange = false;
  if (mesh == m_mesh)
  {
    return didChange;
  }
  // Add/remove Field instances connected to this mesh
  // as needed match the new \a mesh:
  didChange |= this->updateChildren(mesh, options);

  // Assign the mesh and request new ID assignments if needed.
  auto numberOfPointsPrior = m_pointIds ? m_pointIds->size() : 0;
  auto numberOfCellsPrior = m_cellIds ? m_cellIds->size() : 0;

  // Prepare to transition from old m_mesh to new mesh:
  if (mesh)
  {
    auto* resource = dynamic_cast<Resource*>(this->parentResource());
    auto numberOfPoints = static_cast<std::size_t>(mesh->GetNumberOfElements(vtkDataObject::POINT));
    auto numberOfCells = static_cast<std::size_t>(mesh->GetNumberOfElements(vtkDataObject::CELL));
    auto* self = const_cast<UnstructuredData*>(this);
    auto assignedPointIdSetup = [&mesh, self](
                                  const std::shared_ptr<IdSpace>& domain,
                                  IdNature nature,
                                  IdType begin,
                                  IdType end) -> std::shared_ptr<smtk::markup::AssignedIds> {
      if (auto* pdata = mesh->GetAttributes(vtkDataObject::POINT))
      {
        if (auto* pids = pdata->GetPedigreeIds())
        {
          (void)pids;
          auto assignedIds =
            std::make_shared<IndirectAssignedIds>(domain, nature, begin, end, self);
          // Initialize lookups from index←→ID here.
          // TODO: assignedIds->setPedigreeArray(pids);
          return assignedIds;
        }
      }
      auto seqIds = std::make_shared<SequentialAssignedIds>(domain, nature, begin, end, self);
      return seqIds;
    };
    if (numberOfPoints > numberOfPointsPrior || numberOfCells > numberOfCellsPrior)
    {
      // NB: Unstructured data should expect shared ownership of point IDs.
      if (sharedPointIds)
      {
        auto pointSpace = resource->domains().findAs<IdSpace>("points"_token);
        auto sr = sharedPointIds->range();
        // Make a copy of AssignedIds for ourselves and insert it into the interval tree:
        m_pointIds =
          std::make_shared<AssignedIds>(pointSpace, sharedPointIds->nature(), sr[0], sr[1], this);
        pointSpace->insertAssignment(m_pointIds.get());
      }
      else
      {
        m_pointIds = resource->domains()
                       .findAs<IdSpace>("points"_token)
                       ->requestRange(
                         IdNature::NonExclusive, numberOfPoints, InvalidId(), assignedPointIdSetup);
      }

      // Unlike point IDs, cell IDs should be exclusively owned by unstructured data.
      // Otherwise, you should create a side set or a subset instead of UnstructuredData.
      auto assignedCellIdSetup = [self](
                                   const std::shared_ptr<IdSpace>& domain,
                                   IdNature nature,
                                   IdType begin,
                                   IdType end) -> std::shared_ptr<smtk::markup::AssignedIds> {
        auto seqIds = std::make_shared<SequentialAssignedIds>(domain, nature, begin, end, self);
        return seqIds;
      };
      m_cellIds =
        resource->domains()
          .findAs<IdSpace>("cells"_token)
          ->requestRange(IdNature::Primary, numberOfCells, InvalidId(), assignedCellIdSetup);
    }
    // TODO: Should we iterate over referential geometry and erase it?
    //       In some cases, operations might be able to preserve them.
  }

  m_mesh = mesh;
  this->properties().get<long>()["dimension"] = maxDimension(m_mesh);
  didChange = true;

  return didChange;
}

vtkSmartPointer<vtkDataObject> UnstructuredData::shapeData() const
{
  return m_mesh;
}

ArcEndpointInterface<arcs::BoundariesToShapes, ConstArc, OutgoingArc> UnstructuredData::parents()
  const
{
  return this->outgoing<arcs::BoundariesToShapes>();
}

ArcEndpointInterface<arcs::BoundariesToShapes, NonConstArc, OutgoingArc> UnstructuredData::parents()
{
  return this->outgoing<arcs::BoundariesToShapes>();
}

ArcEndpointInterface<arcs::BoundariesToShapes, ConstArc, IncomingArc> UnstructuredData::children()
  const
{
  return this->incoming<arcs::BoundariesToShapes>();
}

ArcEndpointInterface<arcs::BoundariesToShapes, NonConstArc, IncomingArc>
UnstructuredData::children()
{
  return this->incoming<arcs::BoundariesToShapes>();
}

} // namespace markup
} // namespace smtk
