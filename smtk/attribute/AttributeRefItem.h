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
// .NAME AttributeRefItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_AttributeRefItem_h
#define __smtk_attribute_AttributeRefItem_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h"
#include <vector>

namespace smtk
{
  namespace attribute
  {
    class Attribute;
    class AttributeRefItemDefinition;
    class SMTKCORE_EXPORT AttributeRefItem : public Item
    {
      friend class Attribute;
    public:
      AttributeRefItem(Attribute *owningAttribute, int itemPosition);
      AttributeRefItem(Item *owningItem, int myPosition, int mySubGroupPosition);
      virtual ~AttributeRefItem();
      virtual Item::Type type() const;
      virtual bool setDefinition(smtk::ConstAttributeItemDefinitionPtr def);
      std::size_t numberOfValues() const
      {return this->m_values.size();}
      bool  setNumberOfValues(std::size_t newSize);
      int numberOfRequiredValues() const;
      smtk::AttributePtr value(int element=0) const
      {return this->m_values[element].lock();}
      bool setValue( smtk::AttributePtr val)
      {return this->setValue(0, val);}
      bool setValue(int element, smtk::AttributePtr val);
      bool appendValue(smtk::AttributePtr val);
      bool removeValue(int element);
      virtual void reset();
      virtual std::string valueAsString(const std::string &format="") const
      {return this->valueAsString(0, format);}
      virtual std::string valueAsString(int element, const std::string &format="") const;
      virtual bool isSet(int element=0) const
      {return this->m_values[element].lock().get() != NULL;}
      virtual void unset(int element=0);

    protected:
      void clearAllReferences();
      std::vector<WeakAttributePtr>m_values;
    private:
    };
  }
}


#endif /* __smtk_attribute_AttributeRefItem_h */
