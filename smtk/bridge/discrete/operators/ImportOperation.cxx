//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "ImportOperation.h"

#include "smtk/bridge/discrete/Resource.h"
#include "smtk/bridge/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/ModelToMesh.h"

#include "smtk/mesh/core/Collection.h"

#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/SessionRef.h"

#include "smtk/extension/vtk/reader/vtkCMBGeometryReader.h"

#include "vtkDataSetRegionSurfaceFilter.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkMasterPolyDataNormals.h"
#include "vtkMergeDuplicateCells.h"
#include "vtkModel.h"
#include "vtkModelItem.h"
#include "vtkPDataSetReader.h"

#ifdef SMTK_ENABLE_REMUS_SUPPORT
#include "smtk/extension/vtk/meshing/vtkCMBTriangleMesher.h"
#include "smtk/extension/vtk/reader/vtkCMBGeometry2DReader.h"
#include "smtk/extension/vtk/reader/vtkCMBMapReader.h"
#endif

#ifdef SMTK_ENABLE_MOAB_DISCRETE_READER
#include "smtk/bridge/discrete/moabreader/vtkCmbMoabReader.h"
#endif

#include "ImportOperation_xml.h"
#include "ModelParserHelper.h"
#include <vtksys/SystemTools.hxx>

#include "cJSON.h"
#include "smtk/io/SaveJSON.h"

using namespace smtk::model;

