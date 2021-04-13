//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_vtk_model_vtkModelAuxiliaryGeometry_txx
#define smtk_extension_vtk_model_vtkModelAuxiliaryGeometry_txx

#include "smtk/extension/vtk/model/vtkModelAuxiliaryGeometry.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include "vtkTransform.h"
#include "vtkTransformFilter.h"

template<typename T, typename U>
vtkSmartPointer<T> vtkModelAuxiliaryGeometry::ReadData(
  const smtk::model::AuxiliaryGeometry& auxGeom)
{
  vtkNew<U> rdr;
  rdr->SetFileName(auxGeom.url().c_str());
  rdr->Update();
  vtkSmartPointer<T> data = vtkSmartPointer<T>::New();
  if (auxGeom.hasFloatProperties())
  {
    const char* propNames[3] = { "scale", "rotate", "translate" };
    bool hasTransform = false;
    vtkNew<vtkTransform> tfm;
    //tfm->PostMultiply();
    for (int ii = 0; ii < 3; ++ii)
    {
      if (auxGeom.hasFloatProperty(propNames[ii]))
      {
        const smtk::model::FloatList& prop = auxGeom.floatProperty(propNames[ii]);
        {
          hasTransform = true;
          switch (ii)
          {
            case 0:
              tfm->Scale(prop[0], prop[1], prop[2]);
              break;
            case 1:
              tfm->RotateX(prop[0]);
              tfm->RotateY(prop[1]);
              tfm->RotateZ(prop[2]);
              break;
            case 2:
              tfm->Translate(prop[0], prop[1], prop[2]);
              break;
          }
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

#endif
