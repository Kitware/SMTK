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
// .NAME ValueComponentDefinitionTemplate.h -
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_ValueComponentDefinitionTemplate_h
#define __slctk_attribute_ValueComponentDefinitionTemplate_h

#include "ValueComponentDefinition.h"
#include <iostream>
#include <sstream>

namespace slctk
{
  namespace attribute
  {
    template<typename DataT>
    class ValueComponentDefinitionTemplate :
      public slctk::attribute::ValueComponentDefinition
    {
    public:
      typedef DataT DataType;

      ValueComponentDefinitionTemplate(const std::string &myname);
      virtual ~ValueComponentDefinitionTemplate() {}
      
      const DataT &defaultValue() const
      {return this->m_defaultValue;}
      void setDefaultValue(const DataT &val);
      const DataT &discreteValue(int element) const
      {return this->m_discreteValues[element];}
      void addDiscreteValue(const DataT &val);
      void addDiscreteValue(const DataT &val, const std::string &label);
      virtual bool hasRange() const
      {return this->m_minRangeSet || this->m_maxRangeSet;}
      bool hasMinRange() const
      {return this->m_minRangeSet;}
      const DataT &minRange() const
      {return this->m_minRange;}
      bool minRangeInclusive() const
      {return this->m_minRangeInclusive;}
      bool setMinRange(const DataT &minVal, bool isInclusive);
      bool hasMaxRange() const
      {return this->m_maxRangeSet;}
      const DataT &maxRange() const
      {return this->m_maxRange;}
      bool maxRangeInclusive() const
      {return this->m_maxRangeInclusive;}
      bool setMaxRange(const DataT &maxVal, bool isInclusive);
      void clearRange();
      int findDiscreteIndex(const DataT &val) const;
      bool isValueValid(const DataT &val) const;
    protected:
      DataT m_defaultValue;
      DataT m_minRange;
      bool m_minRangeSet;
      bool m_minRangeInclusive;
      DataT m_maxRange;
      bool m_maxRangeSet;
      bool m_maxRangeInclusive;
      std::vector<DataT> m_discreteValues;
    private:
      
    };

//----------------------------------------------------------------------------
    template<typename DataT>
    ValueComponentDefinitionTemplate<DataT>::
    ValueComponentDefinitionTemplate(const std::string &myname):
      ValueComponentDefinition(myname)
    {
      this->m_minRangeSet = false;
      this->m_minRangeInclusive = false;
      this->m_maxRangeSet = false;
      this->m_maxRangeInclusive = false;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    void ValueComponentDefinitionTemplate<DataT>::
    setDefaultValue(const DataT &dvalue)
    {
      this->m_defaultValue = dvalue;
      this->m_hasDefault = true;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    void ValueComponentDefinitionTemplate<DataT>::
    addDiscreteValue(const DataT &dvalue)
    {
      // Set the label to be based on the value
      std::ostringstream oss;
      oss << dvalue;
      this->addDiscreteValue(dvalue, oss.str());
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    void ValueComponentDefinitionTemplate<DataT>::
    addDiscreteValue(const DataT &dvalue, const std::string &dlabel)
    {
      this->m_discreteValues.push_back(dvalue);
      this->m_discreteValueLables.push_back(dlabel);
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool ValueComponentDefinitionTemplate<DataT>::
    setMinRange(const DataT &minVal, bool isInclusive)
    {
      if ((!this->m_maxRangeSet) || (minVal < this->m_maxRange))
        {
        this->m_minRangeSet = true;
        this->m_minRange = minVal;
        this->m_minRangeInclusive = isInclusive;
        return true;
        }
      return false;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool ValueComponentDefinitionTemplate<DataT>::
    setMaxRange(const DataT &maxVal, bool isInclusive)
    {
      if ((!this->m_minRangeSet) || (maxVal > this->m_minRange))
        {
        this->m_maxRangeSet = true;
        this->m_maxRange = maxVal;
        this->m_maxRangeInclusive = isInclusive;
        return true;
        }
      return false;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    void ValueComponentDefinitionTemplate<DataT>::
    clearRange()
    {
      this->m_minRangeSet = false;
      this->m_maxRangeSet = false;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    int ValueComponentDefinitionTemplate<DataT>::
    findDiscreteIndex(const DataT &val) const
    {
      // Are we dealing with Discrete Values?
      if (!this->isDiscrete())
        {
        return -1;
        }
      int i, n = this->m_discreteValues.size();
      for (i = 0; i < n; i++)
        {
        if (this->m_discreteValues[i] == val)
          {
          return i;
          }
        }
      return -1;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool ValueComponentDefinitionTemplate<DataT>::
    isValueValid(const DataT &val) const
    {
      // Are we dealing with Discrete Values?
      if (this->isDiscrete())
        {
        return (this->findDiscreteIndex(val) != -1);
        }
      if (!this->hasRange())
        {
        return true;
        }
      if (this->m_minRangeSet && 
          ((val < this->m_minRange) || 
           ((!this->m_minRangeInclusive) && (val == this->m_minRange))))
        {
        return false;
        }
      if (this->m_maxRangeSet && 
          ((val > this->m_maxRange) || 
           ((!this->m_maxRangeInclusive) && (val == this->m_maxRange))))
        {
        return false;
        }
      return true;
    }
  };
};

#endif /* __slctk_attribute_ValueComponentDefinitionTemplate_h */