namespace smtk
{
namespace bridge
{

namespace discrete
{

ImportOperation::ImportOperation()
{
}

bool ImportOperation::ableToOperate()
{
  if (!this->parameters()->isValid())
    return false;

  std::string filename = this->parameters()->findFile("filename")->value();
  if (filename.empty())
    return false;
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(filename);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  bool able = (ext == ".vtk" || ext == ".2dm" || ext == ".3dm" ||
#ifdef SMTK_ENABLE_MOAB_DISCRETE_READER
    ext == ".h5m" || ext == ".sat" || ext == ".brep" || ext == ".stp" || ext == ".cub" ||
    ext == ".exo" ||
#endif
#ifdef SMTK_ENABLE_REMUS_SUPPORT
    ext == ".poly" || ext == ".smesh" || ext == ".map" ||
#endif
    /*  ext == ".tin" ||
      ext == ".fac" ||
      ext == ".obj" ||
      ext == ".sol" ||*/
    ext == ".stl");

// for shape files, the reader needs user inputs, so
// "ShapeBoundaryStyle" item needs to be checked first.
#ifdef SMTK_ENABLE_REMUS_SUPPORT
  if (ext == ".shp")
  {
    smtk::attribute::StringItem::Ptr boundaryItem =
      this->parameters()->findString("ShapeBoundaryStyle");
    able = boundaryItem->isEnabled();
  }
#endif

  return able;
}

ImportOperation::Result ImportOperation::operateInternal()
{
  std::string filename = this->parameters()->findFile("filename")->value();

  if (filename.empty())
  {
    std::cerr << "File name is empty!\n";
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // There are three possible import modes
  //
  // 1. Import a mesh into an existing resource
  // 2. Import a mesh as a new model, but using the session of an existing resource
  // 3. Import a mesh into a new resource

  smtk::bridge::discrete::Resource::Ptr resource = nullptr;
  smtk::bridge::discrete::Session::Ptr session = nullptr;

  // Modes 2 and 3 requre an existing resource for input
  smtk::attribute::ResourceItem::Ptr existingResourceItem =
    this->parameters()->findResource("resource");

  if (existingResourceItem && existingResourceItem->isEnabled())
  {
    smtk::bridge::discrete::Resource::Ptr existingResource =
      std::static_pointer_cast<smtk::bridge::discrete::Resource>(existingResourceItem->value());

    session = existingResource->discreteSession();

    smtk::attribute::StringItem::Ptr sessionOnlyItem =
      this->parameters()->findString("session only");
    if (sessionOnlyItem->value() == "this file")
    {
      // If the "session only" value is set to "this file", then we use the
      // existing resource
      resource = existingResource;
    }
    else
    {
      // If the "session only" value is set to "this session", then we create a
      // new resource with the session from the exisiting resource
      resource = smtk::bridge::discrete::Resource::create();
      resource->setSession(session);
    }
  }
  else
  {
    // If no existing resource is provided, then we create a new session and
    // resource.
    resource = smtk::bridge::discrete::Resource::create();
    session = smtk::bridge::discrete::Session::create();

    // Create a new resource for the import
    resource->setLocation(filename);
    resource->setSession(session);
  }

  // Create a new model to hold the result.
  vtkNew<vtkDiscreteModelWrapper> mod;

  // ******************************************************************************
  // This is where we should have the logic to import files other than .cmb formats
  // ******************************************************************************
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(filename);
  if (ext == ".h5m" || ext == ".sat" || ext == ".brep" || ext == ".stp" || ext == ".cub" ||
    ext == ".exo")
  {
#ifdef SMTK_ENABLE_MOAB_DISCRETE_READER

    vtkNew<vtkCmbMoabReader> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();

    m_op->Operate(mod.GetPointer(), reader.GetPointer());
#endif
  }
  else if (ext == ".2dm" || ext == ".3dm" ||
#ifdef SMTK_ENABLE_REMUS_SUPPORT
    ext == ".poly" || ext == ".smesh" ||
#endif
    /*  ext == ".tin" ||
      ext == ".fac" ||
      ext == ".obj" ||
      ext == ".sol" || */
    ext == ".stl")
  {
    vtkNew<vtkCMBGeometryReader> reader;
    reader->SetFileName(filename.c_str());
    reader->SetPrepNonClosedSurfaceForModelCreation(true);
    reader->Update();

    bool hasBoundaryEdges = reader->GetHasBoundaryEdges();

    if (ext == ".poly" || ext == ".smesh" || hasBoundaryEdges)
    {
      m_op->Operate(mod.GetPointer(), reader.GetPointer());
    }
    else
    {
      vtkNew<vtkMasterPolyDataNormals> normals;
      normals->SetInputData(0, reader->GetOutputDataObject(0));
      normals->Update();

      vtkNew<vtkMergeDuplicateCells> merge;
      merge->SetModelRegionArrayName(ModelParserHelper::GetShellTagName());
      merge->SetModelFaceArrayName(ModelParserHelper::GetModelFaceTagName());
      merge->SetInputData(0, normals->GetOutputDataObject(0));
      merge->Update();

      m_op->Operate(mod.GetPointer(), merge.GetPointer());
    }
  }
#ifdef SMTK_ENABLE_REMUS_SUPPORT
  else if (ext == ".map")
  {
    vtkNew<vtkCMBMapReader> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();

    vtkNew<vtkCMBTriangleMesher> trimesher;
    trimesher->SetPreserveEdgesAndNodes(true);
    trimesher->SetInputData(0, reader->GetOutputDataObject(0));
    trimesher->Update();

    m_mapOp->Operate(mod.GetPointer(), trimesher.GetPointer());
  }
  else if (ext == ".shp")
  {
    smtk::attribute::StringItem::Ptr boundaryItem =
      this->parameters()->findString("ShapeBoundaryStyle");
    if (boundaryItem->isEnabled())
    {
      vtkNew<vtkCMBGeometry2DReader> reader;
      reader->SetFileName(filename.c_str());
      std::string boundaryStyle = boundaryItem->value();
      if (boundaryStyle == "None") // default
      {
        reader->SetBoundaryStyle(vtkCMBGeometry2DReader::NONE);
      }
      else if (boundaryStyle == "Relative Margin")
      {
        reader->SetBoundaryStyle(vtkCMBGeometry2DReader::RELATIVE_MARGIN);
        smtk::attribute::StringItem::Ptr relMarginItem =
          this->parameters()->findString("relative margin");
        reader->SetRelativeMarginString(relMarginItem->value().c_str());
      }
      else if (boundaryStyle == "Absolute Margin")
      {
        reader->SetBoundaryStyle(vtkCMBGeometry2DReader::ABSOLUTE_MARGIN);
        smtk::attribute::StringItem::Ptr absMarginItem =
          this->parameters()->findString("absolute margin");
        reader->SetAbsoluteMarginString(absMarginItem->value().c_str());
      }
      else if (boundaryStyle == "Bounding Box")
      {
        reader->SetBoundaryStyle(vtkCMBGeometry2DReader::ABSOLUTE_BOUNDS);
        smtk::attribute::StringItem::Ptr absBoundsItem =
          this->parameters()->findString("absolute bounds");
        reader->SetAbsoluteBoundsString(absBoundsItem->value().c_str());
      }
      else if (boundaryStyle == "Bounding File")
      {
        reader->SetBoundaryStyle(vtkCMBGeometry2DReader::IMPORTED_POLYGON);
        smtk::attribute::StringItem::Ptr boundsFileItem =
          this->parameters()->findString("imported polygon");
        reader->SetBoundaryFile(boundsFileItem->value().c_str());
      }
      else
      {
        std::cerr << "Invalid Shape file boundary. No boundary will be set.\n";
        reader->SetBoundaryStyle(vtkCMBGeometry2DReader::NONE);
      }
      reader->Update();
      m_shpOp->Operate(mod.GetPointer(), reader.GetPointer(),
        /*cleanVerts:*/ 0);
    }
    else
      std::cerr << "Shape file boundary has to be set.\n";
  }
#endif
  else if (ext == ".vtk")
  {
    vtkNew<vtkPDataSetReader> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();

    vtkNew<vtkDataSetRegionSurfaceFilter> surface;
    surface->SetRegionArrayName(ModelParserHelper::GetShellTagName());
    surface->SetInputData(0, reader->GetOutputDataObject(0));
    surface->Update();

    vtkNew<vtkMasterPolyDataNormals> normals;
    normals->SetInputData(0, surface->GetOutputDataObject(0));
    normals->Update();

    vtkNew<vtkMergeDuplicateCells> merge;
    merge->SetModelRegionArrayName(ModelParserHelper::GetShellTagName());
    merge->SetModelFaceArrayName(ModelParserHelper::GetModelFaceTagName());
    merge->SetInputData(0, normals->GetOutputDataObject(0));
    merge->Update();

    m_op->Operate(mod.GetPointer(), merge.GetPointer());
  }

  // Now assign a UUID to the model and associate its filename with
  // a URL property (if things went OK).
  if (!m_op->GetOperateSucceeded()
#ifdef SMTK_ENABLE_REMUS_SUPPORT
    && !m_mapOp->GetOperateSucceeded() && !m_shpOp->GetOperateSucceeded()
#endif
        )
  {
    std::cerr << "Failed to import file \"" << filename << "\".\n";
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Now that the model is imported, we need to set the associated file name as
  // the .cmb file, rather than the original file from which the import
  // occurred. Otherwise, the imported file extension will be incorrectly used
  // when subsequently serializing to disk.
  filename = (vtksys::SystemTools::GetFilenameWithoutLastExtension(filename) + ".cmb");
  smtk::common::UUID modelId = session->trackModel(mod.GetPointer(), filename, resource);
  smtk::model::EntityRef modelEntity(resource, modelId);

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  {
    smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
    created->appendValue(modelEntity.component());
  }
  // for 2dm files model and mesh have same geometry,
  // so create meshes for faces and edges
  if (ext == ".2dm" || ext == ".3dm")
  {
    smtk::io::ModelToMesh convert;
    smtk::mesh::CollectionPtr c = convert(modelEntity.as<smtk::model::Model>());
    if (c->isValid() && c->numberOfMeshes() > 0)
    {
      if (c->name().empty())
      {
        c->name(vtksys::SystemTools::GetFilenameWithoutExtension(filename));
      }
      result->findComponent("mesh_created")->setValue(modelEntity.component());
    }
  }

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->setValue(resource);
  }

  auto resultModels = result->findComponent("model");
  resultModels->setValue(modelEntity.component());

  modelEntity.as<smtk::model::Model>().setSession(
    smtk::model::SessionRef(modelEntity.resource(), session->sessionId()));

  return result;
}

const char* ImportOperation::xmlDescription() const
{
  return ImportOperation_xml;
}

} // namespace discrete
} // namespace bridge
} // namespace smtk
