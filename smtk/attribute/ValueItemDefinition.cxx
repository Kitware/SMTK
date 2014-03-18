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

#include <algorithm>

#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/RefItemDefinition.h"

using namespace smtk::attribute; 

//----------------------------------------------------------------------------
ValueItemDefinition::ValueItemDefinition(const std::string &myName):
  ItemDefinition(myName)
{
  this->m_defaultDiscreteIndex = -1;
  this->m_hasDefault = false;
  this->m_useCommonLabel = false;
  this->m_numberOfRequiredValues = 1;
  this->m_maxNumberOfValues = 0;
  this->m_isExtensible = false;
  this->m_expressionDefinition = RefItemDefinition::New("expression");
  this->m_expressionDefinition->setNumberOfRequiredValues(1);
}

//----------------------------------------------------------------------------
ValueItemDefinition::~ValueItemDefinition()
{
}
//----------------------------------------------------------------------------
bool ValueItemDefinition::setNumberOfRequiredValues(std::size_t esize)
{
  if (esize == this->m_numberOfRequiredValues)
    {
    return true;
    }
  std::size_t maxN = this->maxNumberOfValues();
  if (maxN && (esize > maxN))
    {
    return false;
    }

  this->m_numberOfRequiredValues = esize;
  if (!(this->m_useCommonLabel || this->m_isExtensible))
    {
    this->m_valueLabels.resize(esize);
    }
  return true;
}
//----------------------------------------------------------------------------
bool  ValueItemDefinition::setMaxNumberOfValues(std::size_t esize)
{
  if (esize && (esize > this->m_numberOfRequiredValues))
    {
    return false;
    }
  this->m_maxNumberOfValues = esize;
  return true;
}
//----------------------------------------------------------------------------
void ValueItemDefinition::setValueLabel(std::size_t element, const std::string &elabel)
{
  if (this->m_isExtensible)
    {
    return;
    }
  if (this->m_valueLabels.size() != this->m_numberOfRequiredValues)
    {
    this->m_valueLabels.resize(this->m_numberOfRequiredValues);
    }
  this->m_useCommonLabel = false;
  this->m_valueLabels[element] = elabel;
}
//----------------------------------------------------------------------------
void ValueItemDefinition::setCommonValueLabel(const std::string &elabel)
{
  if (this->m_valueLabels.size() != 1)
    {
    this->m_valueLabels.resize(1);
    }
  this->m_useCommonLabel = true;
  this->m_valueLabels[0] = elabel;
}

//----------------------------------------------------------------------------
std::string ValueItemDefinition::valueLabel(std::size_t element) const
{
  if (this->m_useCommonLabel)
    {
    return this->m_valueLabels[0];
    }
  if (element < this->m_valueLabels.size())
    {
    return this->m_valueLabels[element];
    }
  return ""; // If we threw execeptions this method could return const string &
}
//----------------------------------------------------------------------------
bool ValueItemDefinition::isValidExpression(smtk::attribute::AttributePtr exp) const
{
  if (this->m_expressionDefinition->attributeDefinition() &&
      this->m_expressionDefinition->isValueValid(exp))
    {
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool ValueItemDefinition::allowsExpressions() const
{
  return this->m_expressionDefinition->attributeDefinition() ? true : false;
}
//----------------------------------------------------------------------------
smtk::attribute::DefinitionPtr ValueItemDefinition::expressionDefinition() const
{
  return this->m_expressionDefinition->attributeDefinition();
}
//----------------------------------------------------------------------------
void
ValueItemDefinition::setExpressionDefinition(smtk::attribute::DefinitionPtr exp)
{
  this->m_expressionDefinition->setAttributeDefinition(exp);
}
//----------------------------------------------------------------------------
void
ValueItemDefinition::buildExpressionItem(ValueItem *vitem, int position) const
{
  smtk::attribute::RefItemPtr aref =
    smtk::dynamic_pointer_cast<smtk::attribute::RefItem>
    (this->m_expressionDefinition->buildItem(vitem, position, -1));
  aref->setDefinition(this->m_expressionDefinition);
  vitem->m_expressions[static_cast<size_t>(position)] = aref;
}
//----------------------------------------------------------------------------
void
ValueItemDefinition::buildChildrenItems(ValueItem *vitem) const
{
  std::map<std::string, smtk::attribute::ItemDefinitionPtr>::const_iterator it;
  smtk::attribute::ItemPtr child;
  for (it = this->m_itemDefs.begin(); it != this->m_itemDefs.end(); it++)
    {
    child = it->second->buildItem(vitem, 0, -1);
    child->setDefinition(it->second);
    vitem->m_childrenItems[it->first] = child;
    }
}
//----------------------------------------------------------------------------
void ValueItemDefinition::setDefaultDiscreteIndex(int discreteIndex)
{
  this->m_defaultDiscreteIndex = discreteIndex;
  this->m_hasDefault = true;
}
//----------------------------------------------------------------------------
bool ValueItemDefinition::
addChildItemDefinition(smtk::attribute::ItemDefinitionPtr cdef)
{
  // First see if there is a item by the same name
  if (this->hasChildItemDefinition(cdef->name()))
    {
    return false;
    }
  this->m_itemDefs[cdef->name()] = cdef;
  return true;
}
//----------------------------------------------------------------------------
bool ValueItemDefinition::
addConditionalItem(const std::string &valueName, const std::string &itemName)
{
  // Do we have this valueName?
  if (std::find(this->m_discreteValueEnums.begin(),
                this->m_discreteValueEnums.end(), valueName) ==
      this->m_discreteValueEnums.end())
    {
    return false;
    }
  // Next do we have such an iten definition?
  if (!this->hasChildItemDefinition(itemName))
    {
    return false;
    }

  // Finally we need to verify that we don't already have this item assigned
  if (this->hasChildItemDefinition(valueName, itemName))
    {
    return false;
    }

  // create the association
  this->m_valueToItemAssociations[valueName].push_back(itemName);
  this->m_itemToValueAssociations[itemName].insert(valueName);
  return true;
}
//----------------------------------------------------------------------------
std::vector<std::string>
ValueItemDefinition::conditionalItems(const std::string &valueName) const
{
  // Do we have this valueName?
  if (std::find(this->m_discreteValueEnums.begin(),
                this->m_discreteValueEnums.end(), valueName) ==
      this->m_discreteValueEnums.end())
    {
    std::vector<std::string> temp;
    return temp;
    }
  std::map<std::string, std::vector<std::string> >::const_iterator citer =
    this->m_valueToItemAssociations.find(valueName);
  // Does the value have conditional items associated with it?
  if (citer == this->m_valueToItemAssociations.end())
    {
    std::vector<std::string> dummy;
    return dummy;
    }
  return citer->second;
}
//----------------------------------------------------------------------------
void ValueItemDefinition::setIsExtensible(bool mode)
{
  this->m_isExtensible = mode;
  if (mode && !this->usingCommonLabel())
    {
    // Need to clear individual labels - can only use common label with
    // extensible values
    this->setCommonValueLabel("");
    }
}
