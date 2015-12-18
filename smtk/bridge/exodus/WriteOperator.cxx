//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/exodus/WriteOperator.h"

#include "smtk/bridge/exodus/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "vtkDataSetWriter.h"
#include "vtkXMLImageDataWriter.h"
#include "vtkContourFilter.h"
#include "vtkThreshold.h"
#include "vtkFieldData.h"
#include "vtkDataArray.h"
#include "vtkDataSetAttributes.h"
#include "vtkPointData.h"
#include "vtkUnsignedCharArray.h"
#include "vtkImageData.h"
#include "vtkStringArray.h"
#include "vtkTypeInt32Array.h"
#include "vtkInformation.h"
#include "vtkUnstructuredGrid.h"
#include "vtkPassArrays.h"
#include "vtkImageConstantPad.h"

#include "vtkVector.h"
#include "vtkVectorOperators.h"

#include "boost/filesystem.hpp"

using namespace smtk::model;
using namespace smtk::common;
using namespace boost::filesystem;

namespace smtk {
  namespace bridge {
    namespace exodus {

smtk::model::OperatorResult WriteOperator::operateInternal()
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
    else if (ext == ".vtk" || ext == ".vti")
      filetype = "label map";
    else if (ext == ".exo" || ext == ".g" || ext == ".ex2" || ext == ".exii")
      filetype = "exodus";
    }

  // Downcase the filetype (especially for when we did not infer it):
  std::transform(filetype.begin(), filetype.end(), filetype.begin(), ::tolower);

  if (filetype == "slac")
    return this->writeSLAC();
  else if (filetype == "label map")
    return this->writeLabelMap();

  // The default is to assume it is an Exodus file:
  return this->writeExodus();
}

smtk::model::OperatorResult WriteOperator::writeExodus()
{
  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_FAILED);
  return result;
}

smtk::model::OperatorResult WriteOperator::writeSLAC()
{
  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_FAILED);
  return result;
}

template<typename T>
void RewriteLabels(
  vtkImageData* img,
  T* lblp,
  double thresh,

  vtkVector3d& scenter,
  double sradius,

  vtkVector3d& basept,
  vtkVector3d& normal
  )
{
  enum SimulationVoxelCodes {
    VOXEL_VOID = -1,
    AIRWAY = 0,
    ATMOSPHERE = 0, // "Antechamber to the airway"
    INLET = 100,
    OUTLET = 200
  };
  // Density array
  vtkDataArray* den = img->GetPointData()->GetArray("scalars");
  vtkIdType numPts = img->GetNumberOfPoints();
  vtkVector3d x;
  vtkVector3d spacing;
  img->GetSpacing(spacing.GetData());
  double delta = 0.; // sqrt(spacing.Dot(spacing));
  for (int i = 0; i < 3; ++i)
    if (delta < spacing[i])
      delta = spacing[i];
  for (vtkIdType p = 0; p < numPts; ++p)
    {
    vtkVector3d ray;
    double dist;
    bool invitro;
    img->GetPoint(p, x.GetData());
    if ((dist = (x - basept).Dot(normal)) < -delta) // Below the lower cutoff plane?
      lblp[p] = VOXEL_VOID;
    else if ((dist < delta) && (lblp[p] == 1)) // "On" the lower cutoff plane?
      lblp[p] = OUTLET;
    else if ((dist = sqrt((ray = (x - scenter)).Dot(ray)) - sradius) < delta) // In or on the nose sphere?
      {
      invitro = den->GetTuple1(p) < thresh ? false : true;
      // In or on the nose sphere, anything marked "airway" must stay that way:
      if (lblp[p] == 1)
        lblp[p] = AIRWAY;
      else
        { // ... otherwise, it becomes "atmosphere" or "void":
        if (invitro) // In the body? Void.
          lblp[p] = VOXEL_VOID;
        else // Outside the body? Make "atmosphere" or "inlet":
          lblp[p] = dist < -delta ? ATMOSPHERE : INLET;
        }
      }
    else // We are not near the nostril and above the cut-plane:
      lblp[p] = (lblp[p] == 1 ? AIRWAY : VOXEL_VOID); // Preserve the airway; all else is void.
    }
}

