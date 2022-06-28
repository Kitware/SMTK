//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtDateTimeItem - UI components for attribute DateTimeItem
// .SECTION Description
// .SECTION See Also
// qtItem

#ifndef smtk_extension_qtDateTimeItem_h
#define smtk_extension_qtDateTimeItem_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtItem.h"
#include <QString>

class QAction;
class QDateTime;
class QDateTimeEdit;

namespace smtk
{
namespace extension
{
class SMTKQTEXT_EXPORT qtDateTimeItem : public qtItem
{
  Q_OBJECT

public:
  static qtItem* createItemWidget(const qtAttributeItemInfo& info);
  qtDateTimeItem(const qtAttributeItemInfo& info);
  ~qtDateTimeItem() override;
  void setLabelVisible(bool) override;
  bool eventFilter(QObject* filterObj, QEvent* ev) override;

public Q_SLOTS:
  void setOutputOptional(int);
  void updateItemData() override;

Q_SIGNALS:

protected Q_SLOTS:
  void onChildWidgetSizeChanged() override;
  /* virtual void onAddNewValue(); */
  /* virtual void onRemoveValue(); */

  void onDateTimeChanged(const QDateTime& newValue);

  // Time zone menu actions
  void onTimeZoneUnset();
  void onTimeZoneUTC();
  void onTimeZoneRegion();

  // Time zone dialog actions
  void onRegionSelected();

protected:
  void createWidget() override;
  QWidget* createDateTimeWidget(int elementIdx);
  virtual void loadInputValues();
  virtual void updateUI();
  virtual void addInputEditor(int i);
  /* virtual void updateExtensibleState(); */
  virtual void clearChildWidgets();
  void updateBackground(QDateTimeEdit* dtEdit, bool valid);
  void updateTimeZoneMenu(QAction* selectedAction = nullptr);

  void setTimeZone(std::size_t element, const QString& region);
  void setTimeZoneToUTC(std::size_t element);

private:
  class qtDateTimeItemInternals;
  qtDateTimeItemInternals* Internals;
}; // class qDateTimeItem
}; // namespace extension
}; // namespace smtk

#endif
