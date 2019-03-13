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
#include "smtk/view/View.h"
#include <QObject>

class qtItemInternals;

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

namespace smtk
{
namespace extension
{
class qtBaseView;
class qtUIManager;

// This struct is used to construct qtItem instances using factory methods
class SMTKQTEXT_EXPORT AttributeItemInfo
{
public:
  AttributeItemInfo(smtk::attribute::ItemPtr item, smtk::view::View::Component itemComp,
    QWidget* parent, qtBaseView* bview)
    : m_item(item)
    , m_component(itemComp)
    , m_parentWidget(parent)
    , m_baseView(bview)
  {
  }

  virtual ~AttributeItemInfo() {}
  smtk::attribute::ItemPtr item() const { return m_item.lock(); }

  template <typename ItemType>
  std::shared_ptr<ItemType> itemAs() const
  {
    return std::dynamic_pointer_cast<ItemType>(this->item());
  }

  smtk::view::View::Component component() const { return m_component; }

  QWidget* parentWidget() const { return m_parentWidget; }

  qtBaseView* baseView() const { return m_baseView; }

  qtUIManager* uiManager() const;

protected:
  smtk::attribute::WeakItemPtr m_item;     // Pointer to the attribute Item
  smtk::view::View::Component m_component; // qtItem Component Definition
  QWidget* m_parentWidget;                 // Parent Widget of the qtItem
  qtBaseView* m_baseView;                  // View Definition
};

class SMTKQTEXT_EXPORT qtItem : public QObject
{
  Q_OBJECT

public:
  qtItem(const AttributeItemInfo& info);
  virtual ~qtItem();

  smtk::attribute::ItemPtr item() const { return m_itemInfo.item(); }

  template <typename ItemType>
  std::shared_ptr<ItemType> itemAs() const
  {
    return m_itemInfo.itemAs<ItemType>();
  }

  qtUIManager* uiManager() const { return m_itemInfo.uiManager(); }

  QWidget* widget() { return m_widget; }
  QWidget* parentWidget() { return m_itemInfo.parentWidget(); }

  virtual void addChildItem(qtItem*);
  virtual void clearChildItems();
  QList<qtItem*>& childItems();

  bool isLeafItem() { return m_isLeafItem; }

  virtual void setLabelVisible(bool) { ; }

  bool passAdvancedCheck();
  void showAdvanceLevelOverlay(bool);
  bool useSelectionManager() const { return m_useSelectionManager; }
  void setReadOnly(bool mode) { m_readOnly = mode; }
  bool isReadOnly() const { return m_readOnly; }

public slots:
  // Controls whether the Selection Manager should be used for setting model
  // and mesh entity items - Note that this is just a hint and could be ignored
  // due to other criteria
  // virtual void setUseSelectionManager(bool mode) { m_useSelectionManager = mode; }

signals:
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

  QWidget* m_widget;
  bool m_isLeafItem;
  bool m_useSelectionManager;
  bool m_readOnly;
  AttributeItemInfo m_itemInfo;
  QList<smtk::extension::qtItem*> m_childItems;

private:
  qtItemInternals* Internals;
}; // class
}; // namespace attribute
}; // namespace smtk

#endif
