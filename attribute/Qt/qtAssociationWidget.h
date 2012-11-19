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

#ifndef __slctk_attribute_qtAssociationWidget_h
#define __slctk_attribute_qtAssociationWidget_h

#include <QWidget>
#include "AttributeExports.h"
#include "attribute/PublicPointerDefs.h"

class qtAssociationWidgetInternals;
class QListWidgetItem;
class QListWidget;
class ModelEntity;

namespace slctk
{
  namespace attribute
  {
    class SLCTKATTRIBUTE_EXPORT qtAssociationWidget : public QWidget
    {
      Q_OBJECT

    public:
      qtAssociationWidget(QWidget* p);
      virtual ~qtAssociationWidget();

    public slots:
      void showAdvanced(int show);
      void showAttributeAssociation(slctk::AttributePtr att, QString& category);
      void showEntityAssociation(ModelEntity*, QString& category){;}
      void onCurrentListSelectionChanged(QListWidgetItem * , QListWidgetItem * );
      void onAvailableListSelectionChanged(QListWidgetItem * , QListWidgetItem * );

    protected slots:
      void onRemoveAssigned();
      void onAddAvailable();
      void onExchange();

    protected:
      virtual void initWidget( );
      QListWidgetItem* getSelectedItem(QListWidget* theLis);
      slctk::AttributePtr getSelectedAttribute(QListWidget* theLis);
      slctk::AttributePtr getAttributeFromItem(QListWidgetItem * item);
      QListWidgetItem* addAttributeListItem(QListWidget* theList,
        slctk::AttributePtr childData);

    private:

      qtAssociationWidgetInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace slctk


#endif
