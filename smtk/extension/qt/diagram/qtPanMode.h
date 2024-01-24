//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtPanMode_h
#define smtk_extension_qtPanMode_h

#include "smtk/extension/qt/diagram/qtDiagramViewMode.h"

namespace smtk
{
namespace extension
{

class qtDiagramView;

/**\brief A mode where click+dragging on the background pans the view.
  *
  * Pressing the shift key will temporarily (until the shift key is released)
  * switch into qtSelectMode and allow rubber-band selection.
  */
class SMTKQTEXT_EXPORT qtPanMode : public qtDiagramViewMode
{
  Q_OBJECT

public:
  using Superclass = qtDiagramViewMode;

  /// Construct a mode for \a diagram and add it to \a toolbar and \a modeGroup.
  qtPanMode(qtDiagram* diagram, qtDiagramView* view, QToolBar* toolbar, QActionGroup* modeGroup);
  ~qtPanMode() override;

protected:
  friend class qtDiagram;

  bool eventFilter(QObject* obj, QEvent* event) override;
  void enterMode() override;
  void exitMode() override;

  void removeSelection();
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtPanMode_h
