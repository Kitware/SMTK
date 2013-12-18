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
// .NAME DirectoryItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_DirectoryItemDefinition_h
#define __smtk_attribute_DirectoryItemDefinition_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/ItemDefinition.h"

namespace smtk
{
  namespace attribute
  {
    class Attribute;
    class SMTKCORE_EXPORT DirectoryItemDefinition:
      public ItemDefinition
    {
    public:
      static smtk::attribute::DirectoryItemDefinitionPtr New(const std::string &myName)
      { return smtk::attribute::DirectoryItemDefinitionPtr(new DirectoryItemDefinition(myName));}

      virtual ~DirectoryItemDefinition();

      virtual Item::Type type() const;
      bool isValueValid(const std::string &val) const;

      virtual smtk::attribute::ItemPtr buildItem(Attribute *owningAttribute,
                                                int itemPosition) const;
      virtual smtk::attribute::ItemPtr buildItem(Item *owningItem,
                                                int position,
                                                int subGroupPosition) const;
      size_t numberOfRequiredValues() const
      {return this->m_numberOfRequiredValues;}
      void setNumberOfRequiredValues(size_t esize);

      bool hasValueLabels() const
      {return !this->m_valueLabels.empty();}

      void setValueLabel(size_t element, const std::string &elabel);
      void setCommonValueLabel(const std::string &elabel);
      bool usingCommonLabel() const
      {return this->m_useCommonLabel;}
      std::string valueLabel(size_t element) const;
      bool shouldExist() const
      {return this->m_shouldExist;}
      void setShouldExist(bool val)
      { this->m_shouldExist = val;}
      bool shouldBeRelative() const
      {return this->m_shouldBeRelative;}
      void setShouldBeRelative(bool val)
      {this->m_shouldBeRelative = val;}

    protected:
      DirectoryItemDefinition(const std::string &myName);
      bool m_shouldExist;
      bool m_shouldBeRelative;
      bool m_useCommonLabel;
      std::vector<std::string> m_valueLabels;
      size_t m_numberOfRequiredValues;
    private:

    };
  }
}

#endif /* __smtk_attribute_DirectoryItemDefinition_h */
