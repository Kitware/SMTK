/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME qtInputsItem - an analysis type item
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef __slctk_attribute_qtInputsItem_h
#define __slctk_attribute_qtInputsItem_h

#include "qtItem.h"

class qtInputsItemInternals;
class QBoxLayout;

namespace slctk
{
  namespace attribute
  {
    class SLCTKATTRIBUTE_EXPORT qtInputsItem : public qtItem
    {
      Q_OBJECT

    public:         
      qtInputsItem(slctk::AttributeItemPtr, QWidget* p);
      virtual ~qtInputsItem();  

    public slots:
      void setOutputOptional(int);

    protected slots:
      virtual void updateItemData() {this->updateUI();}

    protected:
      virtual void createWidget();
      virtual void loadInputValues(
        QBoxLayout* labelLayout, QBoxLayout* entryLayout);
      virtual void updateUI();
    private:

      qtInputsItemInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace slctk

#endif
