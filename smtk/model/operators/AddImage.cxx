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
#include "smtk/model/AuxiliaryGeometry.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/model/operators/AddImage_xml.h"

namespace smtk
{
namespace model
{

AddImage::Result AddImage::operateInternal()
{
  Result res = this->AddAuxiliaryGeometry::operateInternal();
  smtk::attribute::ComponentItem::Ptr created = res->findComponent("created");
  for (size_t i = 0; i < created->numberOfValues(); ++i)
  {
    smtk::model::EntityPtr entRec =
      std::dynamic_pointer_cast<smtk::model::Entity>(created->value(i));
    entRec->setEntityFlags(entRec->entityFlags() | DIMENSION_2);
    entRec->referenceAs<smtk::model::AuxiliaryGeometry>().setStringProperty("_type", "image");
  }
  return res;
}

const char* AddImage::xmlDescription() const
{
  return AddImage_xml;
}
} // namespace model
} // namespace smtk
