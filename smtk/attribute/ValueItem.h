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
// .NAME ValueItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_ValueItem_h
#define __smtk_attribute_ValueItem_h

#include "smtk/attribute/Item.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SMTKCoreExports.h"

namespace smtk
{
  namespace attribute
  {
    class Attribute;
    class AttributeRefItem;
    class ValueItemDefinition;
    class SMTKCORE_EXPORT ValueItem : public smtk::attribute::Item
    {
    public:
      friend class ValueItemDefinition;
      // This method is for wrapping code.  C++ developers should use smtk::dynamicCastPointer
      static smtk::ValueItemPtr CastTo(const smtk::AttributeItemPtr &p)
      {return smtk::dynamic_pointer_cast<ValueItem>(p);}

      virtual ~ValueItem();
      virtual std::size_t numberOfValues() const
      {return this->m_isSet.size();}
      int numberOfRequiredValues() const;

      bool allowsExpressions() const;
      bool isExpression(int elementIndex=0) const
      { return (this->expression(elementIndex) != NULL);}
      smtk::AttributePtr expression(int elementIndex=0) const;
      bool setExpression(smtk::AttributePtr exp)
      {return this->setExpression(0, exp);}
      bool setExpression(int elementIndex, smtk::AttributePtr exp);
      virtual bool appendExpression(smtk::AttributePtr exp);
      virtual bool setNumberOfValues(std::size_t newSize) = 0;

      int discreteIndex(int elementIndex=0) const
      {return this->m_discreteIndices[elementIndex];}
      bool isDiscrete() const;

      bool setDiscreteIndex(int value)
      {return this->setDiscreteIndex(0, value);}
      bool setDiscreteIndex(int elementIndex, int value);
      // Reset returns the item to its initial state.
      //If the item is of fixed size, then it's values  to their initial state.
      // If there is a default available it will use it, else
      // it will be marked as unset.
      //If the item's definition indicated a size of 0 then it will go back to
      // having no values
      virtual void reset();
      virtual bool setToDefault(int elementIndex=0) = 0;
      virtual std::string valueAsString() const
      { return this->valueAsString(0);}

      virtual std::string valueAsString(int elementIndex) const = 0;
      virtual bool isSet(int elementIndex = 0) const
      {return this->m_isSet[elementIndex];}
      virtual void unset(int elementIndex=0)
      {this->m_isSet[elementIndex] = false;}
      smtk::AttributeRefItemPtr expressionReference(int elementIndex=0) const
      {return this->m_expressions[elementIndex];}

    protected:
      ValueItem(Attribute *owningAttribute, int itemPosition);
      ValueItem(Item *owningItem, int myPosition, int mySubGroupPosition);
      virtual bool setDefinition(smtk::ConstAttributeItemDefinitionPtr def);
      virtual void updateDiscreteValue(int elementIndex) = 0;
      std::vector<int> m_discreteIndices;
      std::vector<bool> m_isSet;
      std::vector<smtk::AttributeRefItemPtr > m_expressions;
    private:

    };
  }
}

#endif /* __smtk_attribute_ValueItem_h */
