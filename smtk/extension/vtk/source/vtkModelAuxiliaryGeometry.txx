//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#pragma once

#include "smtk/extension/vtk/source/vtkModelAuxiliaryGeometry.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include "vtkTransform.h"
#include "vtkTransformFilter.h"

template <typename T, typename U>
vtkSmartPointer<T> vtkModelAuxiliaryGeometry::ReadData(
  const smtk::model::AuxiliaryGeometry& auxGeom)
{
  vtkNew<U> rdr;
  rdr->SetFileName(auxGeom.url().c_str());
  rdr->Update();
  vtkSmartPointer<T> data = vtkSmartPointer<T>::New();
  if (auxGeom.hasFloatProperties())
  {
    const smtk::model::FloatData& props(auxGeom.floatProperties());
    const char* propNames[3] = { "scale", "rotate", "translate" };
    smtk::model::FloatData::const_iterator propIt;
    bool hasTransform = false;
    vtkNew<vtkTransform> tfm;
    //tfm->PostMultiply();
    for (int ii = 0; ii < 3; ++ii)
    {
      propIt = props.find(propNames[ii]);
      if (propIt != props.end())
      {
        hasTransform = true;
        switch (ii)
        {
          case 0:
            tfm->Scale(propIt->second[0], propIt->second[1], propIt->second[2]);
            break;
          case 1:
            tfm->RotateX(propIt->second[0]);
            tfm->RotateY(propIt->second[1]);
            tfm->RotateZ(propIt->second[2]);
            break;
          case 2:
            tfm->Translate(propIt->second[0], propIt->second[1], propIt->second[2]);
            break;
        }
      }
    }
    if (hasTransform)
    {
      vtkNew<vtkTransformFilter> xfm;
      xfm->SetInputConnection(rdr->GetOutputPort());
      xfm->SetTransform(tfm.GetPointer());
      xfm->Update();
      data->ShallowCopy(xfm->GetOutput());
      return data;
    }
  }
  data->ShallowCopy(rdr->GetOutput());
  return data;
}
