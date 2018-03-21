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

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"
#include <QObject>
#include <QPointer>
#include <QWidget>

class qtAttributeInternals;
class QWidget;

namespace smtk
{
namespace extension
{

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
  QWidget* widget() { return m_widget; }
  QWidget* parentWidget();

  virtual void addItem(qtItem*);
  QList<qtItem*>& items() const;
  virtual void showAdvanceLevelOverlay(bool show);
  bool useSelectionManager() const { return m_useSelectionManager; }

  // A basic layout for an attribute
  void createBasicLayout(bool includeAssociations);

  // create all the items
  static qtItem* createItem(smtk::attribute::ItemPtr item, QWidget* p, qtBaseView* view,
    Qt::Orientation enVectorItemOrient = Qt::Horizontal);

  static void setItemWidgetFactory(qtAttributeItemWidgetFactory* f);
  static qtAttributeItemWidgetFactory* itemWidgetFactory();

signals:
  // Signal indicates that the underlying item has been modified
  void modified();
  void itemModified(qtItem*);

protected:
  virtual void createWidget();

  QPointer<QWidget> m_widget;
  static qtAttributeItemWidgetFactory* s_factory;

protected slots:
  void onItemModified();

private:
  qtAttributeInternals* m_internals;
  bool m_useSelectionManager;
};

} // namespace attribute
} // namespace smtk

#endif
