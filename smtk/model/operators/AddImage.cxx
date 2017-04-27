//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/AddImage.h"
#include "smtk/model/AddAuxiliaryGeometry_xml.h"

#include "smtk/model/AddImage_xml.h"

namespace smtk
{
namespace model
{

smtk::model::OperatorResult AddImage::operateInternal()
{
  smtk::model::OperatorResult res = this->AddAuxiliaryGeometry::operateInternal();
  smtk::attribute::ModelEntityItemPtr created = res->findModelEntity("created");
  for (size_t i = 0; i < created->numberOfValues(); ++i)
  {
    smtk::model::Entity* entRec;
    if (created->value(i).isValid(&entRec))
    {
      entRec->setEntityFlags(entRec->entityFlags() | DIMENSION_2);
      created->value(i).setStringProperty("_type", "image");
    }
  }
  return res;
}
}
}

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::model::AddImage, add_image, "add image",
  AddImage_xml, smtk::model::Session);
