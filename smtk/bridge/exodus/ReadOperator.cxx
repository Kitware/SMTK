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
#include "vtkXMLImageDataReader.h"
#include "vtkContourFilter.h"
#include "vtkThreshold.h"
#include "vtkFieldData.h"
#include "vtkDataArray.h"
#include "vtkDataSetAttributes.h"
#include "vtkPointData.h"
#include "vtkUnsignedCharArray.h"
#include "vtkImageData.h"
#include "vtkStringArray.h"
#include "vtkInformation.h"
#include "vtkUnstructuredGrid.h"

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
    else if (ext == ".vti")
      filetype = "label map";
    else if (ext == ".exo" || ext == ".g" || ext == ".ex2" || ext == ".exii")
      filetype = "exodus";
    }

  // Downcase the filetype (especially for when we did not infer it):
  std::transform(filetype.begin(), filetype.end(), filetype.begin(), ::tolower);

  if (filetype == "slac")
    return this->readSLAC();
  else if (filetype == "label map")
    return this->readLabelMap();

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

  // ++ 1 ++
  // If a UUID has been saved to field data, we should copy it to the info object here.
  vtkStringArray* uuidArr =
    vtkStringArray::SafeDownCast(
      data->GetFieldData()->GetAbstractArray("UUID"));
  if (uuidArr && uuidArr->GetNumberOfTuples() > 0)
    info->Set(Session::SMTK_UUID_KEY(), uuidArr->GetValue(0).c_str());
  // -- 1 --

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

template<typename T, typename V>
void MarkChildren(vtkMultiBlockDataSet* data, T* key, V value)
{
  if (!key)
    return;

  int nb = data->GetNumberOfBlocks();
  for (int i = 0; i < nb; ++i)
    {
    vtkDataObject* obj = data->GetBlock(i);
    if (obj)
      obj->GetInformation()->Set(key, value);
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

  // Mark any volumes as "invisible" so there is no z-fighting by default:
  MarkChildren(voluBlocks.GetPointer(), Session::SMTK_VISIBILITY(), -1);

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
  return labelSet.size();
}

smtk::model::OperatorResult ReadOperator::readLabelMap()
{
  smtk::attribute::FileItem::Ptr filenameItem =
    this->specification()->findFile("filename");

  smtk::attribute::StringItem::Ptr labelItem =
    this->specification()->findString("label map");

  std::string filename = filenameItem->value();
  std::string labelname = labelItem->value();
  if (labelname.empty())
    labelname = "label map";

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

  vtkSmartPointer<vtkMultiBlockDataSet> modelOut =
    vtkSmartPointer<vtkMultiBlockDataSet>::New();

  modelOut->SetNumberOfBlocks(1);
  modelOut->SetBlock(0, img.GetPointer());

  MarkMeshInfo(modelOut.GetPointer(), imgDim, path(filename).stem().c_str(), EXO_MODEL, -1);
  MarkMeshInfo(img.GetPointer(), imgDim, labelname.c_str(), EXO_LABEL_MAP, -1);
  for (int i = 0; i < numLabels; ++i)
    {
    this->exodusSession()->ensureChildParentMapEntry(
      vtkDataObject::SafeDownCast(Session::SMTK_CHILDREN()->Get(info, i)),
      img.GetPointer(),
      i);
    }

  Session* brdg = this->exodusSession();
  smtk::model::Model smtkModelOut =
    brdg->addModel(modelOut);
  smtkModelOut.setStringProperty("url", filename);
  smtkModelOut.setStringProperty("type", "label map");

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
