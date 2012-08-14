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
// .NAME ValueComponent.h -
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_ValueComponent_h
#define __slctk_attribute_ValueComponent_h

#include "attribute/Component.h"
#include "attribute/PublicPointerDefs.h"
#include "AttributeExports.h"

namespace slctk
{
  namespace attribute
  {
    class Attribute;
    class AttributeReferenceComponent;
    class ValueComponentDefinition;
    class SLCTKATTRIBUTE_EXPORT ValueComponent : public slctk::attribute::Component
    {
    public:
      ValueComponent();
      virtual ~ValueComponent();
      std::size_t numberOfValues() const
      {return this->m_isSet.size();}

      virtual bool setDefinition(slctk::ConstAttributeComponentDefinitionPtr def);
      bool allowsExpressions() const;
      bool isExpression() const
      {return this->isExpression(0);}
      bool isExpression(int elementIndex) const
      { return (this->expression(elementIndex) != NULL);}
      slctk::AttributePtr expression() const
      {return this->expression(0);}
      slctk::AttributePtr expression(int elementIndex) const;
      bool setExpression(slctk::AttributePtr exp)
      {return this->setExpression(0, exp);}
      bool setExpression(int elementIndex, slctk::AttributePtr exp);
      virtual bool appendExpression(slctk::AttributePtr exp);

      int discreteIndex() const
      { return this->discreteIndex(0);}
      int discreteIndex(int elementIndex) const
      {return this->m_discreteIndices[elementIndex];}
      bool isDiscrete() const;
      
      void setDiscreteIndex(int value)
      {this->setDiscreteIndex(0, value);}
      void setDiscreteIndex(int elementIndex, int value);
      // Reset returns the component to its initial state.
      //If the component is of fixed size, then it's values  to their initial state.  
      // If there is a default available it will use it, else
      // it will be marked as unset.
      //If the component's definition indicated a size of 0 then it will go back to 
      // having no values
      virtual void reset() = 0;
      virtual bool setToDefault(int elementIndex) = 0;
      virtual const std::string &valueAsString(const std::string &format) const
      { return this->valueAsString(0, format);}

      virtual const std::string &valueAsString(int elementIndex, const std::string &format) const = 0;
      virtual bool isSet() const
      { return this->m_isSet[0];}
      virtual bool isSet(int elementIndex) const
      {return this->m_isSet[elementIndex];}
      virtual void unset() 
      {this->unset(0);}
      virtual void unset(int elementIndex)
      {this->m_isSet[elementIndex] = false;}

    protected:
      virtual void updateDiscreteValue(int elementIndex) = 0;
      std::vector<int> m_discreteIndices;
      std::vector<bool> m_isSet;
      std::vector<slctk::AttributeReferenceComponentPtr > m_expressions;
      mutable std::string m_tempString;
    private:
      
    };
  };
};

#endif /* __slctk_attribute_ValueComponent_h */
