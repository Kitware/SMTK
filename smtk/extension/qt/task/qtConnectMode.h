//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtConnectMode_h
#define smtk_extension_qtConnectMode_h

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
class qtPreviewArc;

/**\brief A mode where clicking on a pair of objects will
  *       launch an operation to connect them with an arc.
  *
  * Pressing the escape key after the first object has been selected
  * will restart the arc-creation process.
  * Pressing the escape key when no object has been selected will
  * exit this mode for the task-editor's default mode.
  */
class SMTKQTEXT_EXPORT qtConnectMode : public qtGraphViewMode
{
  Q_OBJECT

public:
  using Superclass = qtGraphViewMode;

  /// Construct a mode for \a editor and add it to \a toolbar and \a modeGroup.
  qtConnectMode(
    const std::shared_ptr<smtk::operation::Manager>& operationManager,
    qtTaskEditor* editor,
    qtTaskView* view,
    QToolBar* toolbar,
    QActionGroup* modeGroup);

  ~qtConnectMode() override;

public Q_SLOTS:
  void hoverConnectNode(qtBaseTaskNode* node);
  void clickConnectNode(qtBaseTaskNode* node);
  void abandonConnection();

protected:
  friend class qtTaskEditor;

  bool eventFilter(QObject* obj, QEvent* event) override;
  void sceneCleared() override;
  void enterMode() override;
  void exitMode() override;

  void updateArcTypes();
  void setConnectionType(int arcTypeItemIndex);

  QPointer<qtPreviewArc> m_previewArc;
  QPointer<QComboBox> m_connectType;
  QPointer<QAction> m_connectTypeAction;
  std::shared_ptr<smtk::operation::Manager> m_operationManager;
  smtk::operation::GroupObservers::Key m_groupObserverKey;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtConnectMode_h
