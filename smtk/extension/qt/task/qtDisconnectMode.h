//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtDisconnectMode_h
#define smtk_extension_qtDisconnectMode_h

#include "smtk/extension/qt/task/qtGraphViewMode.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"

class QComboBox;

namespace smtk
{
namespace extension
{

class qtBaseTaskNode;
class qtTaskView;

/**\brief A mode where click+dragging rubberband-selects arcs.
  *       Pressing the backspace key with arcs selected will
  *       launch an operation to delete the arcs.
  *
  * If no operation exists to delete any of the selected arcs,
  * none of the selected arcs will be deleted and an error message
  * will be provided.
  */
class SMTKQTEXT_EXPORT qtDisconnectMode : public qtGraphViewMode
{
  Q_OBJECT

public:
  using Superclass = qtGraphViewMode;

  /// Construct a mode for \a editor and add it to \a toolbar and \a modeGroup.
  qtDisconnectMode(
    const std::shared_ptr<smtk::operation::Manager>& operationManager,
    qtTaskEditor* editor,
    qtTaskView* view,
    QToolBar* toolbar,
    QActionGroup* modeGroup);

  ~qtDisconnectMode() override = default;

public Q_SLOTS:
  void removeSelectedArcs();

protected:
  friend class qtTaskEditor;

  bool eventFilter(QObject* obj, QEvent* event) override;
  void enterMode() override;
  void exitMode() override;

  std::shared_ptr<smtk::operation::Manager> m_operationManager;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtDisconnectMode_h
