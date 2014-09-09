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
// .NAME ValueItemDefinitionTemplate.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_ValueItemDefinitionTemplate_h
#define __smtk_attribute_ValueItemDefinitionTemplate_h

#include "smtk/attribute/ValueItemDefinition.h"
#include <iostream>
#include <sstream>

namespace smtk
{
  namespace attribute
  {
    template<typename DataT>
    class ValueItemDefinitionTemplate :
      public smtk::attribute::ValueItemDefinition
    {
    public:
      typedef DataT DataType;

      virtual ~ValueItemDefinitionTemplate() {}

      const DataT &defaultValue() const
      {return this->m_defaultValue;}
      bool setDefaultValue(const DataT &val);
      const DataT &discreteValue(std::size_t element) const
      {return this->m_discreteValues[element];}
      void addDiscreteValue(const DataT &val);
      void addDiscreteValue(const DataT &val, const std::string &discreteEnum);
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
      ValueItemDefinitionTemplate(const std::string &myname);
      void copyTo(ValueItemDefinitionPtr def,
        smtk::attribute::ItemDefinition::CopyInfo& info) const;
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
    ValueItemDefinitionTemplate<DataT>::
    ValueItemDefinitionTemplate(const std::string &myname):
      ValueItemDefinition(myname)
    {
      this->m_minRangeSet = false;
      this->m_minRangeInclusive = false;
      this->m_maxRangeSet = false;
      this->m_maxRangeInclusive = false;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool ValueItemDefinitionTemplate<DataT>::
    setDefaultValue(const DataT &dvalue)
    {
      // Is this a vaild value?
      if (!this->isValueValid(dvalue))
        {
        return false;
        }
      this->m_defaultValue = dvalue;
      this->m_hasDefault = true;
      return true;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    void ValueItemDefinitionTemplate<DataT>::
    addDiscreteValue(const DataT &dvalue)
    {
      // Set the label to be based on the value
      std::ostringstream oss;
      oss << dvalue;
      this->addDiscreteValue(dvalue, oss.str());
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    void ValueItemDefinitionTemplate<DataT>::
    addDiscreteValue(const DataT &dvalue, const std::string &dlabel)
    {
      this->m_discreteValues.push_back(dvalue);
      this->m_discreteValueEnums.push_back(dlabel);
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool ValueItemDefinitionTemplate<DataT>::
    setMinRange(const DataT &minVal, bool isInclusive)
    {
      // If there is a default value is it within the new range?
      if (this->m_hasDefault)
        {
        if (this->m_defaultValue < minVal)
          {
          return false;
          }
        if ((!isInclusive) && (this->m_defaultValue == minVal))
          {
          return false;
          }
        }
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
    bool ValueItemDefinitionTemplate<DataT>::
    setMaxRange(const DataT &maxVal, bool isInclusive)
    {
      // If there is a default value is it within the new range?
      if (this->m_hasDefault)
        {
        if (this->m_defaultValue > maxVal)
          {
          return false;
          }
        if ((!isInclusive) && (this->m_defaultValue == maxVal))
          {
          return false;
          }
        }
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
    void ValueItemDefinitionTemplate<DataT>::
    clearRange()
    {
      this->m_minRangeSet = false;
      this->m_maxRangeSet = false;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    int ValueItemDefinitionTemplate<DataT>::
    findDiscreteIndex(const DataT &val) const
    {
      // Are we dealing with Discrete Values?
      if (!this->isDiscrete())
        {
        return -1;
        }
      std::size_t i, n = this->m_discreteValues.size();
      for (i = 0; i < n; i++)
        {
        if (this->m_discreteValues[i] == val)
          {
          return static_cast<int>(i);
          }
        }
      return -1;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool ValueItemDefinitionTemplate<DataT>::
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

//----------------------------------------------------------------------------
// Copies my contents to input definition
// Input argument is ValueItemDefinition shared pointer, which must be
// cast to (raw) ValueItemTemplateDefinition pointer.
    template<typename DataT>
    void ValueItemDefinitionTemplate<DataT>::
      copyTo(ValueItemDefinitionPtr def,
      smtk::attribute::ItemDefinition::CopyInfo& info) const
    {
    // Get raw pointer and cast to ValueItemDefinitionTemplate*
    ValueItemDefinition *rawDef = def.get();
    ValueItemDefinitionTemplate<DataT> *vdef =
      dynamic_cast<ValueItemDefinitionTemplate<DataT>* >(rawDef);

    if (this->hasDefault())
      {
      vdef->setDefaultValue(m_defaultValue);
      }

    if (m_minRangeSet)
      {
      vdef->setMinRange(m_minRange, m_minRangeInclusive);
      }

    if (m_maxRangeSet)
      {
      vdef->setMaxRange(m_maxRange, m_maxRangeInclusive);
      }

    if (this->isDiscrete())
      {
      // Copy values & labels
      DataT value;
      std::string labelStr;
      for (std::size_t i=0; i<m_discreteValues.size(); ++i)
        {
        value = m_discreteValues[i];
        labelStr = m_discreteValueEnums[i];
        vdef->addDiscreteValue(value, labelStr);
        }
      vdef->setDefaultDiscreteIndex(m_defaultDiscreteIndex);
      }

    // Copy superclass *after* our stuff, so that discrete values are set up
    ValueItemDefinition::copyTo(def, info);
    }

  } // namespace attribute
} // namespace smtk

#endif /* __smtk_attribute_ValueItemDefinitionTemplate_h */
