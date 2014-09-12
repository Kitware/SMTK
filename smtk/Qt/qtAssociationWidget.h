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
// .NAME qtAssociationWidget - the Attribute-Model association widget
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtk_attribute_qtAssociationWidget_h
#define __smtk_attribute_qtAssociationWidget_h

#include <QWidget>
#include "smtk/QtSMTKExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/model/ModelEntity.h"
#include "smtk/model/GroupEntity.h"

#include <set>

class qtAssociationWidgetInternals;
class QListWidgetItem;
class QListWidget;

namespace smtk
{
  namespace attribute
  {
    class qtBaseView;
    class QTSMTK_EXPORT qtAssociationWidget : public QWidget
    {
      Q_OBJECT

    public:
      qtAssociationWidget(QWidget* p, qtBaseView* view);
      virtual ~qtAssociationWidget();

    public slots:
      virtual void showEntityAssociation(smtk::attribute::AttributePtr theAtt);
      virtual void showAttributeAssociation(smtk::model::ModelEntity theEntiy,
                                            std::vector<smtk::attribute::DefinitionPtr>& attDefs);
      virtual void showDomainsAssociation(
        std::vector<smtk::model::GroupEntity>& theDomains,
        std::vector<smtk::attribute::DefinitionPtr>& attDefs);
      void onCurrentListSelectionChanged(QListWidgetItem * , QListWidgetItem * );
      void onAvailableListSelectionChanged(QListWidgetItem * , QListWidgetItem * );

    signals:
      void attAssociationChanged();

    protected slots:
      virtual void onRemoveAssigned();
      virtual void onAddAvailable();
      virtual void onExchange();
      virtual void onNodalOptionChanged(int);
      virtual void onDomainAssociationChanged();

    protected:
      virtual void initWidget( );
      QListWidgetItem* getSelectedItem(QListWidget* theLis);
      virtual void removeSelectedItem(QListWidget* theLis);
      smtk::attribute::AttributePtr getAttribute(QListWidgetItem * item);
      smtk::attribute::AttributePtr getSelectedAttribute(QListWidget* theLis);

      smtk::model::ModelEntity getModelItem(QListWidgetItem * item);
      smtk::model::ModelEntity getSelectedModelItem(QListWidget* theLis);

      //returns the Item it has added to the widget
      //ownership of the item is handled by the widget so no need to delete
      virtual QListWidgetItem* addModelAssociationListItem(
           QListWidget* theList, smtk::model::ModelEntity modelItem);

      //returns the Item it has added to the widget
      //ownership of the item is handled by the widget so no need to delete
      virtual QListWidgetItem* addAttributeAssociationItem(
        QListWidget* theList, smtk::attribute::AttributePtr att);


      virtual void addDomainListItem( const smtk::model::GroupEntity& domainItem,
                                      QList<smtk::attribute::AttributePtr>& allAtts);

      std::set<smtk::model::ModelEntity> processAttUniqueness(smtk::attribute::DefinitionPtr attDef,
                                                  const smtk::model::ModelEntities &assignedIds);

      QList<smtk::attribute::DefinitionPtr> processDefUniqueness(const smtk::model::ModelEntity& theEntity,
                                                                 smtk::attribute::Manager* attManager);

    private:
      qtAssociationWidgetInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk


#endif
