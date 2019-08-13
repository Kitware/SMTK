//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtItem - an abstract UI class for attribute item
// .SECTION Description

#ifndef __smtk_extension_qtItem_h
#define __smtk_extension_qtItem_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtAttributeItemInfo.h"
#include "smtk/view/View.h"
#include <QObject>
#include <QPointer>

class qtItemInternals;

namespace smtk
{
namespace extension
{
class qtUIManager;

/* Define a casting macro for use by the constants below.  */
#if defined(__cplusplus)
#define smtk_TYPE_CAST(T, V) static_cast<T>(V)
#else
#define smtk_TYPE_CAST(T, V) ((T)(V))
#endif

#define smtk_INT_MIN smtk_TYPE_CAST(int, ~(~0u >> 1))
#define smtk_INT_MAX smtk_TYPE_CAST(int, ~0u >> 1)
#define smtk_UNSIGNED_INT_MIN smtk_TYPE_CAST(unsigned int, 0)
#define smtk_UNSIGNED_INT_MAX smtk_TYPE_CAST(unsigned int, ~0u)
#define smtk_LONG_MIN smtk_TYPE_CAST(long, ~(~0ul >> 1))
#define smtk_LONG_MAX smtk_TYPE_CAST(long, ~0ul >> 1)
#define smtk_UNSIGNED_LONG_MIN smtk_TYPE_CAST(unsigned long, 0ul)
#define smtk_UNSIGNED_LONG_MAX smtk_TYPE_CAST(unsigned long, ~0ul)
#define smtk_FLOAT_MIN smtk_TYPE_CAST(float, -1.0e+38f)
#define smtk_FLOAT_MAX smtk_TYPE_CAST(float, 1.0e+38f)
#define smtk_DOUBLE_MIN smtk_TYPE_CAST(double, -1.0e+299)
#define smtk_DOUBLE_MAX smtk_TYPE_CAST(double, 1.0e+299)

#define smtk_DOUBLE_CONSTRAINT_PRECISION 0.000001
#define smtk_USER_DATA_TYPE 10000

class SMTKQTEXT_EXPORT qtItem : public QObject
{
  Q_OBJECT

public:
  qtItem(const qtAttributeItemInfo& info);
  virtual ~qtItem();

  smtk::attribute::ItemPtr item() const { return m_itemInfo.item(); }

  template <typename ItemType>
  std::shared_ptr<ItemType> itemAs() const
  {
    return m_itemInfo.itemAs<ItemType>();
  }

  qtUIManager* uiManager() const { return m_itemInfo.uiManager(); }

  QPointer<QWidget> widget() { return m_widget; }
  QPointer<QWidget> parentWidget() { return m_itemInfo.parentWidget(); }

  bool isLeafItem() { return m_isLeafItem; }

  virtual void setLabelVisible(bool) { ; }

  void showAdvanceLevelOverlay(bool);
  bool useSelectionManager() const { return m_useSelectionManager; }
  void setReadOnly(bool mode) { m_readOnly = mode; }
  bool isReadOnly() const { return m_readOnly; }
  ///\brief Indicates that the item should be deleted.  This is similar to Qt's
  /// deleteLater() method (in fact it calls it); however, it also allows the qtItem to do some
  /// cleanup such as stop observing SMTK "signals".
  virtual void markForDeletion();

public slots:
  // Controls whether the Selection Manager should be used for setting model
  // and mesh entity items - Note that this is just a hint and could be ignored
  // due to other criteria
  // virtual void setUseSelectionManager(bool mode) { m_useSelectionManager = mode; }

signals:
  /// /brief Signal indicates that the underlying widget's size has been modified
  void widgetSizeChanged();
  // Signal indicates that the underlying item has been modified
  void modified();

protected slots:
  virtual void updateItemData();
  virtual void onAdvanceLevelChanged(int levelIdx);
  virtual void onChildWidgetSizeChanged() { ; }

protected:
  virtual void createWidget() { ; }
  virtual void setAdvanceLevel(int level);
  virtual void addChildItem(qtItem*);
  virtual void removeChildItem(qtItem*);
  virtual void clearChildItems();
  QList<qtItem*>& childItems();

  QPointer<QWidget> m_widget;
  bool m_isLeafItem;
  bool m_useSelectionManager;
  bool m_readOnly;
  qtAttributeItemInfo m_itemInfo;
  QList<smtk::extension::qtItem*> m_childItems;

private:
  qtItemInternals* Internals;
}; // class
}; // namespace extension
}; // namespace smtk

#endif
