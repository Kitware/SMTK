//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME MeshItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_MeshItemDefinition_h
#define __smtk_attribute_MeshItemDefinition_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/ItemDefinition.h"
#include "smtk/mesh/core/MeshSet.h"

namespace smtk
{
namespace attribute
{
class Attribute;
class SMTKCORE_EXPORT MeshItemDefinition : public ItemDefinition
{
public:
  smtkTypeMacro(MeshItemDefinition);
  static smtk::attribute::MeshItemDefinitionPtr New(const std::string& myName)
  {
    return smtk::attribute::MeshItemDefinitionPtr(new MeshItemDefinition(myName));
  }

  ~MeshItemDefinition() override;

  Item::Type type() const override;
  std::size_t numberOfRequiredValues() const;
  void setNumberOfRequiredValues(std::size_t esize);
  std::size_t maxNumberOfValues() const { return this->m_maxNumberOfValues; }
  void setMaxNumberOfValues(std::size_t maxNum);

  bool isValueValid(const smtk::mesh::MeshSet& val) const;
  bool isExtensible() const { return this->m_isExtensible; }
  void setIsExtensible(bool extensible) { this->m_isExtensible = extensible; }

  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override;
  smtk::attribute::ItemPtr buildItem(
    Item* owningItem, int position, int subGroupPosition) const override;

  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

protected:
  MeshItemDefinition(const std::string& myName);

  std::size_t m_numberOfRequiredValues;
  std::size_t m_maxNumberOfValues;
  bool m_isExtensible;
};
}
}

#endif /* __smtk_attribute_MeshItemDefinition_h */
