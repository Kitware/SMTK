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
// .NAME qtAssociationWidget - the Attribute Section
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtk_attribute_qtAssociationWidget_h
#define __smtk_attribute_qtAssociationWidget_h

#include <QWidget>
#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"

class qtAssociationWidgetInternals;
class QListWidgetItem;
class QListWidget;
class ModelEntity;

namespace smtk
{
  namespace attribute
  {
    class SMTKCORE_EXPORT qtAssociationWidget : public QWidget
    {
      Q_OBJECT

    public:
      qtAssociationWidget(QWidget* p);
      virtual ~qtAssociationWidget();

    public slots:
      virtual void showAdvanced(int show);
      virtual void showEntityAssociation(smtk::ModelItemPtr entity, QString& category);
      void onCurrentListSelectionChanged(QListWidgetItem * , QListWidgetItem * );
      void onAvailableListSelectionChanged(QListWidgetItem * , QListWidgetItem * );

    protected slots:
      virtual void onRemoveAssigned();
      virtual void onAddAvailable();
      virtual void onExchange();

    protected:
      virtual void initWidget( );
      QListWidgetItem* getSelectedItem(QListWidget* theLis);
      smtk::AttributePtr getSelectedAttribute(QListWidget* theLis);
      smtk::AttributePtr getAttributeFromItem(QListWidgetItem * item);
      virtual QListWidgetItem* addAttributeRefListItem(QListWidget* theList,
        smtk::AttributeItemPtr refItem);

    private:

      qtAssociationWidgetInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk


#endif
