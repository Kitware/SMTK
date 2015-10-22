//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/exodus/ReadOperator.h"

#include "smtk/bridge/exodus/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "vtkExodusIIReader.h"
#include "vtkSLACReader.h"
#include "vtkFieldData.h"
#include "vtkDataArray.h"
#include "vtkStringArray.h"
#include "vtkInformation.h"

#include "boost/filesystem.hpp"

using namespace smtk::model;
using namespace smtk::common;
using namespace boost::filesystem;

namespace smtk {
  namespace bridge {
    namespace exodus {

smtk::model::OperatorResult ReadOperator::operateInternal()
{
  smtk::attribute::FileItem::Ptr filenameItem =
    this->specification()->findFile("filename");
  smtk::attribute::StringItem::Ptr filetypeItem =
    this->specification()->findString("filetype");

  std::string filename = filenameItem->value();
  std::string filetype = filetypeItem->value();

  if (filetype.empty())
    { // Infer file type from name
    std::string ext = path(filename).extension().string();
    if (ext == ".nc" || ext == ".ncdf")
      filetype = "slac";
    else if (ext == ".exo" || ext == ".g" || ext == ".ex2" || ext == ".exii")
      filetype = "exodus";
    }

  // Downcase the filetype (especially for when we did not infer it):
  std::transform(filetype.begin(), filetype.end(), filetype.begin(), ::tolower);

  if (filetype == "slac")
    return this->readSLAC();

  // The default is to assume it is an Exodus file:
  return this->readExodus();
}

static void MarkMeshInfo(
  vtkDataObject* data, int dim, const char* name, EntityType etype, int pedigree)
{
  if (!data)
    return; // Skip empty leaf nodes

  vtkInformation* info = data->GetInformation();
  info->Set(Session::SMTK_DIMENSION(), dim);
  info->Set(Session::SMTK_GROUP_TYPE(), etype);
  info->Set(vtkCompositeDataSet::NAME(), name);

  // If a UUID has been saved to field data, we should copy it to the info object here.
  vtkStringArray* uuidArr =
    vtkStringArray::SafeDownCast(
      data->GetFieldData()->GetAbstractArray("UUID"));
  if (uuidArr && uuidArr->GetNumberOfTuples() > 0)
    info->Set(Session::SMTK_UUID_KEY(), uuidArr->GetValue(0).c_str());

  info->Set(Session::SMTK_PEDIGREE(), pedigree);
}

static void MarkExodusMeshWithChildren(
  vtkMultiBlockDataSet* data, int dim, const char* name, EntityType etype, EntityType childType,
  vtkExodusIIReader* rdr, vtkExodusIIReader::ObjectType rdrIdType)
{
  MarkMeshInfo(data, dim, name, etype, -1);
  int nb = data->GetNumberOfBlocks();
  for (int i = 0; i < nb; ++i)
    {
    std::ostringstream autoName;
    autoName << EntityTypeNameString(childType) << " " << i;
    const char* name = data->GetMetaData(i)->Get(vtkCompositeDataSet::NAME());
    int pedigree = rdr->GetObjectId(rdrIdType, i);
    MarkMeshInfo(data->GetBlock(i), dim, name && name[0] ? name : autoName.str().c_str(), childType, pedigree);
    }
}

static void MarkSLACMeshWithChildren(
  vtkMultiBlockDataSet* data, int dim, const char* name, EntityType etype, EntityType childType)
{
  MarkMeshInfo(data, dim, name, etype, -1);
  int nb = data->GetNumberOfBlocks();
  for (int i = 0; i < nb; ++i)
    {
    std::ostringstream autoName;
    autoName << EntityTypeNameString(childType) << " " << i;
    const char* name = data->GetMetaData(i)->Get(vtkCompositeDataSet::NAME());
    MarkMeshInfo(data->GetBlock(i), dim, name && name[0] ? name : autoName.str().c_str(), childType, i);
    }
}

smtk::model::OperatorResult ReadOperator::readExodus()
{
  smtk::attribute::FileItem::Ptr filenameItem =
    this->specification()->findFile("filename");

  std::string filename = filenameItem->value();

  vtkNew<vtkExodusIIReader> rdr;
  rdr->SetFileName(filenameItem->value(0).c_str());
  rdr->UpdateInformation();
  // Turn on all side and node sets.
  vtkExodusIIReader::ObjectType set_types[] = {
    vtkExodusIIReader::SIDE_SET,
    vtkExodusIIReader::NODE_SET,
    vtkExodusIIReader::ELEM_BLOCK
  };
  const int num_set_types = sizeof(set_types) / sizeof(set_types[0]);
  for (int j = 0; j < num_set_types; ++j)
    for (int i = 0; i < rdr->GetNumberOfObjects(set_types[j]); ++i)
      rdr->SetObjectStatus(set_types[j], i, 1);

  // Read in the data (so we can obtain tessellation info)
  rdr->Update();
  vtkSmartPointer<vtkMultiBlockDataSet> modelOut =
    vtkSmartPointer<vtkMultiBlockDataSet>::New();
  modelOut->ShallowCopy(
    vtkMultiBlockDataSet::SafeDownCast(
      rdr->GetOutputDataObject(0)));
  int dim = rdr->GetDimensionality();

  // Now iterate over the dataset and mark each block (leaf or not)
  // with information needed by the session to determine how it should
  // be presented.
  MarkMeshInfo(modelOut, dim, path(filename).stem().c_str(), EXO_MODEL, -1);
  vtkMultiBlockDataSet* elemBlocks =
    vtkMultiBlockDataSet::SafeDownCast(
      modelOut->GetBlock(0));
  MarkExodusMeshWithChildren(
    elemBlocks, dim,
    modelOut->GetMetaData(0u)->Get(vtkCompositeDataSet::NAME()),
    EXO_BLOCKS, EXO_BLOCK, rdr.GetPointer(), vtkExodusIIReader::ELEM_BLOCK);

  vtkMultiBlockDataSet* sideSets =
    vtkMultiBlockDataSet::SafeDownCast(
      modelOut->GetBlock(4));
  MarkExodusMeshWithChildren(
    sideSets, dim - 1,
    modelOut->GetMetaData(4)->Get(vtkCompositeDataSet::NAME()),
    EXO_SIDE_SETS, EXO_SIDE_SET, rdr.GetPointer(), vtkExodusIIReader::SIDE_SET);

  vtkMultiBlockDataSet* nodeSets =
    vtkMultiBlockDataSet::SafeDownCast(
      modelOut->GetBlock(7));
  MarkExodusMeshWithChildren(
    nodeSets, 0,
    modelOut->GetMetaData(7)->Get(vtkCompositeDataSet::NAME()),
    EXO_NODE_SETS, EXO_NODE_SET, rdr.GetPointer(), vtkExodusIIReader::NODE_SET);


  // Now that the datasets we wish to present are marked,
  // have the Session create entries in the model manager for us:
  Session* brdg = this->exodusSession();
  smtk::model::Model smtkModelOut =
    brdg->addModel(modelOut);

  smtkModelOut.setStringProperty("url", filename);
  smtkModelOut.setStringProperty("type", "exodus");

  // Now set model for session and transcribe everything.
  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  smtk::attribute::ModelEntityItem::Ptr resultModels =
    result->findModelEntity("model");
  resultModels->setValue(smtkModelOut);
  smtk::attribute::ModelEntityItem::Ptr created =
    result->findModelEntity("created");
  created->setNumberOfValues(1);
  created->setValue(smtkModelOut);
  created->setIsEnabled(true);

  return result;
}

smtk::model::OperatorResult ReadOperator::readSLAC()
{
  smtk::attribute::FileItem::Ptr filenameItem =
    this->specification()->findFile("filename");

  smtk::attribute::IntItem::Ptr readVolumes =
    this->specification()->findInt("readSLACVolumes");

  std::string filename = filenameItem->value();

  vtkNew<vtkSLACReader> rdr;
  rdr->SetMeshFileName(filenameItem->value(0).c_str());

  rdr->SetReadInternalVolume(readVolumes->discreteIndex());
  rdr->ReadExternalSurfaceOn();
  rdr->ReadMidpointsOn();

  // Read in the data (so we can obtain tessellation info)
  rdr->Update();

  vtkSmartPointer<vtkMultiBlockDataSet> modelOut =
    vtkSmartPointer<vtkMultiBlockDataSet>::New();
  vtkNew<vtkMultiBlockDataSet> surfBlocks;
  surfBlocks->ShallowCopy(
    vtkMultiBlockDataSet::SafeDownCast(
      rdr->GetOutputDataObject(0)));
  vtkNew<vtkMultiBlockDataSet> voluBlocks;
  voluBlocks->ShallowCopy(
    vtkMultiBlockDataSet::SafeDownCast(
      rdr->GetOutputDataObject(1)));

  modelOut->SetNumberOfBlocks(2);
  modelOut->SetBlock(0, surfBlocks.GetPointer());
  modelOut->SetBlock(1, voluBlocks.GetPointer());

  MarkMeshInfo(modelOut.GetPointer(), 3, path(filename).stem().c_str(), EXO_MODEL, -1);
  MarkSLACMeshWithChildren(surfBlocks.GetPointer(), 2, "surfaces", EXO_SIDE_SETS, EXO_SIDE_SET);
  MarkSLACMeshWithChildren(voluBlocks.GetPointer(), 3, "volumes", EXO_BLOCKS, EXO_BLOCK);

  Session* brdg = this->exodusSession();
  smtk::model::Model smtkModelOut =
    brdg->addModel(modelOut);
  smtkModelOut.setStringProperty("url", filename);
  smtkModelOut.setStringProperty("type", "slac");

  // Now set model for session and transcribe everything.
  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  smtk::attribute::ModelEntityItem::Ptr resultModels =
    result->findModelEntity("model");
  resultModels->setValue(smtkModelOut);
  smtk::attribute::ModelEntityItem::Ptr created =
    result->findModelEntity("created");
  created->setNumberOfValues(1);
  created->setValue(smtkModelOut);
  created->setIsEnabled(true);

  /*
  // The side and node sets now exist; go through
  // and use the Exodus reader's private information
  // to correct the property information.
  Groups groups = smtkModelOut.groups();
  for (Groups::iterator git = groups.begin(); git != groups.end(); ++git)
    {
    int oid;
    EntityHandle handle = this->exodusHandle(*git);
    switch (handle.entityType)
      {
    case EXO_BLOCK:
      oid = rdr->GetObjectId(vtkExodusIIReader::ELEM_BLOCK, handle.entityId);
      git->setStringProperty("exodus type", "element block");
      git->setMembershipMask(DIMENSION_3 | MODEL_DOMAIN);
      break;
    case EXO_NODE_SET:
      oid = rdr->GetObjectId(vtkExodusIIReader::NODE_SET, handle.entityId);
      git->setStringProperty("exodus type", "node set");
      git->setMembershipMask(DIMENSION_0 | MODEL_BOUNDARY);
      break;
    case EXO_SIDE_SET:
      oid = rdr->GetObjectId(vtkExodusIIReader::SIDE_SET, handle.entityId);
      git->setStringProperty("exodus type", "side set");
      git->setMembershipMask(ANY_DIMENSION | MODEL_BOUNDARY);
      break;
    case EXO_MODEL:
    default:
      continue; // skip
      }
    git->setIntegerProperty("exodus id", oid);
    }
    */

  return result;
}

    } // namespace exodus
  } //namespace bridge
} // namespace smtk

#include "smtk/bridge/exodus/ReadOperator_xml.h"
#include "smtk/bridge/exodus/Exports.h"

smtkImplementsModelOperator(
  SMTKEXODUSSESSION_EXPORT,
  smtk::bridge::exodus::ReadOperator,
  exodus_read,
  "read",
  ReadOperator_xml,
  smtk::bridge::exodus::Session);
