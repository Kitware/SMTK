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
// .NAME qtAttributeRefItem - an Expression item
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef __smtk_attribute_qtAttributeRefItem_h
#define __smtk_attribute_qtAttributeRefItem_h

#include "smtk/Qt/qtItem.h"

class qtAttributeRefItemInternals;

namespace smtk
{
  namespace attribute
  {
    class QTSMTK_EXPORT qtAttributeRefItem : public qtItem
    {
      Q_OBJECT

    public:
      qtAttributeRefItem(smtk::AttributeItemPtr,
        QWidget* parent);
      virtual ~qtAttributeRefItem();
      QString labelText() const;
      void setLabelVisible(bool);

    public slots:
      void onInputValueChanged();

    protected slots:
      virtual void updateItemData();

    protected:
      virtual void createWidget();

    private:

      qtAttributeRefItemInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk


#endif
