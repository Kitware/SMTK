//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtDiscreteValueEditor - an item for display discrete value item
// .SECTION Description
// .SECTION See Also

#ifndef smtk_extension_qtDiscreteValueEditor_h
#define smtk_extension_qtDiscreteValueEditor_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtItem.h"
#include <QWidget>

class qtDiscreteValueEditorInternals;

namespace smtk
{
namespace extension
{
class qtInputsItem;

class SMTKQTEXT_EXPORT qtDiscreteValueEditor : public QWidget
{
  Q_OBJECT

public:
  qtDiscreteValueEditor(qtInputsItem* item, int elementIdx, QLayout* childLayout);
  ~qtDiscreteValueEditor() override;
  bool useSelectionManger() const { return m_useSelectionManager; }
  void updateContents();

  /** Tab ordering methods.
   *
   * The current logic does not support conditional children items.
   */
  QWidget* lastEditingWidget() const; // the last internal editing widget
  void updateTabOrder(QWidget* /*precedingEditor*/) {}

public Q_SLOTS:
  void onInputValueChanged();
  virtual void updateItemData();
  // Indicates if it is possible use the Selection Manager
  // void setUseSelectionManager(bool mode) { m_useSelectionManager = mode; }

Q_SIGNALS:
  /// /brief Signal indicates that the underlying widget's size has been modified
  void widgetSizeChanged();

  /** \brief Indicates that editing widget changed. */
  void editingWidgetChanged();

protected Q_SLOTS:

protected:
  virtual void createWidget();

private:
  qtDiscreteValueEditorInternals* Internals;
  bool m_useSelectionManager{ false };

}; // class
}; // namespace extension
}; // namespace smtk

#endif
