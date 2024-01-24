//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtSelectMode_h
#define smtk_extension_qtSelectMode_h

#include "smtk/extension/qt/diagram/qtDiagramViewMode.h"

namespace smtk
{
namespace extension
{

class qtDiagramView;

/**\brief A mode where click+dragging on the background rubber-band selects nodes.
  *
  * This mode does not select arcs. \sa qtDisconnectMode to select arcs.
  */
class SMTKQTEXT_EXPORT qtSelectMode : public qtDiagramViewMode
{
  Q_OBJECT

public:
  using Superclass = qtDiagramViewMode;

  /// Construct a mode for \a diagram and add it to \a toolbar and \a modeGroup.
  qtSelectMode(qtDiagram* diagram, qtDiagramView* view, QToolBar* toolbar, QActionGroup* modeGroup);
  ~qtSelectMode() override;

protected:
  bool eventFilter(QObject* obj, QEvent* event) override;
  // void sceneCleared() override;
  void enterMode() override;
  void exitMode() override;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtSelectMode_h
