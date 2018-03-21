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

#ifndef __smtk_extension_qtDiscreteValueEditor_h
#define __smtk_extension_qtDiscreteValueEditor_h

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
  virtual ~qtDiscreteValueEditor();
  QSize sizeHint() const override;
  bool useSelectionManger() const { return m_useSelectionManager; }

public slots:
  void onInputValueChanged();
  // Indicates if it is possible use the Selection Manager
  // void setUseSelectionManager(bool mode) { m_useSelectionManager = mode; }

protected slots:
  virtual void updateItemData();

protected:
  virtual void createWidget();

private:
  qtDiscreteValueEditorInternals* Internals;
  bool m_useSelectionManager;

}; // class
}; // namespace attribute
}; // namespace smtk

#endif
