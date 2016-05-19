//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtAttribute - a class that encapsulates the UI of an Attribute
// .SECTION Description

#ifndef __smtk_extension_qtAttribute_h
#define __smtk_extension_qtAttribute_h

#include <QObject>
#include <QPointer>
#include <QWidget>
#include "smtk/extension/qt/Exports.h"
#include "smtk/PublicPointerDefs.h"

class qtAttributeInternals;
class QWidget;

namespace smtk {
  namespace extension {

  class qtAttributeItemWidgetFactory;
  class qtBaseView;
  class qtItem;

  class SMTKQTEXT_EXPORT qtAttribute : public QObject
    {
      Q_OBJECT

    public:
      qtAttribute(smtk::attribute::AttributePtr, QWidget* parent, qtBaseView* view);
      virtual ~qtAttribute();

      smtk::attribute::AttributePtr attribute();
      QWidget* widget()
      {return this->m_widget;}
      QWidget* parentWidget();

      virtual void addItem(qtItem*);
      QList<qtItem*>& items() const;
      virtual void showAdvanceLevelOverlay(bool show);

      // A basic layout for an attribute
      void createBasicLayout(bool includeAssociations);
      
      // create all the items
      static qtItem* createItem(smtk::attribute::ItemPtr item, QWidget* p, qtBaseView* view,
        Qt::Orientation enVectorItemOrient = Qt::Horizontal);

      static void setItemWidgetFactory(qtAttributeItemWidgetFactory* f);
      static qtAttributeItemWidgetFactory* itemWidgetFactory();

    public slots:
      virtual void onRequestEntityAssociation();

    protected slots:
      
    protected:
      virtual void createWidget();

      QPointer<QWidget> m_widget;
      static qtAttributeItemWidgetFactory* s_factory;

    private:
      qtAttributeInternals* m_internals;
    };

  } // namespace attribute
} // namespace smtk

#endif
