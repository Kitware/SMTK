//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtAttributeView - the Attribute Section
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtk_attribute_qtAttributeView_h
#define __smtk_attribute_qtAttributeView_h

#include "smtk/extension/qt/qtBaseView.h"

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
      virtual void childrenResized();
      virtual void showAdvanceLevelOverlay(bool show);

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
