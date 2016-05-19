//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtAssociationWidget - the Attribute-Model association widget
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtk_extension_qtAssociationWidget_h
#define __smtk_extension_qtAssociationWidget_h

#include <QWidget>
#include "smtk/extension/qt/Exports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"

#include <set>

class qtAssociationWidgetInternals;
class QListWidgetItem;
class QListWidget;

namespace smtk
{
  namespace extension
  {
    class qtBaseView;
    class SMTKQTEXT_EXPORT qtAssociationWidget : public QWidget
    {
      Q_OBJECT

    public:
      qtAssociationWidget(QWidget* p, qtBaseView* view);
      virtual ~qtAssociationWidget();

    public slots:
      virtual void showEntityAssociation(smtk::attribute::AttributePtr theAtt);
      virtual void showAttributeAssociation(smtk::model::EntityRef theEntiy,
                                            std::vector<smtk::attribute::DefinitionPtr>& attDefs);
      virtual void showDomainsAssociation(
        std::vector<smtk::model::Group>& theDomains,
        std::vector<smtk::attribute::DefinitionPtr>& attDefs);

    signals:
      void attAssociationChanged();

    protected slots:
      virtual void onRemoveAssigned();
      virtual void onAddAvailable();
      virtual void onExchange();
      virtual void onNodalOptionChanged(int);
      virtual void onDomainAssociationChanged();
      virtual void onEntitySelected(QListWidgetItem * , QListWidgetItem * );

    protected:
      virtual void initWidget( );
      QListWidgetItem* getSelectedItem(QListWidget* theLis);
      virtual void removeSelectedItem(QListWidget* theLis);
      smtk::attribute::AttributePtr getAttribute(QListWidgetItem * item);
      smtk::attribute::AttributePtr getSelectedAttribute(QListWidget* theLis);

      smtk::model::EntityRef getModelEntityItem(QListWidgetItem * item);
      smtk::model::EntityRef getSelectedModelEntityItem(QListWidget* theLis);

      //returns the Item it has added to the widget
      //ownership of the item is handled by the widget so no need to delete
      virtual QListWidgetItem* addModelAssociationListItem(
           QListWidget* theList, smtk::model::EntityRef modelItem, bool sort=true);

      //returns the Item it has added to the widget
      //ownership of the item is handled by the widget so no need to delete
      virtual QListWidgetItem* addAttributeAssociationItem(
        QListWidget* theList, smtk::attribute::AttributePtr att, bool sort=true);


      virtual void addDomainListItem( const smtk::model::Group& domainItem,
                                      QList<smtk::attribute::AttributePtr>& allAtts);

      std::set<smtk::model::EntityRef> processAttUniqueness(smtk::attribute::DefinitionPtr attDef,
                                                  const smtk::model::EntityRefs &assignedIds);

      QList<smtk::attribute::DefinitionPtr> processDefUniqueness(const smtk::model::EntityRef& theEntity,
                                                                 smtk::attribute::System* attSystem);

    private:
      qtAssociationWidgetInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk


#endif
