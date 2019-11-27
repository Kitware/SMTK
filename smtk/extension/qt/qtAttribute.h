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
#include "smtk/extension/qt/qtItem.h"
#include <QObject>
#include <QPointer>
#include <QWidget>

class qtAttributeInternals;
class QWidget;

namespace smtk
{
namespace extension
{

class qtBaseView;
class qtItem;

class SMTKQTEXT_EXPORT qtAttribute : public QObject
{
  Q_OBJECT

public:
  qtAttribute(smtk::attribute::AttributePtr, const smtk::view::Configuration::Component& comp,
    QWidget* parent, qtBaseView* view);
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

  //Returns true if it does not display any of its items
  bool isEmpty() const;

signals:
  // Signal indicates that the underlying item has been modified
  void modified();
  void itemModified(qtItem*);

protected:
  virtual void createWidget();

  QPointer<QWidget> m_widget;

protected slots:
  void onItemModified();

private:
  qtAttributeInternals* m_internals;
  bool m_useSelectionManager;
  bool m_isEmpty;
};

} // namespace attribute
} // namespace smtk

#endif
