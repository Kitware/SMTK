//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "ImportOperator.h"

#include "smtk/bridge/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/io/ModelToMesh.h"
#include "smtk/mesh/Collection.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/SessionRef.h"

#include "vtkDiscreteModelWrapper.h"
#include "vtkModelItem.h"
#include "vtkModel.h"
#include "vtkPDataSetReader.h"
#include "vtkMasterPolyDataNormals.h"
#include "vtkMergeDuplicateCells.h"
#include "vtkDataSetRegionSurfaceFilter.h"
#include "smtk/extension/vtk/reader/vtkCMBGeometryReader.h"

#ifdef SMTK_ENABLE_REMUS_SUPPORT
  #include "smtk/extension/vtk/reader/vtkCMBGeometry2DReader.h"
  #include "smtk/extension/vtk/reader/vtkCMBMapReader.h"
  #include "smtk/extension/vtk/meshing/vtkCMBTriangleMesher.h"
#endif

#ifdef SMTK_ENABLE_MOAB_DISCRETE_READER
#include "smtk/bridge/discrete/moabreader/vtkCmbMoabReader.h"
#endif

#include <vtksys/SystemTools.hxx>
#include "ModelParserHelper.h"
#include "ImportOperator_xml.h"

#include "smtk/io/ExportJSON.h"
#include "cJSON.h"

using namespace smtk::model;

namespace smtk {
  namespace bridge {

  namespace discrete {

ImportOperator::ImportOperator()
{
}

bool ImportOperator::ableToOperate()
{
  if(!this->specification()->isValid())
    return false;

  std::string filename = this->specification()->findFile("filename")->value();
  if (filename.empty())
    return false;
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(filename);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  bool able = (ext == ".vtk" || ext == ".2dm" ||
               ext == ".3dm" ||
#ifdef SMTK_ENABLE_MOAB_DISCRETE_READER
               ext == ".h5m" || ext == ".sat" ||
               ext == ".brep" || ext == ".stp" ||
               ext == ".cub" || ext == ".exo" ||
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
  if(ext == ".shp")
    {
    smtk::attribute::StringItem::Ptr boundaryItem =
      this->specification()->findString("ShapeBoundaryStyle");
    able = boundaryItem->isEnabled();
    }
#endif

  return able;
}

OperatorResult ImportOperator::operateInternal()
{
/*
  smtk::attribute::FileItem::Ptr filenameItem =
    this->specification()->findFile("filename");
  smtk::attribute::StringItem::Ptr filetypeItem =
    this->specification()->findString("filetype");

  std::string filename = filenameItem ?
    filenameItem->value() : "";
//  std::string filetype = filetypeItem ?
//    filetypeItem->value() : "";
*/
  std::string filename = this->specification()->findFile("filename")->value();

/*
  cJSON* json = cJSON_CreateObject();
  smtk::io::ExportJSON::forOperator(this->specification(), json);
  std::cout << "Import Operator: " << cJSON_Print(json) << "\n";
  cJSON_Delete(json);

*/
  if (filename.empty())
    {
    std::cerr << "File name is empty!\n";
    return this->createResult(OPERATION_FAILED);
    }

  // Create a new model to hold the result.
  vtkNew<vtkDiscreteModelWrapper> mod;

// ******************************************************************************
// This is where we should have the logic to import files other than .cmb formats
// ******************************************************************************
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(filename);
  if(ext == ".h5m" || ext == ".sat" ||
     ext == ".brep" || ext == ".stp" ||
     ext == ".cub" || ext == ".exo" )
    {
#ifdef SMTK_ENABLE_MOAB_DISCRETE_READER

    vtkNew<vtkCmbMoabReader> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();

    this->m_op->Operate(mod.GetPointer(), reader.GetPointer());
#endif
    }
  else if (ext == ".2dm" ||
      ext == ".3dm" ||
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

    if(ext == ".poly" || ext == ".smesh" || hasBoundaryEdges)
      {
      this->m_op->Operate(mod.GetPointer(), reader.GetPointer());
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

      this->m_op->Operate(mod.GetPointer(), merge.GetPointer());
      }
    }
#ifdef SMTK_ENABLE_REMUS_SUPPORT
  else if(ext == ".map")
    {
    vtkNew<vtkCMBMapReader> reader;
    reader->SetFileName(filename.c_str());
    reader->Update();

    vtkNew<vtkCMBTriangleMesher> trimesher;
    trimesher->SetPreserveEdgesAndNodes(true);
    trimesher->SetInputData(0, reader->GetOutputDataObject(0));
    trimesher->Update();

    this->m_mapOp->Operate(mod.GetPointer(), trimesher.GetPointer());
    }
  else if(ext == ".shp")
    {
    smtk::attribute::StringItem::Ptr boundaryItem =
      this->specification()->findString("ShapeBoundaryStyle");
    if(boundaryItem->isEnabled())
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
          this->specification()->findString("relative margin");
        reader->SetRelativeMarginString(relMarginItem->value().c_str());
        }
      else if (boundaryStyle == "Absolute Margin")
        {
        reader->SetBoundaryStyle(vtkCMBGeometry2DReader::ABSOLUTE_MARGIN);
        smtk::attribute::StringItem::Ptr absMarginItem =
          this->specification()->findString("absolute margin");
        reader->SetAbsoluteMarginString(absMarginItem->value().c_str());
        }
      else if (boundaryStyle == "Bounding Box")
        {
        reader->SetBoundaryStyle(vtkCMBGeometry2DReader::ABSOLUTE_BOUNDS);
        smtk::attribute::StringItem::Ptr absBoundsItem =
          this->specification()->findString("absolute bounds");
        reader->SetAbsoluteBoundsString(absBoundsItem->value().c_str());
        }
      else if (boundaryStyle == "Bounding File")
        {
        reader->SetBoundaryStyle(vtkCMBGeometry2DReader::IMPORTED_POLYGON);
        smtk::attribute::StringItem::Ptr boundsFileItem =
          this->specification()->findString("imported polygon");
        reader->SetBoundaryFile(boundsFileItem->value().c_str());
        }
      else
        {
        std::cerr << "Invalid Shape file boundary. No boundary will be set.\n";
        reader->SetBoundaryStyle(vtkCMBGeometry2DReader::NONE);
        }
      reader->Update();
      this->m_shpOp->Operate(mod.GetPointer(), reader.GetPointer(),
                             /*cleanVerts:*/ 0);
      }
    else
      std::cerr << "Shape file boundary has to be set.\n";
    }
#endif
  else if(ext == ".vtk")
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

