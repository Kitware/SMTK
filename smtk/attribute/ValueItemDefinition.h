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
// .NAME ValueItemDefinition.h -
// .SECTION Description
// The base class for attribute items that have an input value.
// It can have a specified number of values (m_numberOfRequiredValues > 0)
// or an unbounded number of values (m_numberOfRequredValues == 0).
// m_valueLabels is used to store labels for individual component values
// but can hold a common label that should be used for all components.
// m_valueLabels should only be used if m_numberOfRequiredValues is not 1.
// .SECTION See Also

#ifndef __smtk_attribute_ValueItemDefinition_h
#define __smtk_attribute_ValueItemDefinition_h

#include <string>
#include <set>
#include <vector>

#include "smtk/SMTKCoreExports.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/PublicPointerDefs.h"

namespace smtk
{
  namespace attribute
  {
    class Attribute;
    class AttributeRefItem;
    class AttributeRefItemDefinition;
    class Cluster;
    class ValueItem;
    class SMTKCORE_EXPORT ValueItemDefinition :
      public smtk::attribute::ItemDefinition
    {
    public:
      ValueItemDefinition(const std::string &myname);
      virtual ~ValueItemDefinition();

      const std::string &units() const
      {
        if(this->m_units.length())
          int i = 0;
        return this->m_units;
      }
      void setUnits(const std::string &newUnits)
      { this->m_units = newUnits;}

      bool isDiscrete() const
      {return (this->m_discreteValueEnums.size() != 0);}
      std::size_t numberOfDiscreteValues() const
      {return this->m_discreteValueEnums.size();}
      const std::string &discreteEnum(int ith) const
      {return this->m_discreteValueEnums[ith];}
      int defaultDiscreteIndex() const
      {return this->m_defaultDiscreteIndex;}
      void setDefaultDiscreteIndex(int discreteIndex);

      bool allowsExpressions() const;
      bool isValidExpression(smtk::attribute::AttributePtr exp) const;
      smtk::attribute::DefinitionPtr expressionDefinition() const;
      void setExpressionDefinition(smtk::attribute::DefinitionPtr exp);
      // Should only be called internally by the ValueItem
      void buildExpressionItem(ValueItem *vitem, int position) const;

      bool hasDefault() const
      {return this->m_hasDefault;}

      virtual bool hasRange() const = 0;

      int numberOfRequiredValues() const
      {return this->m_numberOfRequiredValues;}
      void setNumberOfRequiredValues(int esize);

      // Description:
      // Return whether or not there are labels for components.
      // There should only be labels if there is more than a single
      // component (i.e. m_numberOfRequiredValues != 1).
      bool hasValueLabels() const
      {return this->m_valueLabels.size();}

      // Description:
      // Specify whether the components label is coming from a common
      // label that is repeated.
      bool usingCommonLabel() const
      {return this->m_useCommonLabel;}
      void setValueLabel(int element, const std::string &elabel);
      void setCommonValueLabel(const std::string &elabel);
      // Description:
      // Get the component label for specified element component. This
      // takes into account whether to use the common label or specific
      // component label.
      std::string valueLabel(int element) const;
      bool isDiscreteIndexValid(int index) const
      {return ((index > -1) && (index < this->m_discreteValueEnums.size()));}

    protected:

      bool m_hasDefault;
      bool m_useCommonLabel;
      std::vector<std::string> m_valueLabels;
      std::vector<std::string> m_discreteValueEnums;
      int m_defaultDiscreteIndex;
      int m_numberOfRequiredValues;
      std::string m_units;
      smtk::attribute::RefItemDefinitionPtr m_expressionDefinition;
    private:

    };
  }
}

#endif /* __smtk_attribute_ValueItemDefinition_h */
