/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME ModelEntityItemDefinition.h - A definition for attribute items that store UUIDs as values.
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_ModelEntityItemDefinition_h
#define __smtk_attribute_ModelEntityItemDefinition_h

#include "smtk/util/UUID.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/model/EntityTypeBits.h" // for smtk::model::BitFlags

namespace smtk
{
  namespace attribute
  {

class SMTKCORE_EXPORT ModelEntityItemDefinition : public ItemDefinition
{
public:
  smtkTypeMacro(ModelEntityItemDefinition);
  static smtk::attribute::ModelEntityItemDefinitionPtr New(const std::string& sname)
    { return smtk::attribute::ModelEntityItemDefinitionPtr(new ModelEntityItemDefinition(sname));}

  virtual ~ModelEntityItemDefinition();

  virtual Item::Type type() const;

  smtk::model::BitFlags membershipMask() const;
  void setMembershipMask(smtk::model::BitFlags entMask);

  bool isValueValid(const smtk::model::Cursor& entity) const;

  virtual smtk::attribute::ItemPtr buildItem(
    Attribute* owningAttribute, int itemPosition) const;
  virtual smtk::attribute::ItemPtr buildItem(
    Item* owningItem, int position, int subGroupPosition) const;

  std::size_t numberOfRequiredValues() const;
  void setNumberOfRequiredValues(std::size_t esize);

  bool hasValueLabels() const;
  std::string valueLabel(std::size_t element) const;
  void setValueLabel(std::size_t element, const std::string &elabel);
  void setCommonValueLabel(const std::string &elabel);
  bool usingCommonLabel() const;

  virtual smtk::attribute::ItemDefinitionPtr
    createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const;
protected:
  ModelEntityItemDefinition(const std::string& myName);

  smtk::model::BitFlags m_membershipMask;
  bool m_useCommonLabel;
  std::vector<std::string> m_valueLabels;
  std::size_t m_numberOfRequiredValues;
};

  } // namespace attribute
} // namespace smtk

#endif /* __smtk_attribute_ModelEntityItemDefinition_h */
