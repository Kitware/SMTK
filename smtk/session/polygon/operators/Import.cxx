//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "Import.h"

#include "smtk/Options.h"

#include "smtk/session/polygon/Session.h"
#include "smtk/session/polygon/internal/Model.h"
#include "smtk/session/polygon/operators/CreateEdge.h"
#include "smtk/session/polygon/operators/CreateEdgeFromPoints.h"
#include "smtk/session/polygon/operators/CreateModel.h"
#include "smtk/session/polygon/operators/ForceCreateFace.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/SessionRef.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/common/Paths.h"

#include "smtk/session/polygon/operators/Import_xml.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkIdTypeArray.h"
#include "vtkLongArray.h"
#include "vtkNew.h"
#include "vtkPDataSetReader.h"
#include "vtkPolyData.h"
#include "vtkStripper.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLPolyDataWriter.h"
#include <vtksys/SystemTools.hxx>

using namespace smtk::model;

namespace smtk
{
namespace session
{

namespace polygon
{

int polyLines2modelEdges(
  vtkPolyData* mesh,
  smtk::operation::Operation::Ptr edgeOp,
  smtk::model::EntityRefArray& createdEds,
  smtk::attribute::DoubleItem::Ptr pointsItem,
  const vtkIdType* pts,
  vtkIdType npts,
  smtk::io::Logger& logger)
{
  double p[3];
  // create edge for current line cell
  pointsItem->setNumberOfValues(npts * 3);
  for (vtkIdType j = 0; j < npts; ++j)
  {
    mesh->GetPoint(pts[j], p);
    for (int i = 0; i < 3; ++i)
    {
      pointsItem->setValue(3 * j + i, p[i]);
    }
  }
  smtk::operation::Operation::Result edgeResult = edgeOp->operate();
  if (
    edgeResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    smtkDebugMacro(logger, "\"create edge\" op failed to creat edge with given line cells.");
    return 0;
  }
  // Append createdEds to newEdges:
  auto newEdges = edgeResult->findComponent("created");
  newEdges->as(createdEds, [](smtk::resource::PersistentObjectPtr obj) {
    return smtk::model::EntityRef(std::dynamic_pointer_cast<smtk::model::Entity>(obj));
  });
  return static_cast<int>(newEdges->numberOfValues());
}

int Import::taggedPolyData2PolygonModelEntities(
  smtk::session::polygon::Resource::Ptr& resource,
  vtkIdTypeArray* tagInfo,
  vtkPolyData* pdata,
  smtk::model::Model& model)
{
  smtk::session::polygon::SessionPtr sess = resource->polygonSession();
  smtk::model::Resource::Ptr modelResource = sess->resource();
  internal::pmodel::Ptr storage = resource->findStorage<internal::pmodel>(model.entity());
  vtkPoints* points = pdata->GetPoints();
  vtkCellArray* verts = pdata->GetVerts();
  std::vector<double> pcoords;
  const vtkIdType* pts{ nullptr };
  vtkIdType npts{ 0 };
  double pnt[3];
  int numEnts = 0;
  vtkIdType linesOffset, n = 0;
  if (verts)
  {
    // Need to copy the point coordinate of the vertices
    vtkIdType i;
    n = verts->GetNumberOfCells();
    pcoords.resize(3 * n);
    for (i = 0, verts->SetTraversalLocation(0); verts->GetNextCell(npts, pts);)
    {
      // There should only be 1 point per vertex
      if (npts != 1)
      {
        smtkErrorMacro(this->log(), "Encountered Vertx Cell with " << npts << " points!");
      }
      points->GetPoint(pts[0], pnt);
      pcoords[i++] = pnt[0];
      pcoords[i++] = pnt[1];
      pcoords[i++] = pnt[2];
    }
    smtk::model::Vertices gVerts = storage->findOrAddModelVertices(modelResource, pcoords, 3);
    numEnts += static_cast<int>(gVerts.size());
    i = 0;
    for (auto gVert : gVerts)
    { // Add raw relationships from model to/from vertex:
      model.addCell(gVert);
      gVert.setIntegerProperty("pedigree", tagInfo->GetValue(i++));
    }
  }

  // Next lets process the edges
  linesOffset = n;
  vtkCellArray* lines = pdata->GetLines();
  vtkIdType currentEdgeTag, cellId;
  vtkIdType lastPointId = -1;
  smtk::operation::Operation::Ptr edgeOp = CreateEdgeFromPoints::create();
  auto createEdgeOp = std::dynamic_pointer_cast<CreateEdgeFromPoints>(edgeOp);
  if (!lines)
  {
    return numEnts;
  }
  n = lines->GetNumberOfCells();
  pcoords.clear();
  pcoords.reserve((n + 1) * 3);

  for (cellId = linesOffset, lines->SetTraversalLocation(0); lines->GetNextCell(npts, pts);
       cellId++)
  {
    // There should only be 2 point per line
    if (npts != 2)
    {
      smtkErrorMacro(this->log(), "Encountered Line Cell with " << npts << " points!");
    }
    // Is this a new edge? - in that case we need to use both points
    if (pcoords.empty() || (currentEdgeTag != tagInfo->GetValue(cellId)))
    {
      // Were we walking a model edge already?
      if (!pcoords.empty())
      {
        createEdgeOp->process(pcoords, 3, model);
        numEnts++;
      }
      pcoords.clear();
      for (int j = 0; j < 2; j++)
      {
        points->GetPoint(pts[j], pnt);
        pcoords.push_back(pnt[0]);
        pcoords.push_back(pnt[1]);
        pcoords.push_back(pnt[2]);
      }
      currentEdgeTag = tagInfo->GetValue(cellId);
      lastPointId = pts[1];
      continue;
    }
    // We assume that the lines are in order - lets verify that!
    if (pts[0] != lastPointId)
    {
      smtkErrorMacro(this->log(), "Found a disjointed model edge!");
    }
    points->GetPoint(pts[1], pnt);
    pcoords.push_back(pnt[0]);
    pcoords.push_back(pnt[1]);
    pcoords.push_back(pnt[2]);
    lastPointId = pts[1];
  }
  if (!pcoords.empty())
  {
    createEdgeOp->process(pcoords, 3, model);
    numEnts++;
  }

  return numEnts;
}

int Import::basicPolyData2PolygonModelEntities(
  smtk::session::polygon::Resource::Ptr& resource,
  vtkPolyData* polyLines,
  smtk::model::Model& model)
{
  smtk::session::polygon::SessionPtr sess = resource->polygonSession();
  // First lets strip the original polydata into polylines
  vtkNew<vtkPolyData> pdata;
  vtkNew<vtkStripper> stripper;
  stripper->SetInputData(polyLines);
  stripper->Update();
  pdata->ShallowCopy(stripper->GetOutput());
  smtk::model::Resource::Ptr modelResource = sess->resource();
  internal::pmodel::Ptr storage = resource->findStorage<internal::pmodel>(model.entity());
  vtkPoints* points = pdata->GetPoints();
  std::vector<double> pcoords;
  const vtkIdType* pts{ nullptr };
  vtkIdType npts{ 0 };
  double pnt[3];
  int numEnts = 0;
  vtkIdType i, j;
  vtkCellArray* lines = pdata->GetLines();
  vtkIdType cellId;
  smtk::operation::Operation::Ptr edgeOp = CreateEdgeFromPoints::create();
  auto createEdgeOp = std::dynamic_pointer_cast<CreateEdgeFromPoints>(edgeOp);
  if (!lines)
  {
    return numEnts;
  }

  for (cellId = 0, lines->SetTraversalLocation(0); lines->GetNextCell(npts, pts); cellId++)
  {
    pcoords.clear();
    pcoords.resize(npts * 3);
    for (j = 0, i = 0; i < npts; i++)
    {
      points->GetPoint(pts[i], pnt);
      pcoords[j++] = pnt[0];
      pcoords[j++] = pnt[1];
      pcoords[j++] = pnt[2];
    }
    createEdgeOp->process(pcoords, 3, model);
    numEnts++;
  }
  return numEnts;
}

int polyLines2modelEdgesAndFaces(
  vtkPolyData* mesh,
  smtk::model::Model& model,
  smtk::io::Logger& logger)
{
  int numEdges = 0;
  vtkCellArray* lines = mesh->GetLines();
  if (lines)
  {
    smtk::operation::Operation::Ptr edgeOp = CreateEdge::create();
    smtk::attribute::AttributePtr spec = edgeOp->parameters();
    spec->associateEntity(model);
    smtk::attribute::IntItem::Ptr constructMethod = spec->findInt("construction method");
    constructMethod->setDiscreteIndex(0); // "points coornidates"
    smtk::attribute::IntItem::Ptr numCoords = spec->findInt("coordinates");
    numCoords->setValue(3); // number of elements in coordinates
    smtk::attribute::DoubleItem::Ptr pointsItem = spec->findDouble("points");

    smtk::operation::Operation::Ptr faceOp = ForceCreateFace::create();
    smtk::attribute::AttributePtr faceSpec = faceOp->parameters();
    faceSpec->findInt("construction method")->setDiscreteIndex(1); // "edges"

    vtkIdTypeArray* pedigreeIds =
      vtkIdTypeArray::SafeDownCast(mesh->GetCellData()->GetPedigreeIds());

    vtkIdType numPedIDs = pedigreeIds ? pedigreeIds->GetNumberOfTuples() : 0;
    vtkIdType* pedigree =
      numPedIDs == lines->GetNumberOfCells() && pedigreeIds ? pedigreeIds->GetPointer(0) : nullptr;
    /*
    std::cout << "number of line cells: " << lines->GetNumberOfCells() << std::endl;
    if(pedigreeIds)
      {
      std::cout << "number of pedigreeIds: " << numPedIDs << std::endl;
      }
*/
    vtkIdType pidx = 0;
    const vtkIdType* pts{ nullptr };
    vtkIdType npts{ 0 };
    for (lines->SetTraversalLocation(0); lines->GetNextCell(npts, pts); ++pidx)
    {
      std::vector<int>
        counts; // how many edges make up the outer loop, how many inner loops are there and how many edges for each?
      smtk::model::EntityRefArray createdEds;
      // create edge for current line cell
      int numNewEdges =
        polyLines2modelEdges(mesh, edgeOp, createdEds, pointsItem, pts, npts, logger);

      counts.push_back(numNewEdges);
      counts.push_back(0);
      // peek at next pedigree id if possible, to see if the pedId is the same, if yes,
      // the next cell is the inner loop
      vtkIdType pedId = -1;
      if (pedigree && numNewEdges > 0)
      {
        pedId = pedigree[pidx];
        //std::cout << "pedid: " << pedId << std::endl;
        while (pidx < numPedIDs - 1 && pedId == pedigree[pidx + 1])
        {
          // The next line cell is an inner loop
          if (lines->GetNextCell(npts, pts))
          {
            ++counts[1]; // increment the number of inner loops
            numNewEdges +=
              polyLines2modelEdges(mesh, edgeOp, createdEds, pointsItem, pts, npts, logger);
            counts.push_back(numNewEdges);
          }
          //std::cout << "inner pedid: " << pedId << std::endl;
          ++pidx;
        }
      }
      if (numNewEdges > 0)
      {
        numEdges += numNewEdges;
        auto assocs = faceSpec->associations();
        assocs->setNumberOfValues(createdEds.size());
        int ii = 0;
        for (const auto& ced : createdEds)
        {
          assocs->setValue(ii, ced.component());
          ++ii;
        }
        //std::cout << "number of created new edges: " << createdEds.size() << std::endl;
        smtk::attribute::IntItem::Ptr orientArr = faceSpec->findInt("orientations");
        std::vector<int> orients(numNewEdges, -1);
        orients[0] = +1; // first one is outer loop
        orientArr->setValues(orients.begin(), orients.end());
        smtk::attribute::IntItem::Ptr countsArr = faceSpec->findInt("counts");
        countsArr->setValues(counts.begin(), counts.end());

        smtk::operation::Operation::Result faceResult = faceOp->operate();
        if (
          faceResult->findInt("outcome")->value() !=
          static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
        {
          smtkDebugMacro(
            logger, "\"force create face\" op failed to create face with given edges.");
        }
        // Add a pedigree ID (if we have it, or -1 otherwise) to each face:
        smtk::model::EntityRef(faceResult->findComponent("created")->valueAs<smtk::model::Entity>())
          .setIntegerProperty("pedigree", pedId);
      }
    }
  }
  return numEdges;
}

Import::Import() = default;

bool Import::ableToOperate()
{
  if (!this->parameters()->isValid())
    return false;

  std::string filename = this->parameters()->findFile("filename")->value();
  if (filename.empty())
    return false;
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(filename);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  return (ext == ".vtp" || ext == ".vtk");
}

Import::Result Import::operateInternal()
{
  Result result;
  std::string filename = this->parameters()->findFile("filename")->value();
  if (filename.empty())
  {
    smtkErrorMacro(log(), "File name is empty!");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  vtkPolyData* polyOutput = vtkPolyData::New();

  // ******************************************************************************
  // This is where we should have the logic to import files other than .cmb formats
  // ******************************************************************************
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(filename);
  if (ext == ".vtp")
  {
    vtkNew<vtkXMLPolyDataReader> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();

    vtkNew<vtkDataSetSurfaceFilter> surface;
    surface->SetInputData(0, reader->GetOutputDataObject(0));
    surface->Update();
    polyOutput->ShallowCopy(surface->GetOutput());
  }
  else if (ext == ".vtk")
  {
    vtkNew<vtkPDataSetReader> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();

    vtkNew<vtkDataSetSurfaceFilter> surface;
    surface->SetInputData(0, reader->GetOutputDataObject(0));
    surface->Update();
    polyOutput->ShallowCopy(surface->GetOutput());
  }
  else
  {
    smtkErrorMacro(log(), "Unhandled file extension " << ext << ".");
    polyOutput->Delete();
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // First create a model with CreateModel op, then use line cells from reader's
  // output polydata to create edges
  smtk::operation::Operation::Ptr modOp = smtk::session::polygon::CreateModel::create();
  if (!modOp)
  {
    smtkErrorMacro(log(), "Failed to create CreateModel op.");
    result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  //modOp->findInt("model scale")->setValue(1);

  double bds[6];
  polyOutput->GetBounds(bds);
  double diam = 0.0;
  for (int i = 0; i < 3; ++i)
  {
    diam += (bds[2 * i + 1] - bds[2 * i]) * (bds[2 * i + 1] - bds[2 * i]);
  }
  diam = sqrt(diam);
  //std::cout << "diam " << diam << "\n";

  // Use the lower-left-front bounds as the origin of the plane.
  // This keeps the projected integer coordinates small when the dataset is not
  // well-centered about the origin and makes overflow less likely.
  // However, it does mean that multiple imported polygon models in the same
  // plane will not share coordinates exactly.
  for (int i = 0; i < 3; ++i)
  {
    modOp->parameters()->findDouble("origin")->setValue(i, bds[2 * i]);
  }
  // Infer a feature size from the bounds:
  modOp->parameters()->findDouble("feature size")->setValue(diam / 1000.0);

  {
    smtk::attribute::ReferenceItem::Ptr existingResourceItem = this->parameters()->associations();

    if (existingResourceItem->numberOfValues() > 0)
    {
      modOp->parameters()->associate(existingResourceItem->value());
      smtk::attribute::StringItem::Ptr sessionOnlyItem =
        this->parameters()->findString("session only");
      modOp->parameters()->findString("session only")->setValue(sessionOnlyItem->value());
    }
  }

  smtk::operation::Operation::Result modResult = modOp->operate();
  if (
    modResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    smtkInfoMacro(log(), "CreateModel operator failed.");
    result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Retrieve the resulting resource
  smtk::attribute::ResourceItemPtr resourceItem =
    std::dynamic_pointer_cast<smtk::attribute::ResourceItem>(
      modResult->findResource("resourcesCreated"));

  smtk::session::polygon::Resource::Ptr resource =
    std::dynamic_pointer_cast<smtk::session::polygon::Resource>(resourceItem->value());

  std::string potentialName = smtk::common::Paths::stem(filename);
  if (!potentialName.empty() && resource && resource->name().empty())
  {
    resource->setName(potentialName);
  }

  // Retrieve the resulting model
  smtk::attribute::ComponentItemPtr componentItem =
    std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(modResult->findComponent("model"));

  // Access the generated model
  smtk::model::Model model = std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

  int numEntities;
  // Are we dealing with tagged polydata or polydata with pedigrre info?
  vtkIdTypeArray* tagInfo =
    vtkIdTypeArray::SafeDownCast(polyOutput->GetCellData()->GetArray("ElementIds"));
  vtkIdTypeArray* pedigreeIds =
    vtkIdTypeArray::SafeDownCast(polyOutput->GetCellData()->GetPedigreeIds());

  if (tagInfo)
  {
    numEntities = this->taggedPolyData2PolygonModelEntities(resource, tagInfo, polyOutput, model);
  }
  else if (!pedigreeIds)
  {
    numEntities = this->basicPolyData2PolygonModelEntities(resource, polyOutput, model);
  }
  else
  {
    numEntities = polyLines2modelEdgesAndFaces(polyOutput, model, log());
  }
  smtkDebugMacro(log(), "Number of entities: " << numEntities << "\n");

  result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  {
    smtk::attribute::ComponentItem::Ptr resultModels = result->findComponent("model");
    resultModels->setValue(model.component());
  }

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resourcesCreated");
    created->appendValue(resource);
  }

  {
    smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
    created->setValue(model.component());
  }

  operation::MarkGeometry(resource).markResult(result);

  polyOutput->Delete();
  return result;
}

const char* Import::xmlDescription() const
{
  return Import_xml;
}

} // namespace polygon
} // namespace session
} // namespace smtk
