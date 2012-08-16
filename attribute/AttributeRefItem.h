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

#ifndef __slctk_attribute_AttributeRefItem_h
#define __slctk_attribute_AttributeRefItem_h

#include "AttributeExports.h"
#include "attribute/PublicPointerDefs.h"
#include "attribute/Item.h"
#include <vector>

namespace slctk
{
  namespace attribute
  {
    class Attribute;
    class AttributeRefItemDefinition;
    class SLCTKATTRIBUTE_EXPORT AttributeRefItem : public Item
    {
    public:
      AttributeRefItem();
      virtual ~AttributeRefItem();
      virtual Item::Type type() const;
      virtual bool setDefinition(slctk::ConstAttributeItemDefinitionPtr def);
      slctk::AttributePtr value(int element=0) const
      {return this->m_values[element].lock();}
      bool setValue( slctk::AttributePtr val)
      {return this->setValue(0, val);}
      bool setValue(int element, slctk::AttributePtr val);
      bool appendValue(slctk::AttributePtr val);
      bool removeValue(int element);
      virtual void reset();
      virtual std::string valueAsString(const std::string &format="") const
      {return this->valueAsString(0, format);}
      virtual std::string valueAsString(int element, const std::string &format="") const;
      virtual bool isSet(int element=0) const
      {return this->m_values[element].lock() != NULL;}
      virtual void unset(int element=0)
      {this->m_values[element].reset();}
     
    protected:
      std::vector<WeakAttributePtr>m_values;
    private:
    };
  };
};


#endif /* __slctk_attribute_AttributeRefItem_h */
