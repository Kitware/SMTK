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

#include "smtk/bridge/exodus/Bridge.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/GroupEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/ModelEntity.h"

#include "vtkExodusIIReader.h"

using namespace smtk::model;
using namespace smtk::common;

namespace smtk {
  namespace bridge {
    namespace exodus {

bool ReadOperator::ableToOperate()
{
  return this->specification()->isValid();
}

smtk::model::OperatorResult ReadOperator::operateInternal()
{
  smtk::attribute::FileItem::Ptr filenameItem =
    this->specification()->findFile("filename");
  smtk::attribute::StringItem::Ptr filetypeItem =
    this->specification()->findString("filetype");

  std::string filename = filenameItem->value();
  std::string filetype = filetypeItem->value();

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
  Bridge* brdg = this->exodusBridge();
  smtk::model::ModelEntity smtkModelOut =
    brdg->addModel(modelOut);

  // Now set model for bridge and transcribe everything.
  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  smtk::attribute::ModelEntityItem::Ptr resultModels =
    result->findModelEntity("model");
  resultModels->setValue(smtkModelOut);

  // The side and node sets now exist; go through
  // and use the Exodus reader's private information
  // to add property information.
  GroupEntities groups = smtkModelOut.groups();
  for (GroupEntities::iterator git = groups.begin(); git != groups.end(); ++git)
    {
    int oid;
    EntityHandle handle = this->exodusHandle(*git);
    switch (handle.entityType)
      {
    case EXO_BLOCK:
      oid = rdr->GetObjectId(vtkExodusIIReader::ELEM_BLOCK, handle.entityId);
      git->setStringProperty("exodus type", "element block");
      break;
    case EXO_NODE_SET:
      oid = rdr->GetObjectId(vtkExodusIIReader::NODE_SET, handle.entityId);
      git->setStringProperty("exodus type", "node set");
      break;
    case EXO_SIDE_SET:
      oid = rdr->GetObjectId(vtkExodusIIReader::SIDE_SET, handle.entityId);
      git->setStringProperty("exodus type", "side set");
      break;
    case EXO_MODEL:
    default:
      continue; // skip
      }
    git->setIntegerProperty("exodus id", oid);
    }

  return result;
}

    } // namespace exodus
  } //namespace bridge
} // namespace smtk

#include "smtk/bridge/exodus/ReadOperator_xml.h"

smtkImplementsModelOperator(
  smtk::bridge::exodus::ReadOperator,
  exodus_read,
  "read",
  ReadOperator_xml,
  smtk::bridge::exodus::Bridge);
