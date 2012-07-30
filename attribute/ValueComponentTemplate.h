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

#include "attribute/ValueComponent.h"
#include "attribute/ValueComponentDefinitionTemplate.h"
#include <vector>

namespace slctk
{
  namespace attribute
  {
    template<typename DataT, typename DefT>
    class ValueComponentTemplate : public ValueComponent
    {
    public:
      int numberOfValues() const
      {return this->m_values.size();}
      DataT value() const
      {return this->m_values[0];}
      DataT value(int element) const
      {return this->m_values[element];}
      virtual const std::string &valueAsString(int element, const std::string &format) const;
      bool setValue(const DataT &val)
      {return this->setValue(0, val);}
      bool setValue(int element, const DataT &val);
      bool appendValue(const DataT &val);
      bool removeValue(int element);
      virtual reset();
      bool setToDefault(int element);

    protected:
      ValueComponentTemplate(DefT *def);
      virtual ~ValueComponentTemplate() {}
      virtual void updateDiscreteValue(int element);
      std::vector<DataT> m_values;
    private:
    };

//----------------------------------------------------------------------------
    template<typename DataT, typename DefT>
    ValueComponentTemplate::ValueComponentTemplate(DataT *def):
      ValueComponent(def)
    {
      int n = def->numberOfValues();
      if (n)
        {
        if (def->hasDefault())
          {
          // Assumes that if the definition is discrete then default value
          // will be based on the default discrete index
          this->m_values.resize(n, def->discreteValue(def->defaultValue()));
          }
        else
          {
          this->m_values.resize(n);
          }
        }
    }
//----------------------------------------------------------------------------
    template<typename DataT, typename DefT>
    bool ValueComponentTemplate::setValue(int element, const DataT &val)
    {
      const DefT *def = static_cast<const DefT *>(this->definition());
      if (def->isDiscrete())
        {
        int index = def->findDiscreteIndex(val);
        if (index != -1)
          {
          this->m_discreteIndices[element] = index;
          this->m_values[element] = val;
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
    template<typename DataT, typename DefT>
    void ValueComponentTemplate::updateDiscreteValue(int element)
    {
      const DefT *def = static_cast<const DefT *>(this->definition());
      this->m_values[element] = def->discreteValue(this->discreteIndex[element]);
    }
//----------------------------------------------------------------------------
    template<typename DataT, typename DefT>
    const std::string &
    ValueComponentTemplate::valueAsString(int element, 
                                          const std::string &format) const
    {
      // For the initial design we will use sprintf and force a limit of 300 char
      char dummy[300];
      sprintf(dummy, format.c_str(), this->m_values[element]);
      this->m_tempString = dummy;
      return this->m_tempString;
    }
//----------------------------------------------------------------------------
    template<typename DataT, typename DefT>
    bool
    ValueComponentTemplate::appendValue(const DataT &val)
    {
      //First - are we allowed to change the number of values?
      const DefT *def = static_cast<const DefT *>(this->definition());
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
          return true;
          }
        return false;
        }
      if (def->isValueValid(val))
        {
        this->m_values.push_back(val);
        this->m_isSet.push_back(true);
        return true;
        }
      return false;
    }
//----------------------------------------------------------------------------
    template<typename DataT, typename DefT>
    bool
    ValueComponentTemplate::removeValue(int element)
    {
      //First - are we allowed to change the number of values?
      const DefT *def = static_cast<const DefT *>(this->definition());
      int n = def->numberOfValues();
      if (n)
        {
        return false; // The number of values is fixed
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
    template<typename DataT, typename DefT>
    bool
    ValueComponentTemplate::setToDefault(int element)
    {
      const DefT *def = static_cast<const DefT *>(this->definition());
      if (!def->hasDefault())
        {
        return false; // Doesn't have a default value
        }

      if (def->isDiscrete())
        {
        this->setDiscreteIndex(element, def->discreteDefaultIndex());
        }
      else
        {
        this->setValue(element, def->defaultValue());
        }
      return true;
    }
//----------------------------------------------------------------------------
    template<typename DataT, typename DefT>
    void
    ValueComponentTemplate::reset()
    {
      const DefT *def = static_cast<const DefT *>(this->definition());
      // Was the initial size 0?
      int i, n = def->numberOfValues();
      if (!n)
        {
        this->m_values.clear();
        this->m_isSet.clear();
        this->m_discreteIndices.clear();
        return;
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
        int index = def->discreteDefaultIndex();
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
  };
};


#endif /* __slctk_attribute_ValueComponentTemplate_h */
