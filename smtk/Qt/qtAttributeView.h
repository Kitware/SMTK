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
// .NAME qtAttributeView - the Attribute Section
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtk_attribute_qtAttributeView_h
#define __smtk_attribute_qtAttributeView_h

#include "smtk/Qt/qtBaseView.h"

#include <QMap>
#include <QStyledItemDelegate>
#include <QComboBox>

class qtAttributeViewInternals;
class QTableWidgetItem;
class QKeyEvent;
class QStandardItem;
class QTableWidget;

namespace smtk
{
  namespace attribute
  {
    class QTSMTK_EXPORT qtAttributeView : public qtBaseView
    {
      Q_OBJECT

    public:
      qtAttributeView(smtk::view::BasePtr, QWidget* p, qtUIManager* uiman);
      virtual ~qtAttributeView();
      const QMap<QString, QList<smtk::attribute::DefinitionPtr> > &attDefinitionMap() const;

      QTableWidgetItem* getSelectedItem();
      int currentViewBy();
      virtual void createNewAttribute(smtk::attribute::DefinitionPtr attDef);

      enum enumViewBy
        {
        VIEWBY_Attribute = 0,
        VIEWBY_PROPERTY
        };
    public slots:
      void onViewBy(int);
      void onViewByWithDefinition(
        int viewBy, smtk::attribute::DefinitionPtr attDef);
      void onShowCategory();
      void onListBoxSelectionChanged();
      void onAttributeValueChanged(QTableWidgetItem*);
      void onAttributeNameChanged(QTableWidgetItem*);
      void onCreateNew();
      void onCopySelected();
      void onDeleteSelected();
      void updateAssociationEnableState(smtk::attribute::AttributePtr);
      virtual void updateModelAssociation();
      void onListBoxClicked(QTableWidgetItem* item);
      void onAttributeCellChanged(int, int);
      void onPropertyDefSelected();
      void attributeFilterChanged(
        const QModelIndex& topLeft, const QModelIndex& bottomRight);
      void propertyFilterChanged(
        const QModelIndex& topLeft, const QModelIndex& bottomRight);

    signals:
      void numOfAttriubtesChanged();
      void attColorChanged();
      void attAssociationChanged();

    protected:
      virtual void createWidget( );
      smtk::attribute::AttributePtr getAttributeFromItem(QTableWidgetItem * item);
      smtk::attribute::ItemPtr getAttributeItemFromItem(QTableWidgetItem * item);

      smtk::attribute::AttributePtr getSelectedAttribute();
      QTableWidgetItem* addAttributeListItem(smtk::attribute::AttributePtr childData);
      void updateTableWithAttribute(smtk::attribute::AttributePtr dataItem);
      void addComparativeProperty(QStandardItem* current,
        smtk::attribute::DefinitionPtr attDef);

      void updateChildWidgetsEnableState(
        smtk::attribute::ItemPtr linkedData, QTableWidgetItem* item);
      void updateItemWidgetsEnableState(
        smtk::attribute::ItemPtr linkedData, int &startRow, bool enabled);
      virtual void getAllDefinitions();

      virtual void updateTableWithProperties();
      virtual void removeComparativeProperty(const QString& propertyName);
      void initSelectionFilters();
      void initSelectAttCombo(smtk::attribute::DefinitionPtr attDef);
      void initSelectPropCombo(smtk::attribute::DefinitionPtr attDef);
      void addComparativeAttribute(smtk::attribute::AttributePtr att);
      void removeComparativeAttribute(smtk::attribute::AttributePtr att);
      void insertTableColumn(QTableWidget* wTable, int insertCol,
        const QString& title, int advancedlevel);

    private:

      qtAttributeViewInternals *Internals;

    }; // class

    //A sublcass of QTextEdit to give initial sizehint
    class QTSMTK_EXPORT qtCheckableComboItemDelegate : public QStyledItemDelegate
      {
      Q_OBJECT
      public:
        qtCheckableComboItemDelegate(QWidget * owner);
        virtual void paint(
          QPainter* painter,
          const QStyleOptionViewItem& option,
          const QModelIndex& index) const;
      };

    //A sublcass of QComboBox to set text when hidePopup
    class QTSMTK_EXPORT qtAttSelectCombo : public QComboBox
      {
      Q_OBJECT
      public:
        qtAttSelectCombo(QWidget * parentW, const QString& displayExt);
        virtual void hidePopup();
        virtual void init();
        virtual void updateText();

      private:
        QStandardItem* m_displayItem;
        QString m_displayTextExt;
      };

  }; // namespace attribute
}; // namespace smtk


#endif