    this->m_op->Operate(mod.GetPointer(), merge.GetPointer());
    }

  // Now assign a UUID to the model and associate its filename with
  // a URL property (if things went OK).
  if (!this->m_op->GetOperateSucceeded()
#ifdef SMTK_ENABLE_REMUS_SUPPORT
   && !this->m_mapOp->GetOperateSucceeded()
   && !this->m_shpOp->GetOperateSucceeded()
#endif
   )
    {
    std::cerr << "Failed to import file \"" << filename << "\".\n";
    return this->createResult(OPERATION_FAILED);
    }

  smtk::common::UUID modelId = this->discreteSession()->trackModel(
    mod.GetPointer(), filename, this->manager());
  smtk::model::EntityRef modelEntity(this->manager(), modelId);

  OperatorResult result = this->createResult(OPERATION_SUCCEEDED);
  this->addEntityToResult(result, modelEntity, CREATED);
  // for 2dm files model and mesh have same geometry,
  // so create meshes for faces and edges
  if (ext == ".2dm" || ext == ".3dm")
    {
    smtk::io::ModelToMesh convert;
    smtk::mesh::CollectionPtr c = convert(modelEntity.as<smtk::model::Model>());
    if(c->isValid() && c->numberOfMeshes() > 0)
      {
      if(c->name().empty())
        {
        c->name("original_mesh");
        }
      result->findModelEntity("mesh_created")->setValue(modelEntity);
      }
    }

  modelEntity.as<smtk::model::Model>().setSession(
    smtk::model::SessionRef(
      modelEntity.manager(),
      this->session()->sessionId()));

  return result;
}

Session* ImportOperator::discreteSession() const
{
  return dynamic_cast<Session*>(this->session());
}

    } // namespace discrete
  } // namespace bridge

} // namespace smtk

smtkImplementsModelOperator(
  SMTKDISCRETESESSION_EXPORT,
  smtk::bridge::discrete::ImportOperator,
  discrete_import,
  "import",
  ImportOperator_xml,
  smtk::bridge::discrete::Session);
