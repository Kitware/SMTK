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
// .NAME ValueItemTemplate.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_ValueItemTemplate_h
#define __smtk_attribute_ValueItemTemplate_h

#include "smtk/attribute/AttributeRefItem.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinitionTemplate.h"
#include <vector>

namespace smtk
{
  namespace attribute
  {
    template<typename DataT>
    class ValueItemTemplate : public ValueItem
    {
      //template<DataT> friend class ValueItemDefinitionTemplate;
    public:
      typedef DataT DataType;
      typedef ValueItemDefinitionTemplate<DataType> DefType;
      
      ValueItemTemplate(Attribute *owningAttribute, int itemPosition);
      ValueItemTemplate(Item *owningItem, int myPosition, int mySubGroupPosition);
      virtual ~ValueItemTemplate() {}
      virtual bool setDefinition(smtk::ConstAttributeItemDefinitionPtr vdef);
      std::size_t numberOfValues() const
      {return this->m_values.size();}
      virtual bool setNumberOfValues(std::size_t newSize);
      DataT value(int element=0) const
      {return this->m_values[element];}
      virtual std::string valueAsString(const std::string &format="") const
      {return this->valueAsString(0, format);}
      virtual std::string valueAsString(int element, const std::string &format="") const;
      bool setValue(const DataT &val)
      {return this->setValue(0, val);}
      bool setValue(int element, const DataT &val);
      bool appendValue(const DataT &val);
      virtual bool appendExpression(smtk::AttributePtr exp);
      bool removeValue(int element);
      virtual void reset();
      bool setToDefault(int element=0);

