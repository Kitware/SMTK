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

#include "smtk/extension/qt/task/qtGraphViewMode.h"

namespace smtk
{
namespace extension
{

class qtTaskView;

/**\brief A mode where click+dragging on the background pans the view.
  *
  * Pressing the shift key will temporarily (until the shift key is released)
  * switch into qtSelectMode and allow rubber-band selection.
  */
class SMTKQTEXT_EXPORT qtPanMode : public qtGraphViewMode
{
  Q_OBJECT

public:
  using Superclass = qtGraphViewMode;

  /// Construct a mode for \a editor and add it to \a toolbar and \a modeGroup.
  qtPanMode(qtTaskEditor* editor, qtTaskView* view, QToolBar* toolbar, QActionGroup* modeGroup);
  ~qtPanMode() override;

protected:
  friend class qtTaskEditor;

  bool eventFilter(QObject* obj, QEvent* event) override;
  void enterMode() override;
  void exitMode() override;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtPanMode_h
