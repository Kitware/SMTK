//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME ValueItemTemplate.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_ValueItemTemplate_h
#define __smtk_attribute_ValueItemTemplate_h

#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinitionTemplate.h"
#include <cassert>
#include <vector>
#include <stdio.h>
#include <sstream>

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
      typedef typename std::vector<DataT> value_type;
      typedef value_type const_iterator;
      typedef ValueItemDefinitionTemplate<DataType> DefType;

      virtual ~ValueItemTemplate() {}
      typename std::vector<DataT>::const_iterator begin() const
        { return this->m_values.begin(); }
      typename std::vector<DataT>::const_iterator end() const
        { return this->m_values.end(); }
      virtual bool setNumberOfValues(std::size_t newSize);
      DataT value(std::size_t element=0) const
      {return this->m_values[element];}
      virtual std::string valueAsString() const
      {return this->valueAsString(0);}
      virtual std::string valueAsString(std::size_t element) const;
      bool setValue(const DataT &val)
      {return this->setValue(0, val);}
      bool setValue(std::size_t element, const DataT &val);
      template<typename I>
      bool setValues(I vbegin, I vend)
        {
        bool ok = false;
        std::size_t num = vend - vbegin;
        if (this->setNumberOfValues(num))
          {
          ok = true;
          std::size_t i = 0;
          for (I it = vbegin; it != vend; ++it, ++i)
            {
            if (!this->setValue(i, *it))
              {
              ok = false;
              break;
              }
            }
          }
        return ok;
        }
      bool appendValue(const DataT &val);
      virtual bool appendExpression(smtk::attribute::AttributePtr exp);
      bool removeValue(std::size_t element);
      virtual void reset();
      virtual bool setToDefault(std::size_t element=0);
      virtual bool isUsingDefault(std::size_t element) const;
      virtual bool isUsingDefault() const;
      DataT defaultValue() const;
      const std::vector<DataT>& defaultValues() const;
      // Assigns this item to be equivalent to another.  Options are processed by derived item classes
      // Returns true if success and false if a problem occured.  By default, an attribute being used by this
      // to represent an expression will be copied if needed.  Use IGNORE_EXPRESSIONS option to prevent this
      // When an expression attribute is copied, its model associations are by default not.
      // Use COPY_MODEL_ASSOCIATIONS if you want them copied as well.These options are defined in Item.h .
      virtual bool assign(smtk::attribute::ConstItemPtr &sourceItem, unsigned int options = 0);
      shared_ptr<const DefType> concreteDefinition() const
        { return dynamic_pointer_cast<const DefType>(this->definition()); }
    protected:
      ValueItemTemplate(Attribute *owningAttribute, int itemPosition);
      ValueItemTemplate(Item *owningItem, int myPosition, int mySubGroupPosition);
      virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr vdef);
      virtual void updateDiscreteValue(std::size_t element);
      std::vector<DataT> m_values;
      const std::vector<DataT> m_dummy; //(1, DataT());
    private:
    };

//----------------------------------------------------------------------------
    template<typename DataT>
    ValueItemTemplate<DataT>::ValueItemTemplate(Attribute *owningAttribute,
                                                int itemPosition):
      ValueItem(owningAttribute, itemPosition), m_dummy(1, DataT())
    {
    }

