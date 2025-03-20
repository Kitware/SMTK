//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/markup/operators/ChangeUnits.h"

#include "smtk/markup/DiscreteGeometry.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/StringItem.h"

#include "vtkImageData.h"
#include "vtkPointSet.h"
#include "vtkPoints.h"
#include "vtkSMPTools.h"
#include "vtkVector.h"

#include "units/Converter.h"
#include "units/System.h"
#include "units/Unit.h"

#include "smtk/markup/operators/ChangeUnits_xml.h"

namespace smtk
{
namespace markup
{

bool ChangeUnits::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  std::string srcLengthUnitStr = this->parameters()->findString("source units")->value();

  auto assocs = this->parameters()->associations();
  bool ok = assocs->numberOfValues() > 0;
  for (const auto& assoc : *assocs)
  {
    auto* rsrc = dynamic_cast<smtk::markup::Resource*>(assoc->parentResource());
    auto usys = rsrc ? rsrc->unitSystem() : nullptr;
    // Every object associated must live in a resource with a unit system and default length unit
    // that can be converted to the length unit for this operation.
    if (!usys || rsrc->lengthUnit().empty())
    {
      ok = false;
      break;
    }

    auto srcUnit = usys->unit(srcLengthUnitStr, &ok);
    if (!ok)
    {
      break;
    }
    auto dstUnit = usys->unit(rsrc->lengthUnit(), &ok);
    if (!ok)
    {
      break;
    }
    auto cvt = usys->convert(srcUnit, dstUnit);
    if (!cvt)
    {
      ok = false;
      break;
    }
  }
  return ok;
}

ChangeUnits::Result ChangeUnits::operateInternal()
{
  std::string srcLengthUnitStr = this->parameters()->findString("source units")->value();
  auto assocs = this->parameters()->associations();
  auto result = this->createResult(ChangeUnits::Outcome::SUCCEEDED);
  auto mod = result->findComponent("modified");
  mod->setNumberOfValues(assocs->numberOfValues());
  for (const auto& assoc : *assocs)
  {
    auto* rsrc = dynamic_cast<smtk::markup::Resource*>(assoc->parentResource());
    auto usys = rsrc ? rsrc->unitSystem() : nullptr;
    if (!usys)
    {
      continue;
    }
    bool ok;
    auto srcUnit = usys->unit(srcLengthUnitStr, &ok);
    if (!ok)
    {
      continue;
    }
    auto dstUnit = usys->unit(rsrc->lengthUnit(), &ok);
    if (!ok)
    {
      continue;
    }
    auto cvt = usys->convert(srcUnit, dstUnit);
    auto spatialData = std::dynamic_pointer_cast<smtk::markup::DiscreteGeometry>(assoc);
    auto mesh = spatialData->shape();
    if (!cvt || !mesh)
    {
      ok = false;
      break;
    }
    if (auto* image = vtkImageData::SafeDownCast(mesh))
    {
      vtkVector3d xx;
      vtkVector3d yy;
      image->GetOrigin(xx.GetData());
      image->GetSpacing(yy.GetData());
      for (int ii = 0; ii < 3; ++ii)
      {
        xx[ii] = cvt->transform(xx[ii]);
        yy[ii] = cvt->transform(yy[ii]);
      }
      image->SetOrigin(xx.GetData());
      image->SetSpacing(yy.GetData());
      mesh->Modified();
      smtk::operation::MarkGeometry().markModified(assoc);
    }
    else if (auto* pset = vtkPointSet::SafeDownCast(mesh))
    {
      auto* pts = pset->GetPoints();
      vtkSMPTools::For(0, pset->GetNumberOfPoints(), [&](vtkIdType begin, vtkIdType end) {
        vtkVector3d xx;
        for (vtkIdType pp = begin; pp < end; ++pp)
        {
          pts->GetPoint(pp, xx.GetData());
          for (int ii = 0; ii < 3; ++ii)
          {
            xx[ii] = cvt->transform(xx[ii]);
          }
          pts->SetPoint(pp, xx.GetData());
        }
      });
      mesh->Modified();
      smtk::operation::MarkGeometry().markModified(spatialData);
    }
    else
    {
      smtkErrorMacro(
        this->log(),
        "Unhandled shape type '" << mesh->GetClassName()
                                 << "' for "
                                    "component '"
                                 << assoc->name() << "' (" << assoc->typeName() << ").");
    }
  }
  return result;
}

const char* ChangeUnits::xmlDescription() const
{
  return ChangeUnits_xml;
}

} // namespace markup
} // namespace smtk
