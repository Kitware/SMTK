//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME ValueItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_ValueItem_h
#define __smtk_attribute_ValueItem_h

#include "smtk/attribute/Item.h"
#include "smtk/attribute/SearchStyle.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/CoreExports.h"

#include <cassert>

namespace smtk
{
  namespace attribute
  {
    class Attribute;
    class RefItem;
    class ValueItemDefinition;
    class SMTKCORE_EXPORT ValueItem : public smtk::attribute::Item
    {
    public:
      smtkTypeMacro(ValueItem);
      friend class ValueItemDefinition;

      virtual ~ValueItem();
      virtual std::size_t numberOfValues() const
      {return this->m_isSet.size();}
      virtual bool isValid() const;
      std::size_t numberOfRequiredValues() const;
      std::size_t maxNumberOfValues() const;

      bool isExtensible() const;

      bool allowsExpressions() const;
      bool isExpression(std::size_t elementIndex=0) const
      { return (!!this->expression(elementIndex));}
      smtk::attribute::AttributePtr expression(std::size_t elementIndex=0) const;
      bool setExpression(smtk::attribute::AttributePtr exp)
      {return this->setExpression(0, exp);}
      bool setExpression(std::size_t elementIndex, smtk::attribute::AttributePtr exp);
      virtual bool appendExpression(smtk::attribute::AttributePtr exp);
      virtual bool setNumberOfValues(std::size_t newSize) = 0;

      int discreteIndex(std::size_t elementIndex=0) const
      {
        assert(this->m_discreteIndices.size() > elementIndex);
        return this->m_discreteIndices[elementIndex];
      }
      bool isDiscrete() const;
      bool isDiscreteIndexValid(int value) const;
      bool setDiscreteIndex(int value)
      {return this->setDiscreteIndex(0, value);}
      // Returns true if value is a valid index - else it returns false
      bool setDiscreteIndex(std::size_t elementIndex, int value);
      // Reset returns the item to its initial state.
      //If the item is of fixed size, then it's values  to their initial state.
      // If there is a default available it will use it, else
      // it will be marked as unset.
      //If the item's definition indicated a size of 0 then it will go back to
      // having no values
      virtual void reset();
      virtual bool setToDefault(std::size_t elementIndex=0) = 0;
      // Returns true if there is a default defined and the item is curently set to it
      virtual bool isUsingDefault(std::size_t elementIndex) const = 0;
      // This method tests all of the values of the items w/r the default value
      virtual bool isUsingDefault() const = 0;
      // Does this item have a default value?
      bool hasDefault() const;
      virtual std::string valueAsString() const
      { return this->valueAsString(0);}

      virtual std::string valueAsString(std::size_t elementIndex) const = 0;
      virtual bool isSet(std::size_t elementIndex = 0) const
      {
        assert(this->m_isSet.size() > elementIndex);
        return this->m_isSet[elementIndex];
      }
      virtual void unset(std::size_t elementIndex=0)
      {this->m_isSet[elementIndex] = false;}
      smtk::attribute::RefItemPtr expressionReference(std::size_t elementIndex=0) const
      {
        assert(this->m_expressions.size() > elementIndex);
        return this->m_expressions[elementIndex];
      }

      // Interface for getting discrete-value based children items
      std::size_t numberOfChildrenItems() const
      { return this->m_childrenItems.size();}

      const std::map<std::string, smtk::attribute::ItemPtr>& childrenItems() const
      { return this->m_childrenItems; }

      std::size_t numberOfActiveChildrenItems() const
      { return this->m_activeChildrenItems.size();}

      smtk::attribute::ItemPtr activeChildItem(int i) const
      {
        if ((i < 0) || (static_cast<std::size_t>(i) >= this->m_activeChildrenItems.size()))
          {
          smtk::attribute::ItemPtr item;
          return item;
          }
        return this->m_activeChildrenItems[static_cast<std::size_t>(i)];
      }

      // Assigns this item to be equivalent to another.  Options are processed by derived item classes
      // Returns true if success and false if a problem occured.  By default, an attribute being used by this
      // to represent an expression will be copied if needed.  Use IGNORE_EXPRESSIONS option to prevent this
      // When an expression attribute is copied, its model associations are by default not.
      // Use COPY_MODEL_ASSOCIATIONS if you want them copied as well.These options are defined in Item.h .
      virtual bool assign(smtk::attribute::ConstItemPtr &sourceItem, unsigned int options = 0);

      ItemPtr findChild(const std::string& name, smtk::attribute::SearchStyle);
      ConstItemPtr findChild(const std::string& name, smtk::attribute::SearchStyle) const;
    protected:
      ValueItem(Attribute *owningAttribute, int itemPosition);
      ValueItem(Item *owningItem, int myPosition, int mySubGroupPosition);
      virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr def);
      virtual void updateDiscreteValue(std::size_t elementIndex) = 0;
      virtual void updateActiveChildrenItems();
      std::vector<int> m_discreteIndices;
      std::vector<bool> m_isSet;
      std::vector<smtk::attribute::RefItemPtr > m_expressions;
      std::map<std::string, smtk::attribute::ItemPtr> m_childrenItems;
      std::vector<smtk::attribute::ItemPtr> m_activeChildrenItems;
    private:

    };
  }
}

#endif /* __smtk_attribute_ValueItem_h */