//----------------------------------------------------------------------------
    template<typename DataT>
    ValueItemTemplate<DataT>::ValueItemTemplate(Item *inOwningItem,
                                                int itemPosition,
                                                int mySubGroupPosition):
      ValueItem(inOwningItem, itemPosition, mySubGroupPosition),
      m_dummy(1, DataT())
    {
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool ValueItemTemplate<DataT>::
    setDefinition(smtk::attribute::ConstItemDefinitionPtr tdef)
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
      size_t n = def->numberOfRequiredValues();
      if (n)
        {
        if (def->hasDefault())
          {
          // Assumes that if the definition is discrete then default value
          // will be based on the default discrete index
          if (def->defaultValues().size() > 1)
            {
            this->m_values = def->defaultValues();
            }
          else
            {
            this->m_values.resize(n, def->defaultValue());
            }
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
    bool ValueItemTemplate<DataT>::setValue(size_t element, const DataT &val)
    {
      const DefType *def = static_cast<const DefType *>(this->definition().get());
      if (def->isDiscrete())
        {
        int index = def->findDiscreteIndex(val);
        // Is this the current value?
        assert(this->m_discreteIndices.size() > element);
        if (index == this->m_discreteIndices[element] )
          {
          return true;
          }

        if (index != -1)
          {
          this->m_discreteIndices[element] = index;
          assert(this->m_values.size() > element);
          this->m_values[element] = val;
          if (def->allowsExpressions())
            {
            assert(this->m_expressions.size() > element);
            this->m_expressions[element]->unset();
            }
          assert(this->m_isSet.size() > element);
          this->m_isSet[element] = true;
          // Update active children if needed - note that
          // we currently only support active children based on the
          // 0th value changing
          if (element == 0)
            {
            this->updateActiveChildrenItems();
            }
          return true;
          }
        return false;
        }
      if (def->isValueValid(val))
        {
        assert(this->m_values.size() > element);
        this->m_values[element] = val;
        assert(this->m_isSet.size() > element);
        this->m_isSet[element] = true;
        if (def->allowsExpressions())
          {
          assert(this->m_expressions.size() > element);
          this->m_expressions[element]->unset();
          }
        return true;
        }
      return false;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    void ValueItemTemplate<DataT>::updateDiscreteValue(std::size_t element)
    {
      const DefType *def = static_cast<const DefType *>(this->definition().get());
      this->m_values[element] =
        def->discreteValue(static_cast<size_t>(this->m_discreteIndices[element]));
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    std::string
    ValueItemTemplate<DataT>::valueAsString(std::size_t element) const
    {
      assert(this->m_isSet.size() > element);
      if (this->m_isSet[element])
        {
        if (this->isExpression(element))
          {
          return "VALUE_IS_EXPRESSION";
          }
        else
          {
          std::stringstream buffer;
          assert(this->m_values.size() > element);
          buffer << this->m_values[element];
          return buffer.str();
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
      if (!this->isExtensible())
        {
        return false; // The number of values is fixed
        }

      std::size_t n = this->maxNumberOfValues();
      if (n && (this->numberOfValues() >= n))
        {
        return false; // max number reached
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
            std::size_t nextPos = this->m_expressions.size();
            this->m_expressions.resize(nextPos+1);
            def->buildExpressionItem(this, static_cast<int>(nextPos));
            }
          return true;
          }
        return false;
        }
      if (def->isValueValid(val))
        {
        if (def->allowsExpressions())
          {
          std::size_t nextPos = this->m_expressions.size();
          this->m_expressions.resize(nextPos+1);
          def->buildExpressionItem(this, static_cast<int>(nextPos));
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
      if (!this->isExtensible())
        {
        return false; // The number of values is fixed
        }

      // Is this size between the required number and the max?
      if (newSize < this->numberOfRequiredValues())
        {
        return false;
        }

      std::size_t n = this->maxNumberOfValues();
      if (n && (newSize > n))
        {
        return false; // greater than max number
        }

      const DefType *def = static_cast<const DefType *>(this->definition().get());
      n = this->numberOfValues();
      // Are we increasing or decreasing?
      if (newSize < n)
        {
        if (def->allowsExpressions())
          {
          std::size_t i;
          assert(this->m_expressions.size() >= n);
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
        if (def->defaultValues().size() == newSize)
          {
          this->m_values = def->defaultValues();
          }
        else
          {
          this->m_values.resize(newSize, def->defaultValue());
          }
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
        std::size_t i;
        this->m_expressions.resize(newSize);
        for (i = n; i < newSize; i++)
          {
          def->buildExpressionItem(this, static_cast<int>(i));
          }
        }
      return true;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool
    ValueItemTemplate<DataT>::removeValue(std::size_t element)
    {
      //First - are we allowed to change the number of values?
      const DefType *def = static_cast<const DefType *>(this->definition().get());
      if (!this->isExtensible())
        {
        return false; // The number of values is fixed
        }
      if (this->numberOfValues() <= this->numberOfRequiredValues())
        {
        return false; // min number of values reached
        }
      if (def->allowsExpressions())
        {
        assert(this->m_expressions.size() > element);
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
    ValueItemTemplate<DataT>::setToDefault(std::size_t element)
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
        assert(def->defaultValues().size() > element);
        this->setValue(element,
          def->defaultValues().size() > 1 ?
          def->defaultValues()[element] :
          def->defaultValue());
        }
      return true;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool
    ValueItemTemplate<DataT>::isUsingDefault() const
    {
      const DefType *def = static_cast<const DefType *>(this->definition().get());
      if (!def->hasDefault())
        {
        return false; // Doesn't have a default value
        }

      std::size_t i, n = this->numberOfValues();
      DataT dval = def->defaultValue();
      const std::vector<DataT>& dvals = def->defaultValues();
      bool vectorDefault = (dvals.size() == n);
      for (i = 0; i < n; i++)
        {
        assert(this->m_isSet.size() > i);
        assert(this->m_values.size() > i);
        if (!(this->m_isSet[i] && (vectorDefault ? this->m_values[i] == dvals[i] : this->m_values[i] == dval)))
          {
          return false;
          }
        }
      return true;
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool
    ValueItemTemplate<DataT>::isUsingDefault(std::size_t element) const
    {
      const DefType *def = static_cast<const DefType *>(this->definition().get());
      assert(this->m_isSet.size() > element);
      if (!(def->hasDefault() && this->m_isSet[element]))
        {
        return false; // Doesn't have a default value
        }

      DataT dval = def->defaultValue();
      const std::vector<DataT>& dvals = def->defaultValues();
      bool vectorDefault = (dvals.size() == def->numberOfRequiredValues());
      assert(this->m_values.size() > element);
      return (vectorDefault ?
        this->m_values[element] == dvals[element] :
        this->m_values[element] == dval);
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    DataT
    ValueItemTemplate<DataT>::defaultValue() const
    {
      const DefType *def = static_cast<const DefType *>(this->definition().get());
      if (!def)
        return this->m_dummy[0];

      return def->defaultValue();
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    const std::vector<DataT>&
    ValueItemTemplate<DataT>::defaultValues() const
    {
      const DefType *def = static_cast<const DefType *>(this->definition().get());
      if (!def)
        return this->m_dummy;

      return def->defaultValues();
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    void
    ValueItemTemplate<DataT>::reset()
    {
      const DefType *def = static_cast<const DefType *>(this->definition().get());
      // Was the initial size 0?
      std::size_t i, n = this->numberOfRequiredValues();
      if (this->numberOfValues() != n)
        {
        this->setNumberOfValues(n);
        }
      if (!n)
        {
        this->m_values.clear();
        this->m_isSet.clear();
        this->m_discreteIndices.clear();
        if (def->allowsExpressions())
          {
          std::size_t j, m = this->m_expressions.size();
          for (j = 0; j < m; j++)
            {
            this->m_expressions[j]->detachOwningItem();
            }
          this->m_expressions.clear();
          }
        ValueItem::reset();
        return;
        }

      // If we can have expressions then clear them
      if (def->allowsExpressions())
        {
        assert(this->m_expressions.size() >= n);
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
        ValueItem::reset();
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
        DataT dval = def->defaultValue();
        const std::vector<DataT>& dvals = def->defaultValues();
        bool vectorDefault = (dvals.size() == n);
        for(i = 0; i < n; i++)
          {
          this->setValue(i, vectorDefault ? dvals[i] : dval);
          }
        }
      ValueItem::reset();
    }
//----------------------------------------------------------------------------
    template<typename DataT>
    bool
    ValueItemTemplate<DataT>::appendExpression(smtk::attribute::AttributePtr exp)
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
//----------------------------------------------------------------------------
    template<typename DataT>
    bool
    ValueItemTemplate<DataT>::
    assign(smtk::attribute::ConstItemPtr &sourceItem,
           unsigned int options)
    {
      if (!ValueItem::assign(sourceItem, options))
        {
        return false;
        }
      // Cast input pointer to ValueItemTemplate
      const ValueItemTemplate<DataT> *sourceValueItemTemplate =
        dynamic_cast<const ValueItemTemplate<DataT> *>(sourceItem.get());

      if (!sourceValueItemTemplate)
        {
        return false; // Source is not the right type of item
        }
      // Update values
      this->setNumberOfValues(sourceValueItemTemplate->numberOfValues());
      for (std::size_t i=0; i<sourceValueItemTemplate->numberOfValues(); ++i)
        {
        if (sourceValueItemTemplate->isSet(i) &&
            !sourceValueItemTemplate->isExpression(i) &&
            !sourceValueItemTemplate->isDiscrete())
          {
          this->setValue(i, sourceValueItemTemplate->value(i));
          }
        } // for
      return true;
      }
  }  // namespace attribute
}  // namespace smtk

#endif /* __smtk_attribute_ValueItemTemplate_h */
