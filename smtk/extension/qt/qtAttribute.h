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

#ifndef __smtk_attribute_qtAttribute_h
#define __smtk_attribute_qtAttribute_h

#include <QObject>
#include "smtk/extension/qt/QtSMTKExports.h"
#include "smtk/PublicPointerDefs.h"

class qtAttributeInternals;
class QWidget;

namespace smtk
{
  namespace attribute
  {
  class qtItem;
  class qtBaseView;
    class QTSMTK_EXPORT qtAttribute : public QObject
    {
      Q_OBJECT

    public:
      qtAttribute(smtk::attribute::AttributePtr, QWidget* parent, qtBaseView* view);
      virtual ~qtAttribute();

      smtk::attribute::AttributePtr getObject();
      QWidget* widget()
      {return this->Widget;}
      QWidget* parentWidget();

      virtual void addItem(qtItem*);
      virtual void clearItems();
      QList<qtItem*>& items() const;
      virtual void showAdvanceLevelOverlay(bool show);

      // create all the items
      static qtItem* createItem(smtk::attribute::ItemPtr item, QWidget* p, qtBaseView* view,
        Qt::Orientation enVectorItemOrient = Qt::Horizontal);
      static qtItem* createValueItem(smtk::attribute::ValueItemPtr item, QWidget* p, qtBaseView* view,
        Qt::Orientation enVectorItemOrient = Qt::Horizontal);
      static qtItem* createDirectoryItem(smtk::attribute::DirectoryItemPtr item, QWidget* p, qtBaseView* view,
        Qt::Orientation enVectorItemOrient = Qt::Horizontal);
      static qtItem* createAttributeRefItem(smtk::attribute::RefItemPtr item, QWidget* p, qtBaseView* view,
        Qt::Orientation enVectorItemOrient = Qt::Horizontal);
      static qtItem* createFileItem(smtk::attribute::FileItemPtr item, QWidget* p, qtBaseView* view,
        Qt::Orientation enVectorItemOrient = Qt::Horizontal);
      static qtItem* createGroupItem(smtk::attribute::GroupItemPtr item, QWidget* p, qtBaseView* view,
        Qt::Orientation enVectorItemOrient = Qt::Horizontal);
      static qtItem* createDiscreteValueItem(smtk::attribute::ValueItemPtr item, QWidget* p, qtBaseView* view,
        Qt::Orientation enVectorItemOrient = Qt::Horizontal);
      static qtItem* createModelEntityItem(smtk::attribute::ModelEntityItemPtr item, QWidget* pW, qtBaseView* view,
        Qt::Orientation enVectorItemOrient = Qt::Horizontal);
      static qtItem* createMeshSelectionItem(smtk::attribute::MeshSelectionItemPtr item, QWidget* pW, qtBaseView* view,
        Qt::Orientation enVectorItemOrient = Qt::Horizontal);

    protected slots:
      virtual void updateItemsData();

    protected:
      virtual void createWidget();

      QWidget* Widget;
    private:

      qtAttributeInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk

#endif
