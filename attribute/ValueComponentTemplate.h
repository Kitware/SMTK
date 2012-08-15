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
// .NAME ValueComponentTemplate.h -
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_ValueComponentTemplate_h
#define __slctk_attribute_ValueComponentTemplate_h

#include "attribute/AttributeReferenceComponent.h"
#include "attribute/ValueComponent.h"
#include "attribute/ValueComponentDefinitionTemplate.h"
#include <vector>

namespace slctk
{
  namespace attribute
  {
    template<typename DataT>
    class ValueComponentTemplate : public ValueComponent
    {
      //template<DataT> friend class ValueComponentDefinitionTemplate;
    public:
      typedef DataT DataType;
      typedef ValueComponentDefinitionTemplate<DataType> DefType;
      
      ValueComponentTemplate();
      virtual ~ValueComponentTemplate() {}
      virtual bool setDefinition(slctk::ConstAttributeComponentDefinitionPtr vdef);
      DataT value(int element=0) const
      {return this->m_values[element];}
      virtual std::string valueAsString(const std::string &format="") const
      {return this->valueAsString(0, format);}
      virtual std::string valueAsString(int element, const std::string &format="") const;
      bool setValue(const DataT &val)
      {return this->setValue(0, val);}
      bool setValue(int element, const DataT &val);
      bool appendValue(const DataT &val);
      virtual bool appendExpression(slctk::AttributePtr exp);
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
    ValueComponentTemplate<DataT>::ValueComponentTemplate()
    {
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool ValueComponentTemplate<DataT>::
    setDefinition(slctk::ConstAttributeComponentDefinitionPtr tdef)
    {
      // Note that we do a dynamic cast here since we don't
      // know if the proper definition is being passed
      const DefType *def = 
        dynamic_cast<const DefType *>(tdef.get());
      // Call the parent's set definition - similar to constructor calls
      // we call from base to derived
      if ((def == NULL) || (!ValueComponent::setDefinition(tdef)))
        {
        return false;
        }
      int n = def->numberOfValues();
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
    bool ValueComponentTemplate<DataT>::setValue(int element, const DataT &val)
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
    void ValueComponentTemplate<DataT>::updateDiscreteValue(int element)
    {
      const DefType *def = static_cast<const DefType *>(this->definition().get());
      this->m_values[element] =
        def->discreteValue(this->m_discreteIndices[element]);
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    std::string
    ValueComponentTemplate<DataT>::valueAsString(int element, 
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
    ValueComponentTemplate<std::string>::valueAsString(int element, 
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
    ValueComponentTemplate<int>::valueAsString(int element, 
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
    ValueComponentTemplate<double>::valueAsString(int element, 
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
    ValueComponentTemplate<DataT>::appendValue(const DataT &val)
    {
      //First - are we allowed to change the number of values?
      const DefType *def = static_cast<const DefType *>(this->definition());
      int n = def->numberOfValues();
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
            this->m_expressions.
              push_back(def->buildExpressionComponent());
            }
          return true;
          }
        return false;
        }
      if (def->isValueValid(val))
        {
        if (def->allowsExpressions())
          {
          this->m_expressions.
            push_back(def->expressionDefinition()->buildComponent());
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
    ValueComponentTemplate<DataT>::removeValue(int element)
    {
      //First - are we allowed to change the number of values?
      const DefType *def = static_cast<const DefType *>(this->definition());
      int n = def->numberOfValues();
      if (n)
        {
        return false; // The number of values is fixed
        }
      if (def->allowsExpressions)
        {
        delete this->m_expressions[element];
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
    ValueComponentTemplate<DataT>::setToDefault(int element)
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
    ValueComponentTemplate<DataT>::reset()
    {
      const DefType *def = static_cast<const DefType *>(this->definition().get());
      // Was the initial size 0?
      int i, n = def->numberOfValues();
      if (!n)
        {
        this->m_values.clear();
        this->m_isSet.clear();
        this->m_discreteIndices.clear();
        if (def->allowsExpressions())
          {
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
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool
    ValueComponentTemplate<DataT>::appendExpression(slctk::AttributePtr exp)
    {
      // See if the parent class appended the expression
      if (ValueComponent::appendExpression(exp))
        {
        // Resize the values array to match
        this->m_values.resize(this->m_expressions.size());
        return true;
        }
      return false;
    }
  };
};


#endif /* __slctk_attribute_ValueComponentTemplate_h */
