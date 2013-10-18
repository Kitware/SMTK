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
// .NAME RefItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_RefItemDefinition_h
#define __smtk_attribute_RefItemDefinition_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/ItemDefinition.h"

namespace smtk
{
  namespace attribute
  {
    class Attribute;
    class SMTKCORE_EXPORT RefItemDefinition:
      public ItemDefinition
    {
    public:
      static smtk::attribute::RefItemDefinitionPtr New(const std::string &myName)
      { return smtk::attribute::RefItemDefinitionPtr(new RefItemDefinition(myName));}

      // This method is for wrapping code.  C++ developers should use smtk::dynamic_pointer_cast
      static smtk::attribute::RefItemDefinitionPtr
        CastTo(const smtk::attribute::ItemDefinitionPtr &p)
      {return smtk::dynamic_pointer_cast<RefItemDefinition>(p);}

      virtual ~RefItemDefinition();

      virtual Item::Type type() const;
      smtk::attribute::DefinitionPtr attributeDefinition() const
      {return this->m_definition.lock();}

      void setAttributeDefinition(smtk::attribute::DefinitionPtr def)
      {this->m_definition = def;}

      bool isValueValid(smtk::attribute::AttributePtr att) const;

      virtual smtk::attribute::ItemPtr buildItem(Attribute *owningAttribute,
                                                int itemPosition) const;
      virtual smtk::attribute::ItemPtr buildItem(Item *owningItem,
                                                int position,
                                                int subGroupPosition) const;
      int numberOfRequiredValues() const
      {return this->m_numberOfRequiredValues;}
      void setNumberOfRequiredValues(int esize);

      bool hasValueLabels() const
      {return this->m_valueLabels.size();}

      void setValueLabel(int element, const std::string &elabel);
      void setCommonValueLabel(const std::string &elabel);
      bool usingCommonLabel() const
      {return this->m_useCommonLabel;}

      std::string valueLabel(int element) const;
    protected:
      RefItemDefinition(const std::string &myName);
      smtk::attribute::WeakDefinitionPtr m_definition;
      bool m_useCommonLabel;
      std::vector<std::string> m_valueLabels;
      int m_numberOfRequiredValues;
    private:

    };
  }
}

#endif /* __smtk_attribute_RefItemDefinition_h */
