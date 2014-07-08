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
// .NAME DoubleItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_DoubleItemDefinition_h
#define __smtk_attribute_DoubleItemDefinition_h

#include "smtk/attribute/ValueItemDefinitionTemplate.h"

namespace smtk
{
  namespace attribute
  {
    class SMTKCORE_EXPORT DoubleItemDefinition :
      public ValueItemDefinitionTemplate<double>
    {
    public:
      smtkTypeMacro(DoubleItemDefinition);
      static smtk::attribute::DoubleItemDefinitionPtr New(const std::string &myName)
      { return smtk::attribute::DoubleItemDefinitionPtr(new DoubleItemDefinition(myName));}

      virtual ~DoubleItemDefinition();
      virtual Item::Type type() const;
      virtual smtk::attribute::ItemPtr buildItem(Attribute *owningAttribute,
                                                int itemPosition) const;
      virtual smtk::attribute::ItemPtr buildItem(Item *owningItem,
                                                int position,
                                                int subGroupPosition) const;

      virtual smtk::attribute::ItemDefinitionPtr
        createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const;
    protected:
      DoubleItemDefinition(const std::string &myName);

    private:

    };
  }
}

#endif /* __smtk_attribute_DoubleItemDefinition_h */