smtk::model::OperatorResult WriteOperator::writeLabelMap()
{
  smtk::attribute::FileItem::Ptr filenameItem =
    this->specification()->findFile("filename");

  smtk::attribute::DoubleItem::Ptr noseSphereItem =
    this->specification()->findDouble("nose sphere");

  smtk::attribute::DoubleItem::Ptr lowerPlaneItem =
    this->specification()->findDouble("lower plane");

  smtk::model::Models datasets =
    this->specification()->associatedModelEntities<smtk::model::Models>();
  if (datasets.empty())
    {
    smtkErrorMacro(this->log(), "No models to save.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::Model dataset = datasets[0];
  std::string labelStr;
  StringData const& stringProps(dataset.stringProperties());
  StringData::const_iterator sit;
  if (
    (sit = stringProps.find("type")) == stringProps.end() ||
    sit->second.empty() ||
    sit->second[0] != "label map" ||
    (sit = stringProps.find("label array")) == stringProps.end() ||
    sit->second.empty() ||
    (labelStr = sit->second[0]).empty())
    {
    smtkErrorMacro(this->log(), "Model is not a label map or has no label array.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  EntityHandle handle = this->exodusHandle(dataset);
  vtkMultiBlockDataSet* mbds = handle.object<vtkMultiBlockDataSet>();
  vtkImageData* img;
  vtkDataArray* lbl;
  if (
    !mbds ||
    mbds->GetNumberOfBlocks() < 1 ||
    !(img = vtkImageData::SafeDownCast(mbds->GetBlock(0))) ||
    !(lbl = img->GetPointData()->GetArray(labelStr.c_str()))
    )
    {
    smtkErrorMacro(this->log(), "Model does not have image data with labels attached.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  vtkVector3d scenter(&(*noseSphereItem->begin()));
  double sradius = noseSphereItem->value(3);
  vtkVector3d basept(&(*lowerPlaneItem->begin()));
  vtkVector3d normal;
  for (int i = 0; i < 3; ++i)
    normal[i] = lowerPlaneItem->value(i + 3);
  switch (lbl->GetDataType())
    {
    vtkTemplateMacro(
      RewriteLabels(img, static_cast<VTK_TT*>(lbl->GetVoidPointer(0)), -500.0, scenter, sradius, basept, normal));
    }

  // Omit the density field from the MRI scan (and other, non-label arrays):
  vtkNew<vtkPassArrays> pass;
  pass->SetInputDataObject(img);
  pass->AddArray(vtkDataObject::POINT, labelStr.c_str());

  // Pad volume so it is a multiple of 16 voxels in each direction:
  vtkNew<vtkImageConstantPad> pad;
  pad->SetInputConnection(pass->GetOutputPort());
  pad->SetConstant(-1); // pad exterior voxels

  int extent[6];
  img->GetExtent(extent);
  int dims[3] = {extent[1] - extent[0] + 1, extent[3] - extent[2] + 1, extent[5] - extent[4] + 1};
  for (int i = 0; i < 3; ++i)
    {
    if (dims[i] % 16 > 0)
      {
      dims[i]  = dims[i] + (16 - (dims[i] % 16));
      }
    }
  extent[1] = extent[0] + dims[0] - 1;
  extent[3] = extent[2] + dims[1] - 1;
  extent[5] = extent[4] + dims[2] - 1;
  pad->SetOutputWholeExtent(extent);

  // Write out the data
  std::string filename = filenameItem->value();
  vtkNew<vtkDataSetWriter> wri;
  wri->SetFileName(filenameItem->value(0).c_str());
  wri->SetInputConnection(pad->GetOutputPort());
  wri->SetFileTypeToBinary();
  wri->Write();

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  return result;
}

    } // namespace exodus
  } //namespace bridge
} // namespace smtk

#include "smtk/bridge/exodus/WriteOperator_xml.h"
#include "smtk/bridge/exodus/Exports.h"

smtkImplementsModelOperator(
  SMTKEXODUSSESSION_EXPORT,
  smtk::bridge::exodus::WriteOperator,
  exodus_write,
  "write",
  WriteOperator_xml,
  smtk::bridge::exodus::Session);
