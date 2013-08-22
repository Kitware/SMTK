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
// .NAME GroupItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_model_GroupItem_h
#define __smtk_model_GroupItem_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/model/Item.h"
#include <string>

namespace smtk
{
  namespace model
  {
    class SMTKCORE_EXPORT GroupItem : public Item
    {
    public:
      GroupItem(Model *model, int myid, unsigned long mask);
      virtual ~GroupItem();
      virtual Item::Type type() const;
      unsigned long entityMask() const
      { return this->m_entityMask;}
      bool canContain(const smtk::ModelItemPtr ptr) const
      {return this->canContain(ptr->type());}
      bool canContain(smtk::model::Item::Type enType) const
      {return ((this->m_entityMask & enType) != 0);}
      virtual std::size_t numberOfItems() const = 0;
      virtual smtk::ModelItemPtr item(int i) const = 0;
      virtual bool insert(smtk::ModelItemPtr &ptr) = 0;
      virtual bool remove(smtk::ModelItemPtr &ptr) = 0;

    protected:
      unsigned long m_entityMask;

    private:
    };
  };
};

#endif /* __smtk_model_GroupItem_h */
