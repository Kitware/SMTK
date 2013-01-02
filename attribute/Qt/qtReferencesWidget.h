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
// .NAME qtReferencesWidget - the Attribute References Widget
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __slctk_attribute_qtReferencesWidget_h
#define __slctk_attribute_qtReferencesWidget_h

#include <QWidget>
#include "AttributeExports.h"
#include "PublicPointerDefs.h"

class qtReferencesWidgetInternals;
class QListWidgetItem;
class QListWidget;
class ModelEntity;

namespace slctk
{
  namespace attribute
  {
    class SLCTKATTRIBUTE_EXPORT qtReferencesWidget : public QWidget
    {
      Q_OBJECT

    public:
      qtReferencesWidget(QWidget* p);
      virtual ~qtReferencesWidget();

    public slots:
      virtual void showAdvanced(int show);
      virtual void showAttributeReferences(slctk::AttributePtr att, QString& category);
      void onCurrentListSelectionChanged(QListWidgetItem * , QListWidgetItem * );
      void onAvailableListSelectionChanged(QListWidgetItem * , QListWidgetItem * );

    protected slots:
      virtual void onRemoveAssigned();
      virtual void onAddAvailable();
      virtual void onExchange();

    protected:
      virtual void initWidget( );
      QListWidgetItem* getSelectedItem(QListWidget* theLis);
      slctk::AttributePtr getSelectedAttribute(QListWidget* theLis);
      slctk::AttributePtr getAttributeFromItem(QListWidgetItem * item);
      virtual QListWidgetItem* addAttributeRefListItem(QListWidget* theList,
        slctk::AttributeItemPtr refItem);

    private:

      qtReferencesWidgetInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace slctk


#endif