    protected:
      virtual void updateDiscreteValue(int element);
      std::vector<DataT> m_values;
    private:
    };

//----------------------------------------------------------------------------
    template<typename DataT>
    ValueItemTemplate<DataT>::ValueItemTemplate(Attribute *owningAttribute, 
                                                int itemPosition): 
      ValueItem(owningAttribute, itemPosition)
    {
    }

//----------------------------------------------------------------------------
    template<typename DataT>
    ValueItemTemplate<DataT>::ValueItemTemplate(Item *owningItem,
                                                int itemPosition,
                                                int mySubGroupPosition): 
      ValueItem(owningItem, itemPosition, mySubGroupPosition)
    {
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool ValueItemTemplate<DataT>::
    setDefinition(smtk::ConstAttributeItemDefinitionPtr tdef)
    {
      // Note that we do a dynamic cast here since we don't
      // know if the proper definition is being passed
      const DefType *def = 
        dynamic_cast<const DefType *>(tdef.get());
      // Call the parent's set definition - similar to constructor calls
      // we call from base to derived
      if ((def == NULL) || (!ValueItem::setDefinition(tdef)))
        {
        return false;
        }
      int n = def->numberOfRequiredValues();
      if (n)
        {
        if (def->hasDefault())
          {
          // Assumes that if the definition is discrete then default value
          // will be based on the default discrete index
          this->m_values.resize(n, def->defaultValue());
          }
        else
          {
          this->m_values.resize(n);
          }
        }
      return true;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool ValueItemTemplate<DataT>::setValue(int element, const DataT &val)
    {
      const DefType *def = static_cast<const DefType *>(this->definition().get());
      if (def->isDiscrete())
        {
        int index = def->findDiscreteIndex(val);
        if (index != -1)
          {
          this->m_discreteIndices[element] = index;
          this->m_values[element] = val;
          if (def->allowsExpressions())
            {
            this->m_expressions[element]->unset();
            }
          this->m_isSet[element] = true;
          return true;
          }
        return false;
        }
      if (def->isValueValid(val))
        {
        this->m_values[element] = val;
        this->m_isSet[element] = true;
        return true;
        }
      return false;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    void ValueItemTemplate<DataT>::updateDiscreteValue(int element)
    {
      const DefType *def = static_cast<const DefType *>(this->definition().get());
      this->m_values[element] =
        def->discreteValue(this->m_discreteIndices[element]);
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    std::string
    ValueItemTemplate<DataT>::valueAsString(int element, 
                                          const std::string &format) const
    {
      // For the initial design we will use sprintf and force a limit of 300 char
      if (this->m_isSet[element])
        {
        if (this->isExpression(element))
          {
          return "VALUE_IS_EXPRESSION";
          }
        else
          {
          char dummy[300];
          sprintf(dummy, format.c_str(), this->m_values[element]);
          return dummy;
          }
        }
      return "VALUE_IS_NOT_SET";
    }
//----------------------------------------------------------------------------
    template<>
    inline std::string
    ValueItemTemplate<std::string>::valueAsString(int element, 
                                          const std::string &format) const
    {
      // For the initial design we will use sprintf and force a limit of 300 char
      if (this->m_isSet[element])
        {
        if (this->isExpression(element))
          {
          return "VALUE_IS_EXPRESSION";
          }
        else
          {
          char dummy[300];
          if (format == "")
            {
            sprintf(dummy, "%s", this->m_values[element].c_str());
            }
          else
            {
            sprintf(dummy, format.c_str(), this->m_values[element].c_str());
            }
          return dummy;
          }
        }
      return "VALUE_IS_NOT_SET";
    }
//----------------------------------------------------------------------------
    template<>
    inline std::string
    ValueItemTemplate<int>::valueAsString(int element, 
                                               const std::string &format) const
    {
      // For the initial design we will use sprintf and force a limit of 300 char
      if (this->m_isSet[element])
        {
        if (this->isExpression(element))
          {
          return "VALUE_IS_EXPRESSION";
          }
        else
          {
          char dummy[300];
          if (format == "")
            {
            sprintf(dummy, "%d", this->m_values[element]);
            }
          else
            {
            sprintf(dummy, format.c_str(), this->m_values[element]);
            }
          return dummy;
          }
        }
      return "VALUE_IS_NOT_SET";
    }
//----------------------------------------------------------------------------
    template<>
    inline std::string
    ValueItemTemplate<double>::valueAsString(int element, 
                                               const std::string &format) const
    {
      // For the initial design we will use sprintf and force a limit of 300 char
      if (this->m_isSet[element])
        {
        if (this->isExpression(element))
          {
          return "VALUE_IS_EXPRESSION";
          }
        else
          {
          char dummy[300];
          if (format == "")
            {
            sprintf(dummy, "%g", this->m_values[element]);
            }
          else
            {
            sprintf(dummy, format.c_str(), this->m_values[element]);
            }
          return dummy;
          }
        }
      return "VALUE_IS_NOT_SET";
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool
    ValueItemTemplate<DataT>::appendValue(const DataT &val)
    {
      //First - are we allowed to change the number of values?
      const DefType *def = static_cast<const DefType *>(this->definition().get());
      int n = def->numberOfRequiredValues();
      if (n)
        {
        return false; // The number of values is fixed
        }
      
      if (def->isDiscrete())
        {
        int index = def->findDiscreteIndex(val);
        if (index != -1)
          {
          this->m_values.push_back(val);
          this->m_discreteIndices.push_back(index);
          this->m_isSet.push_back(true);
          if (def->allowsExpressions())
            {
            int nextPos = this->m_expressions.size();
            this->m_expressions.resize(nextPos+1);
            def->buildExpressionItem(this, nextPos);
            }
          return true;
          }
        return false;
        }
      if (def->isValueValid(val))
        {
        if (def->allowsExpressions())
          {
          int nextPos = this->m_expressions.size();
          this->m_expressions.resize(nextPos+1);
          def->buildExpressionItem(this, nextPos);
          }
        this->m_values.push_back(val);
        this->m_isSet.push_back(true);
        return true;
        }
      return false;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool
    ValueItemTemplate<DataT>::setNumberOfValues(std::size_t newSize)
    {
      // If the current size is the same just return
      if (this->numberOfValues() == newSize)
        {
        return true;
        }

      //Next - are we allowed to change the number of values?
      const DefType *def = static_cast<const DefType *>(this->definition().get());
      std::size_t n = def->numberOfRequiredValues();
      if (n)
        {
        return false; // The number of values is fixed
        }
      // Are we increasing or decreasing?
      if (newSize < n)
        {
        if (def->allowsExpressions())
          {
          int i;
          for (i = newSize; i < n; i++)
            {
            this->m_expressions[i]->detachOwningItem();
            }
          this->m_expressions.resize(newSize);
          }
        this->m_values.resize(newSize);
        this->m_isSet.resize(newSize);
        if (def->isDiscrete())
          {
          this->m_discreteIndices.resize(newSize);
          }
        return true;
        }
      if (def->hasDefault())
        {
        this->m_values.resize(newSize, def->defaultValue());
        this->m_isSet.resize(newSize, true);
        }
      else
        {
        this->m_values.resize(newSize);
        this->m_isSet.resize(newSize, false);
        }
      if (def->isDiscrete())
        {
        this->m_discreteIndices.resize(newSize, def->defaultDiscreteIndex());
        }
      if (def->allowsExpressions())
        {
        int i;
        this->m_expressions.resize(newSize);
        for (i = n; i < newSize; i++)
          {
          def->buildExpressionItem(this, i);
          }
        }
      return true;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool
    ValueItemTemplate<DataT>::removeValue(int element)
    {
      //First - are we allowed to change the number of values?
      const DefType *def = static_cast<const DefType *>(this->definition().get());
      int n = def->numberOfRequiredValues();
      if (n)
        {
        return false; // The number of values is fixed
        }
      if (def->allowsExpressions())
        {
        this->m_expressions[element]->detachOwningItem();
        this->m_expressions.erase(this->m_expressions.begin()+element);
        }
      this->m_values.erase(this->m_values.begin()+element);
      this->m_isSet.erase(this->m_isSet.begin()+element);
      if (def->isDiscrete())
        {
        this->m_discreteIndices.erase(this->m_discreteIndices.begin()+element);
        }
      return true;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool
    ValueItemTemplate<DataT>::setToDefault(int element)
    {
      const DefType *def = static_cast<const DefType *>(this->definition().get());
      if (!def->hasDefault())
        {
        return false; // Doesn't have a default value
        }

      if (def->isDiscrete())
        {
        this->setDiscreteIndex(element, def->defaultDiscreteIndex());
        }
      else
        {
        this->setValue(element, def->defaultValue());
        }
      return true;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    void
    ValueItemTemplate<DataT>::reset()
    {
      const DefType *def = static_cast<const DefType *>(this->definition().get());
      // Was the initial size 0?
      int i, n = def->numberOfRequiredValues();
      if (!n)
        {
        this->m_values.clear();
        this->m_isSet.clear();
        this->m_discreteIndices.clear();
        if (def->allowsExpressions())
          {
          int j, m = this->m_expressions.size();
          for (j = 0; j < m; j++)
            {
            this->m_expressions[j]->detachOwningItem();
            }
          this->m_expressions.clear();
          }
        return;
        }

      // If we can have expressions then clear them
      if (def->allowsExpressions())
        {
        for (i = 0; i < n; i++)
          {
          this->m_expressions[i]->unset();
          }
        }
      if (!def->hasDefault()) // Do we have default values
        {
        for (i = 0; i < n; i++)
          {
          this->unset(i);
          }
        return;
        }

      if (def->isDiscrete())
        {
        int index = def->defaultDiscreteIndex();
        for(i = 0; i < n; i++)
          {
          this->setDiscreteIndex(i, index);
          }
        }
      else
        {
        DataT val = def->defaultValue();
        for(i = 0; i < n; i++)
          {
          this->setValue(i, val);
          }
        }
      ValueItem::reset();
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool
    ValueItemTemplate<DataT>::appendExpression(smtk::AttributePtr exp)
    {
      // See if the parent class appended the expression
      if (ValueItem::appendExpression(exp))
        {
        // Resize the values array to match
        this->m_values.resize(this->m_expressions.size());
        return true;
        }
      return false;
    }
  };
};


#endif /* __smtk_attribute_ValueItemTemplate_h */
