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

#ifndef __slctk_model_GroupItem_h
#define __slctk_model_GroupItem_h

#include "ModelExports.h"
#include "PublicPointerDefs.h"
#include "model/Item.h"
#include <string>

namespace slctk
{
  namespace model
  {
    class SLCTKMODEL_EXPORT GroupItem : public Item
    {
      GroupItem(Model *model, int myid, unsigned int mask);
      virtual ~GroupItem();
      virtual Item::Type type() const;
      unsigned long entityMask() const
      { return this->m_entityMask;}
      bool canContain(const slctk::ModelItemPtr &ptr) const
      {return ((this->m_entityMask & ptr->Type()) == ptr->Type());}
      std::size_t numberOfItems() const = 0;
      virtual slctk::ModelItemPtr item(int i) const = 0;
      virtual bool insert(slctk::ModelItemPtr &ptr) = 0;
      virtual bool remove(slctk::ModelItemPtr &ptr) = 0;
    protected:
      unsigned int m_entityMask;
    private:
    };
  };
};


#endif /* __slctk_model_GroupItem_h */
