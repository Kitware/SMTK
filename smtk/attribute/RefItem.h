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
// .NAME RefItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_RefItem_h
#define __smtk_attribute_RefItem_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h"
#include <vector>

namespace smtk
{
  namespace attribute
  {
    class Attribute;
    class RefItemDefinition;
    class ValueItemDefinition;
    class SMTKCORE_EXPORT RefItem : public Item
    {
      friend class RefItemDefinition;
      friend class ValueItemDefinition;
    public:
      virtual ~RefItem();
      virtual Item::Type type() const;
      std::size_t numberOfValues() const
      {return this->m_values.size();}
      bool  setNumberOfValues(std::size_t newSize);
      std::size_t numberOfRequiredValues() const;
      smtk::attribute::AttributePtr value(std::size_t element=0) const
      {return this->m_values[element].lock();}
      bool setValue( smtk::attribute::AttributePtr val)
      {return this->setValue(0, val);}
      bool setValue(std::size_t element, smtk::attribute::AttributePtr val);
      bool appendValue(smtk::attribute::AttributePtr val);
      bool removeValue(std::size_t element);
      virtual void reset();
      virtual std::string valueAsString(const std::string &format="") const
      {return this->valueAsString(0, format);}
      virtual std::string valueAsString(std::size_t element, const std::string &format="") const;
      virtual bool isSet(std::size_t element=0) const
      {return this->m_values[element].lock().get() != NULL;}
      virtual void unset(std::size_t element=0);
      virtual void copyFrom(const smtk::attribute::ItemPtr sourceItem,
                            smtk::attribute::Item::CopyInfo& info);

    protected:
      RefItem(Attribute *owningAttribute, int itemPosition);
      RefItem(Item *owningItem, int myPosition, int mySubGroupPosition);
      virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr def);
      void clearAllReferences();
      std::vector<attribute::WeakAttributePtr>m_values;
    private:
    };
  }
}


#endif /* __smtk_attribute_RefItem_h */
