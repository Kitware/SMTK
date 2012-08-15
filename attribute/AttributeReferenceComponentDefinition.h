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
// .NAME AttributeReferenceComponentDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_AttributeReferenceComponentDefinition_h
#define __slctk_attribute_AttributeReferenceComponentDefinition_h

#include "AttributeExports.h"
#include "attribute/PublicPointerDefs.h"

#include "ComponentDefinition.h"

namespace slctk
{
  namespace attribute
  {
    class Attribute;
    class Definition;
    class SLCTKATTRIBUTE_EXPORT AttributeReferenceComponentDefinition:
      public ComponentDefinition
    {
    public:
      AttributeReferenceComponentDefinition(const std::string &myName);
      virtual ~AttributeReferenceComponentDefinition();
      
      slctk::AttributeDefinitionPtr attributeDefinition() const
      {return this->m_definition.lock();}

      void setAttributeDefinition(slctk::AttributeDefinitionPtr def)
      {this->m_definition = def;}

      bool isValueValid(slctk::AttributePtr att) const;

      virtual slctk::AttributeComponentPtr buildComponent() const;
      int numberOfValues() const
      {return this->m_numberOfValues;}
      void setNumberOfValues(int esize);

      bool hasValueLables() const
      {return this->m_valueLables.size();}

      void setValueLabel(int element, const std::string &elabel);
      void setCommonValueLable(const std::string &elable);
      std::string valueLable(int element) const;
    protected:
        slctk::WeakAttributeDefinitionPtr m_definition;
        bool m_useCommonLabel;
        std::vector<std::string> m_valueLables;
        int m_numberOfValues;
     private:
      
    };
  };
};

#endif /* __slctk_attribute_AttributeReferenceComponentDefinition_h */
