//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtItem_h
#define smtk_extension_qtItem_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtAttributeItemInfo.h"
#include "smtk/view/Configuration.h"
#include <QObject>
#include <QPointer>

// VTK's wrapper parser does not properly handle Qt macros on macos.
#if defined(__VTK_WRAP__) && !defined(Q_SLOTS)
#define Q_DISABLE_COPY(x)
#define Q_SLOTS
#define Q_SIGNALS protected
#define Q_OBJECT
#endif

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

/// an abstract UI class for attribute item
class SMTKQTEXT_EXPORT qtItem : public QObject
{
  Q_OBJECT

public:
  qtItem(const qtAttributeItemInfo& info);
  ~qtItem() override;

  smtk::attribute::ItemPtr item() const { return m_itemInfo.item(); }

  template<typename ItemType>
  std::shared_ptr<ItemType> itemAs() const
  {
    return m_itemInfo.itemAs<ItemType>();
  }

  qtUIManager* uiManager() const { return m_itemInfo.uiManager(); }

  /// Return the underlying Attribute Resource
  smtk::attribute::ResourcePtr attributeResource() const;

  QPointer<QWidget> widget() { return m_widget; }
  QPointer<QWidget> parentWidget() { return m_itemInfo.parentWidget(); }

  bool isLeafItem() { return m_isLeafItem; }

  virtual void setLabelVisible(bool) { ; }
  virtual bool isFixedWidth() const;

  void showAdvanceLevelOverlay(bool);
  bool useSelectionManager() const { return m_useSelectionManager; }
  void setReadOnly(bool mode) { m_readOnly = mode; }
  bool isReadOnly() const;
  ///\brief Indicates that the item should be deleted.  This is similar to Qt's
  /// deleteLater() method (in fact it calls it); however, it also allows the qtItem to do some
  /// cleanup such as stop observing SMTK "signals".
  virtual void markForDeletion();

  /** \brief Returns editor widget, used when setting tab order */
  virtual QWidget* lastEditor() const { return nullptr; }
  /** \brief Sets previous widget for tabbing order */
  virtual void updateTabOrder(QWidget* /*precedingEditor*/) {}

public Q_SLOTS:
  // Controls whether the Selection Manager should be used for setting model
  // and mesh entity items - Note that this is just a hint and could be ignored
  // due to other criteria
  // virtual void setUseSelectionManager(bool mode) { m_useSelectionManager = mode; }

  /// Tell the qtItem to update itself based on changes to its underlying
  /// attribute item
  virtual void updateItemData();

Q_SIGNALS:
  /// Signal indicates that the underlying widget's size has been modified
  void widgetSizeChanged();
  /// Signal indicates that the underlying item (or one that it owns) has been modified
  void modified(qtItem* item);

  /** \brief Indicates editing widget changed
  *
  * In some cases, parent item should update tab order.
  */
  void editingWidgetChanged();

protected Q_SLOTS:
  virtual void onAdvanceLevelChanged(int levelIdx);
  virtual void onChildWidgetSizeChanged() { ; }

protected:
  virtual void createWidget() { ; }
  virtual void setLocalAdvanceLevel(unsigned int level);
  virtual void addChildItem(qtItem*);
  virtual void removeChildItem(qtItem*);
  virtual void clearChildItems();
  QList<qtItem*>& childItems();

  QPointer<QWidget> m_widget;
  bool m_isLeafItem;
  bool m_useSelectionManager;
  bool m_readOnly;
  bool m_markedForDeletion;
  qtAttributeItemInfo m_itemInfo;
  QList<smtk::extension::qtItem*> m_childItems;

private:
  qtItemInternals* Internals;
};
} // namespace extension
} // namespace smtk

#endif
