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

#include "smtk/extension/qt/diagram/qtDiagramViewMode.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"

class QComboBox;

namespace smtk
{
namespace extension
{

class qtBaseObjectNode;
class qtDiagramView;
class qtPreviewArc;

/**\brief A mode where clicking on a pair of objects will
  *       launch an operation to connect them with an arc.
  *
  * Pressing the escape key after the first object has been selected
  * will restart the arc-creation process.
  * Pressing the escape key when no object has been selected will
  * exit this mode for the diagram's default mode.
  */
class SMTKQTEXT_EXPORT qtConnectMode : public qtDiagramViewMode
{
  Q_OBJECT

public:
  using Superclass = qtDiagramViewMode;

  /// Construct a mode for \a diagram and add it to \a toolbar and \a modeGroup.
  qtConnectMode(
    qtDiagram* diagram,
    qtDiagramView* view,
    QToolBar* toolbar,
    QActionGroup* modeGroup);

  ~qtConnectMode() override;

  /// Return the preview arc this mode uses to indicate potential connections.
  qtPreviewArc* previewArc() const;

  /// Ensure the preview arc does not become invalid when objects are expunged.
  void updateFromOperation(
    std::unordered_set<smtk::resource::PersistentObject*>& created,
    std::unordered_set<smtk::resource::PersistentObject*>& modified,
    std::unordered_set<smtk::resource::PersistentObject*>& expunged,
    const smtk::operation::Operation& operation,
    const smtk::operation::Operation::Result& result) override;

public Q_SLOTS:
  void hoverConnectNode(qtBaseObjectNode* node);
  void clickConnectNode(qtBaseObjectNode* node);
  void abandonConnection();

protected:
  friend class qtDiagram;

  bool eventFilter(QObject* obj, QEvent* event) override;
  void sceneCleared(qtDiagramGenerator* generator) override;
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
