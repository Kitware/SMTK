//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtReferencesWidget - the Attribute References Widget
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtk_attribute_qtReferencesWidget_h
#define __smtk_attribute_qtReferencesWidget_h

#include <QWidget>
#include "smtk/extension/qt/QtSMTKExports.h"
#include "smtk/PublicPointerDefs.h"

class qtReferencesWidgetInternals;
class QListWidgetItem;
class QListWidget;
class Model;

namespace smtk
{
  namespace attribute
  {
    class QTSMTK_EXPORT qtReferencesWidget : public QWidget
    {
      Q_OBJECT

    public:
      qtReferencesWidget(QWidget* p);
      virtual ~qtReferencesWidget();

    public slots:
      virtual void showAttributeReferences(smtk::attribute::AttributePtr att, QString& category);
      void onCurrentListSelectionChanged(QListWidgetItem * , QListWidgetItem * );
      void onAvailableListSelectionChanged(QListWidgetItem * , QListWidgetItem * );

    protected slots:
      virtual void onRemoveAssigned();
      virtual void onAddAvailable();
      virtual void onExchange();

    protected:
      virtual void initWidget( );
      QListWidgetItem* getSelectedItem(QListWidget* theLis);
      smtk::attribute::AttributePtr getSelectedAttribute(QListWidget* theLis);
      smtk::attribute::AttributePtr getAttributeFromItem(QListWidgetItem * item);
      virtual QListWidgetItem* addAttributeRefListItem(QListWidget* theList,
        smtk::attribute::ItemPtr refItem);

    private:

      qtReferencesWidgetInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk


#endif
