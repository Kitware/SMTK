//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/vtk/operators/Import.h"

#include "smtk/session/vtk/Resource.h"
#include "smtk/session/vtk/Session.h"
#include "smtk/session/vtk/operators/Import_xml.h"

#include "smtk/extension/vtk/source/vtkResourceMultiBlockSource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/io/Logger.h"

#include "smtk/model/Group.h"
#include "smtk/model/Model.h"

#include "smtk/common/Paths.h"
#include "smtk/common/ThreadPool.h"
#include "smtk/common/UUID.h"

#include "vtkContourFilter.h"
#include "vtkDataArray.h"
#include "vtkDataSetAttributes.h"
#include "vtkExodusIIReader.h"
#include "vtkFieldData.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkPointData.h"
#include "vtkSLACReader.h"
#include "vtkStringArray.h"
#include "vtkThreshold.h"
#include "vtkTypeInt32Array.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkXMLImageDataReader.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

using namespace smtk::model;
using namespace smtk::common;
using namespace boost::filesystem;

namespace smtk
{
namespace session
{
namespace vtk
{

Import::Result Import::operateInternal()
{
  smtk::attribute::FileItem::Ptr filenameItem = this->parameters()->findFile("filename");
  smtk::attribute::StringItem::Ptr filetypeItem = this->parameters()->findString("filetype");

  std::string filename = filenameItem->value();
  std::string filetype = filetypeItem->isEnabled() ? filetypeItem->value() : std::string();

  if (filetype.empty())
  { // Infer file type from name
    std::string ext = path(filename).extension().string();
    if (ext == ".nc" || ext == ".ncdf")
      filetype = "slac";
    else if (ext == ".vti")
      filetype = "label map";
    else if (ext == ".exo" || ext == ".g" || ext == ".gen" || ext == ".ex2" || ext == ".exii")
      filetype = "exodus";
  }

  // There are three possible import modes
  //
  // 1. Import a mesh into an existing resource
  // 2. Import a mesh as a new model, but using the session of an existing resource
  // 3. Import a mesh into a new resource

  smtk::session::vtk::Resource::Ptr resource = nullptr;
  smtk::session::vtk::Session::Ptr session = nullptr;

  // Modes 2 and 3 requre an existing resource for input
  smtk::attribute::ReferenceItem::Ptr existingReferenceItem = this->parameters()->associations();

  if (existingReferenceItem && existingReferenceItem->numberOfValues() > 0)
  {
    smtk::session::vtk::Resource::Ptr existingResource =
      std::static_pointer_cast<smtk::session::vtk::Resource>(existingReferenceItem->value());

    session = existingResource->session();

    smtk::attribute::StringItem::Ptr sessionOnlyItem =
      this->parameters()->findString("session only");
    if (sessionOnlyItem->value() == "import into this file")
    {
      // If the "session only" value is set to "import into this file",
      // then we use the existing resource
      resource = existingResource;
    }
    else
    {
      // If the "session only" value is set to "import into this session",
      // then we create a new resource with the session from the exisiting resource
      resource = smtk::session::vtk::Resource::create();
      resource->setSession(session);
    }
  }
  else
  {
    // If no existing resource is provided, then we create a new session and
    // resource.
    resource = smtk::session::vtk::Resource::create();
    session = smtk::session::vtk::Session::create();

    // Create a new resource for the import
    resource->setSession(session);
  }

  std::string potentialName = smtk::common::Paths::stem(filename);
  if (resource->name().empty() && !potentialName.empty())
  {
    resource->setName(potentialName);
  }

  // Downcase the filetype (especially for when we did not infer it):
  std::transform(filetype.begin(), filetype.end(), filetype.begin(), ::tolower);

  if (filetype == "slac")
    return this->importSLAC(resource);
  else if (filetype == "label map")
    return this->importLabelMap(resource);

  // The default is to assume it is an Exodus file:
  return this->importExodus(resource);
}

static void AddPreservedUUID(
  vtkDataObject* data,
  int& curId,
  vtkDataObject* parentData,
  int idxInParent,
  int modelNumber,
  const std::vector<smtk::common::UUID>& uuids,
  const SessionPtr& session,
  const ResourcePtr& resource)
{
  if (!data || curId < 0 || static_cast<std::size_t>(curId) >= uuids.size())
    return;

  // Assign the preserved UUID to the data's information.
  vtkInformation* info = data->GetInformation();
  vtkResourceMultiBlockSource::SetDataObjectUUID(
    info, smtk::common::UUID(uuids.at(curId).toString()));
  EntityHandle handle(modelNumber, data, parentData, idxInParent, session);
  EntityRef eref(resource, uuids.at(curId));
  session->reverseIdMap()[eref] = handle;
  smtk::operation::MarkGeometry(resource).markModified(eref.component());
  ++curId;
}

static void AddPreservedUUIDsRecursive(
  vtkDataObject* data,
  int& curId,
  vtkDataObject* parentData,
  int idxInParent,
  int modelNumber,
  const std::vector<smtk::common::UUID>& uuids,
  const SessionPtr& session,
  const ResourcePtr& resource)
{
  AddPreservedUUID(data, curId, parentData, idxInParent, modelNumber, uuids, session, resource);

  vtkMultiBlockDataSet* mbds = vtkMultiBlockDataSet::SafeDownCast(data);
  if (mbds)
  {
    int nb = mbds->GetNumberOfBlocks();
    for (int i = 0; i < nb; ++i)
    {
      AddPreservedUUIDsRecursive(
        mbds->GetBlock(i), curId, data, i, modelNumber, uuids, session, resource);
    }
  }
}

static smtk::model::Model addModel(
  vtkSmartPointer<vtkMultiBlockDataSet>& modelOut,
  const std::vector<smtk::common::UUID>& uuids,
  const SessionPtr& session,
  const ResourcePtr& resource)
{
  // If we are reading a vtk session resource (as opposed to a new import), we
  // should access the existing model instead of creating a new one here. If
  // this is the case, then the first preserved id will be related to a model
  // entity that is already in the resource (as it was put there by the Read
  // operation calling this one).

  // A default-constructed model is invalid.
  smtk::model::Model smtkModelOut;
  if (!uuids.empty())
  {
    // The first id in the list of preserved ids is for the containing model.
    smtkModelOut = smtk::model::Model(resource, uuids.at(0));

    // The resulting model may be invalid if the model was not successfully
    // constructed in the parent Read operation (e.g. for LegacyRead). In this
    // situation, we can construct the model here.
    if (!smtkModelOut.isValid())
    {
      // First set the UUID on the multiblock dataset representing the model.
      // This way, the resulting smtk model will have the right UUID.
      vtkInformation* info = modelOut->GetInformation();
      vtkResourceMultiBlockSource::SetDataObjectUUID(info, uuids.at(0));
      smtkModelOut = session->addModel(modelOut, smtk::model::SESSION_EVERYTHING);
    }

    // We are about to add a model. Its number will be the next index in the
    // session's vector of models.
    int modelNumber = static_cast<int>(session->numberOfModels());

    // Add the backend information associated to the model, but don't bother
    // transcribing it.
    smtkModelOut = session->addModel(modelOut, smtk::model::SESSION_NOTHING);

    // Assign preserved UUIDs, link the vtk data object to the smtk component
    // and construct tessellations for the imported components.
    int curId = 0;
    AddPreservedUUIDsRecursive(modelOut, curId, nullptr, -1, modelNumber, uuids, session, resource);
  }
  else
  {
    // This is a proper import (there are no preexisitng UUIDs), so we can let
    // the Session create entries in the model resource for us.
    smtkModelOut = session->addModel(modelOut, smtk::model::SESSION_EVERYTHING);
  }

  return smtkModelOut;
}

static void AddBlockChildrenAsModelChildren(vtkMultiBlockDataSet* data)
{
  if (!data)
    return;

  std::vector<vtkObjectBase*> children;
  auto* iter = data->NewTreeIterator();
  iter->VisitOnlyLeavesOn();
  for (iter->GoToFirstItem(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
  {
    vtkDataObject* obj = iter->GetCurrentDataObject();
    if (obj)
    {
      children.push_back(obj);
    }
  }
  iter->Delete();
  Session::SMTK_CHILDREN()->SetRange(
    data->GetInformation(), children.data(), 0, 0, static_cast<int>(children.size()));
}

static void
MarkMeshInfo(vtkDataObject* data, int dim, const char* name, EntityType etype, int pedigree)
{
  if (!data)
    return; // Skip empty leaf nodes

  vtkInformation* info = data->GetInformation();
  info->Set(Session::SMTK_DIMENSION(), dim);
  info->Set(Session::SMTK_GROUP_TYPE(), etype);
  info->Set(vtkCompositeDataSet::NAME(), name);

  auto existingUUID = vtkResourceMultiBlockSource::GetDataObjectUUID(info);
  if (!existingUUID)
  {
    // ++ 1 ++
    // If a UUID has been saved to field data, we should copy it to the info object here.
    vtkStringArray* uuidArr =
      vtkStringArray::SafeDownCast(data->GetFieldData()->GetAbstractArray("UUID"));
    if (uuidArr && uuidArr->GetNumberOfTuples() > 0)
      vtkResourceMultiBlockSource::SetDataObjectUUID(
        info, smtk::common::UUID(uuidArr->GetValue(0).c_str()));
    // -- 1 --
  }

  info->Set(Session::SMTK_PEDIGREE(), pedigree);
}

static void MarkSLACMeshInfo(
  vtkDataObject* data,
  int dim,
  const char* name,
  vtkInformation* meta,
  EntityType etype,
  int pedigree)
{
  const char* name2 = meta->Get(vtkCompositeDataSet::NAME());
  if (name2 && name2[0])
  {
    MarkMeshInfo(data, dim, name2, etype, pedigree);
  }
  else
  {
    std::ostringstream autoName;
    autoName << name << " " << pedigree;
    MarkMeshInfo(data, dim, autoName.str().c_str(), etype, pedigree);
  }
}

static vtkSmartPointer<vtkMultiBlockDataSet> FlattenBlocks(
  vtkMultiBlockDataSet** blocks,
  std::size_t nblk)
{
  auto modelOut = vtkSmartPointer<vtkMultiBlockDataSet>::New();
  if (!blocks)
  {
    return modelOut;
  }

  vtkIdType nbo = 0;
  for (std::size_t bb = 0; bb < nblk; ++bb)
  {
    if (!blocks[bb])
    {
      continue;
    }
    nbo += blocks[bb]->GetNumberOfBlocks();
  }
  modelOut->SetNumberOfBlocks(nbo);
  return modelOut;
}

static void FillAndMarkBlocksFromSrc(
  vtkMultiBlockDataSet* modelOut,
  vtkIdType& ii,
  vtkMultiBlockDataSet* src,
  const char* srcName,
  EntityType srcType,
  std::function<int(vtkIdType)> pedigreeFn = [](vtkIdType zz) { return static_cast<int>(zz); })
{
  vtkIdType nbi = src->GetNumberOfBlocks();
  for (vtkIdType jj = 0; jj < nbi; ++jj, ++ii)
  {
    auto* blk = src->GetBlock(jj);
    int srcDim = 0;
    vtkDataSet* dataSet = vtkDataSet::SafeDownCast(blk);
    {
      if (dataSet != nullptr && dataSet->GetNumberOfCells() > 0)
      {
        srcDim = dataSet->GetCell(0)->GetCellDimension();
      }
    }
    modelOut->SetBlock(ii, blk);
    if (src->HasMetaData(jj))
    {
      modelOut->GetMetaData(ii)->Copy(src->GetMetaData(jj), 1);
    }
    if (blk)
    {
      MarkSLACMeshInfo(blk, srcDim, srcName, modelOut->GetMetaData(ii), srcType, pedigreeFn(jj));
    }
  }
}

namespace
{
vtkSmartPointer<vtkMultiBlockDataSet> importExodusInternal(const std::string filename)
{
  vtkNew<vtkExodusIIReader> rdr;
  rdr->SetFileName(filename.c_str());
  rdr->UpdateInformation();
  // Turn on all side and node sets.
  vtkExodusIIReader::ObjectType set_types[] = { vtkExodusIIReader::SIDE_SET,
                                                vtkExodusIIReader::NODE_SET,
                                                vtkExodusIIReader::ELEM_BLOCK };
  const int num_set_types = sizeof(set_types) / sizeof(set_types[0]);
  for (int j = 0; j < num_set_types; ++j)
    for (int i = 0; i < rdr->GetNumberOfObjects(set_types[j]); ++i)
      rdr->SetObjectStatus(set_types[j], i, 1);

  // Read in the data (so we can obtain tessellation info)
  rdr->Update();

  int dim = rdr->GetDimensionality();
  auto* topIn = vtkMultiBlockDataSet::SafeDownCast(rdr->GetOutputDataObject(0));
  if (!topIn || !vtkMultiBlockDataSet::SafeDownCast(topIn->GetBlock(0)))
  {
    return vtkSmartPointer<vtkMultiBlockDataSet>();
  }

  vtkMultiBlockDataSet* blocks[] = { vtkMultiBlockDataSet::SafeDownCast(topIn->GetBlock(0)),
                                     vtkMultiBlockDataSet::SafeDownCast(topIn->GetBlock(4)),
                                     vtkMultiBlockDataSet::SafeDownCast(topIn->GetBlock(7)) };
  vtkSmartPointer<vtkMultiBlockDataSet> modelOut =
    FlattenBlocks(blocks, sizeof(blocks) / sizeof(blocks[0]));
  vtkIdType ii = 0;
  FillAndMarkBlocksFromSrc(
    modelOut, ii, blocks[0], "element block", EXO_BLOCK, [&rdr](vtkIdType pp) {
      return rdr->GetObjectId(vtkExodusIIReader::ELEM_BLOCK, pp);
    });
  FillAndMarkBlocksFromSrc(modelOut, ii, blocks[1], "side set", EXO_SIDE_SET, [&rdr](vtkIdType pp) {
    return rdr->GetObjectId(vtkExodusIIReader::SIDE_SET, pp);
  });
  FillAndMarkBlocksFromSrc(modelOut, ii, blocks[2], "node set", EXO_NODE_SET, [&rdr](vtkIdType pp) {
    return rdr->GetObjectId(vtkExodusIIReader::NODE_SET, pp);
  });

  MarkMeshInfo(modelOut, dim, path(filename).stem().string<std::string>().c_str(), EXO_MODEL, -1);
  AddBlockChildrenAsModelChildren(modelOut);

  return modelOut;
}
} // namespace

Import::Result Import::importExodus(const smtk::session::vtk::Resource::Ptr& resource)
{
  const auto& session = resource->session();

  smtk::attribute::FileItem::Ptr filenameItem = this->parameters()->findFile("filename");

  if (filenameItem->numberOfValues() > 1 && !this->m_preservedUUIDs.empty())
  {
    smtkErrorMacro(this->log(), "UUID-preserving import from multiple files is not yet supported.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  std::vector<vtkSmartPointer<vtkMultiBlockDataSet>> modelsOut(filenameItem->numberOfValues());
  if (filenameItem->numberOfValues() > 1)
  {
    smtk::common::ThreadPool<vtkSmartPointer<vtkMultiBlockDataSet>> threadPool;
    std::vector<std::future<vtkSmartPointer<vtkMultiBlockDataSet>>> futures;

    for (std::size_t i = 0; i < modelsOut.size(); ++i)
    {
      std::string filename = filenameItem->value();
      futures.push_back(threadPool(std::bind(importExodusInternal, filename)));
    }

    for (std::size_t i = 0; i < modelsOut.size(); ++i)
    {
      modelsOut[i] = futures[i].get();

      if (modelsOut[i] == nullptr)
      {
        smtkErrorMacro(
          this->log(), "Error:Associated file " << filenameItem->value(i) << " is not valid!");
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }
    }
  }
  else
  {
    std::string filename = filenameItem->value();
    modelsOut[0] = importExodusInternal(filename);
  }

  // Now set model for session and transcribe everything.
  Import::Result result = this->createResult(Import::Outcome::SUCCEEDED);

  for (std::size_t i = 0; i < modelsOut.size(); ++i)
  {
    smtk::model::Model smtkModelOut =
      addModel(modelsOut[i], this->m_preservedUUIDs, session, resource);
    if (this->m_preservedUUIDs.empty())
    {
      smtkModelOut.setStringProperty("url", filenameItem->value(i));
      smtkModelOut.setStringProperty("type", "exodus");
    }

    {
      smtk::attribute::ComponentItem::Ptr resultModels = result->findComponent("model");
      resultModels->appendValue(smtkModelOut.component());
    }

    {
      smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
      created->appendValue(smtkModelOut.component());
    }
  }

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->appendValue(resource);
  }

  return result;
}

Import::Result Import::importSLAC(const smtk::session::vtk::Resource::Ptr& resource)
{
  const auto& session = resource->session();

  smtk::attribute::FileItem::Ptr filenameItem = this->parameters()->findFile("filename");

  smtk::attribute::IntItem::Ptr readVolumes = this->parameters()->findInt("readSLACVolumes");

  std::string filename = filenameItem->value();

  vtkNew<vtkSLACReader> rdr;
  rdr->SetMeshFileName(filenameItem->value(0).c_str());

  rdr->SetReadInternalVolume(readVolumes->discreteIndex());
  rdr->ReadExternalSurfaceOn();
  rdr->ReadMidpointsOn();

  // Read in the data (so we can obtain tessellation info)
  rdr->Update();

  vtkMultiBlockDataSet* blocks[] = {
    vtkMultiBlockDataSet::SafeDownCast(rdr->GetOutputDataObject(0)),
    vtkMultiBlockDataSet::SafeDownCast(rdr->GetOutputDataObject(1))
  };
  vtkSmartPointer<vtkMultiBlockDataSet> modelOut =
    FlattenBlocks(blocks, sizeof(blocks) / sizeof(blocks[0]));
  vtkIdType ii = 0;
  FillAndMarkBlocksFromSrc(modelOut, ii, blocks[0], "surface", EXO_SIDE_SET);
  FillAndMarkBlocksFromSrc(modelOut, ii, blocks[1], "volume", EXO_BLOCK);

  MarkMeshInfo(
    modelOut.GetPointer(), 3, path(filename).stem().string<std::string>().c_str(), EXO_MODEL, -1);
  AddBlockChildrenAsModelChildren(modelOut);

  // Mark any volumes as "invisible" so there is no z-fighting by default:
  vtkIdType start = blocks[0]->GetNumberOfBlocks();
  vtkIdType stop = modelOut->GetNumberOfBlocks();
  for (ii = start; ii < stop; ++ii)
  {
    vtkDataObject* obj = modelOut->GetBlock(ii);
    if (obj)
    {
      obj->GetInformation()->Set(Session::SMTK_VISIBILITY(), -1);
    }
  }

  smtk::model::Model smtkModelOut = addModel(modelOut, this->m_preservedUUIDs, session, resource);
  if (this->m_preservedUUIDs.empty())
  {
    smtkModelOut.setStringProperty("url", filename);
    smtkModelOut.setStringProperty("type", "slac");
  }

  // Now set model for session and transcribe everything.
  Import::Result result = this->createResult(Import::Outcome::SUCCEEDED);

  {
    smtk::attribute::ComponentItem::Ptr resultModels = result->findComponent("model");
    resultModels->setValue(smtkModelOut.component());
  }

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->setValue(resource);
  }

  {
    smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
    created->setValue(smtkModelOut.component());
  }

  return result;
}

int DiscoverLabels(vtkDataSet* obj, std::string& labelname, std::set<double>& labelSet)
{
  if (!obj)
    return 0;

  vtkDataSetAttributes* dsa = obj->GetPointData();
  vtkIdType card = obj->GetNumberOfPoints();

  if (card < 1 || !dsa)
    return 0;

  vtkDataArray* labelArray;
  if (labelname.empty())
  {
    labelArray = dsa->GetScalars();
  }
  else
  {
    labelArray = dsa->GetArray(labelname.c_str());
    if (!labelArray)
    {
      labelArray = dsa->GetScalars();
    }
  }
  if (!labelArray || !vtkTypeInt32Array::SafeDownCast(labelArray))
  {
    int numArrays = dsa->GetNumberOfArrays();
    for (int i = 0; i < numArrays; ++i)
    {
      if (vtkTypeInt32Array::SafeDownCast(dsa->GetArray(i)))
      {
        labelArray = dsa->GetArray(i);
        break;
      }
    }
  }

  if (!labelArray)
  { // No scalars or array of the given name? Create one.
    vtkNew<vtkUnsignedCharArray> arr;
    arr->SetName(labelname.empty() ? "label map" : labelname.c_str());
    labelname = arr->GetName(); // Upon output, labelname must be valid
    arr->SetNumberOfTuples(card);
    arr->FillComponent(0, 0.0);
    dsa->SetScalars(arr.GetPointer());

    labelSet.insert(0.0); // We have one label. It is zero.
    return 1;
  }

  labelname = labelArray->GetName();
  for (vtkIdType i = 0; i < card; ++i)
  {
    labelSet.insert(labelArray->GetTuple1(i));
  }
  return static_cast<int>(labelSet.size());
}

Import::Result Import::importLabelMap(const smtk::session::vtk::Resource::Ptr& resource)
{
  const auto& session = resource->session();

  smtk::attribute::FileItem::Ptr filenameItem = this->parameters()->findFile("filename");

  smtk::attribute::StringItem::Ptr labelItem = this->parameters()->findString("label map");

  std::string filename = filenameItem->value();
  std::string labelname;
  if (!labelItem->isEnabled())
  { // you need a label map to indicate which segment each cell belongs to

    smtkErrorMacro(
      this->log(),
      "Label map is needed to indicate which "
      "segment each cell belongs to.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  else
  {
    labelname = labelItem->value();
  }

  vtkNew<vtkXMLImageDataReader> rdr;
  rdr->SetFileName(filenameItem->value(0).c_str());

  // Read in the data and discover the labels:
  rdr->Update();
  vtkNew<vtkImageData> img;
  img->ShallowCopy(rdr->GetOutput());
  int imgDim = img->GetDataDimension();
  std::set<double> labelSet;
  int numLabels = DiscoverLabels(img.GetPointer(), labelname, labelSet);
  // Upon exit, labelname will be a point-data array in img.

  // Prepare the children of the image (holding contour data)
  vtkNew<vtkContourFilter> bdyFilt;
  vtkInformation* info = img->GetInformation();
  Session::SMTK_CHILDREN()->Resize(info, numLabels);
  int i = 0;
  bdyFilt->SetInputDataObject(0, img.GetPointer());
  bdyFilt->SetInputArrayToProcess(
    0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, labelname.c_str());
  bdyFilt->UseScalarTreeOn();
  bdyFilt->ComputeNormalsOn();
  bdyFilt->ComputeGradientsOn();
  bdyFilt->SetNumberOfContours(1);
  for (std::set<double>::iterator it = labelSet.begin(); it != labelSet.end(); ++it, ++i)
  {
    bdyFilt->SetValue(0, *it);
    bdyFilt->Update();
    vtkNew<vtkPolyData> childData;
    childData->ShallowCopy(bdyFilt->GetOutput());
    Session::SMTK_CHILDREN()->Set(info, childData.GetPointer(), i);
    childData->GetInformation()->Set(Session::SMTK_LABEL_VALUE(), *it);

    std::ostringstream cname;
    cname << "label " << i; // << " (" << *it << ")";
    MarkMeshInfo(childData.GetPointer(), imgDim, cname.str().c_str(), EXO_LABEL, int(*it));
    if (*it == 0.0)
      childData->GetInformation()->Set(Session::SMTK_OUTER_LABEL(), 1);
  }

  vtkSmartPointer<vtkMultiBlockDataSet> modelOut = vtkSmartPointer<vtkMultiBlockDataSet>::New();

  modelOut->SetNumberOfBlocks(1);
  modelOut->SetBlock(0, img.GetPointer());

  MarkMeshInfo(
    modelOut.GetPointer(),
    imgDim,
    path(filename).stem().string<std::string>().c_str(),
    EXO_MODEL,
    -1);
  MarkMeshInfo(img.GetPointer(), imgDim, labelname.c_str(), EXO_LABEL_MAP, -1);
  for (int j = 0; j < numLabels; ++j)
  {
    session->ensureChildParentMapEntry(
      vtkDataObject::SafeDownCast(Session::SMTK_CHILDREN()->Get(info, j)), img.GetPointer(), j);
  }

  smtk::model::Model smtkModelOut = addModel(modelOut, this->m_preservedUUIDs, session, resource);
  if (this->m_preservedUUIDs.empty())
  {
    smtkModelOut.setStringProperty("url", filename);
    smtkModelOut.setStringProperty("type", "label map");
    smtkModelOut.setStringProperty("label array", labelname);
  }

  // Now set model for session and transcribe everything.
  Import::Result result = this->createResult(Import::Outcome::SUCCEEDED);
  {
    smtk::attribute::ComponentItem::Ptr resultModels = result->findComponent("model");
    resultModels->setValue(smtkModelOut.component());
  }

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->setValue(resource);
  }

  {
    smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
    created->setValue(smtkModelOut.component());
  }

  return result;
}

const char* Import::xmlDescription() const
{
  return Import_xml;
}

smtk::resource::ResourcePtr importResource(const std::string& filename)
{
  Import::Ptr importResource = Import::create();
  importResource->parameters()->findFile("filename")->setValue(filename);
  Import::Result result = importResource->operate();
  if (result->findInt("outcome")->value() != static_cast<int>(Import::Outcome::SUCCEEDED))
  {
    return smtk::resource::ResourcePtr();
  }
  return result->findResource("resource")->value();
}
} // namespace vtk
} // namespace session
} // namespace smtk
