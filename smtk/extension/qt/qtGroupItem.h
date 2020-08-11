//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtGroupItem - UI components for attribute GroupItem
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef __smtk_extension_qtGroupItem_h
#define __smtk_extension_qtGroupItem_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtItem.h"

class qtGroupItemInternals;

namespace smtk
{
namespace extension
{
class SMTKQTEXT_EXPORT qtGroupItem : public qtItem
{
  Q_OBJECT

public:
  static qtItem* createItemWidget(const qtAttributeItemInfo& info);
  qtGroupItem(const qtAttributeItemInfo& info);
  virtual ~qtGroupItem();
  void setLabelVisible(bool) override;

public slots:
  void updateItemData() override;

protected slots:
  virtual void setEnabledState(bool checked);
  virtual void onAddSubGroup();
  virtual void onRemoveSubGroup();
  void onChildWidgetSizeChanged() override;
  virtual void onChildItemModified();
  void onImportFromFile();

protected:
  void createWidget() override;
  virtual void addSubGroup(int i);
  virtual void updateExtensibleState();
  virtual void addItemsToTable(int i);
  // Calculate the height of the table of sub groups
  void calculateTableHeight();

  bool m_prependMode;

private:
  qtGroupItemInternals* m_internals;

}; // class
}; // namespace attribute
}; // namespace smtk

#endif
