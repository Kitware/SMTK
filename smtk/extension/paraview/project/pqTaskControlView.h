//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_pqTaskControlView_h
#define smtk_extension_pqTaskControlView_h

#include "smtk/extension/paraview/project/smtkPQProjectExtModule.h"
#include "smtk/extension/qt/qtBaseView.h"

namespace smtk
{
namespace extension
{

/**\brief A view for task-level controls.
  *
  * Views of this type may contain any number of widgets for manipulating
  * the currently-active task's completion status, port data, etc.
  * The view may also be configured to show diagnostic/summary information
  * about the active task.
  */
class SMTKPQPROJECTEXT_EXPORT pqTaskControlView : public qtBaseView
{
  Q_OBJECT

public:
  smtkTypenameMacro(smtk::extension::pqTaskControlView);
  smtkSuperclassMacro(smtk::extension::qtBaseView);

  static qtBaseView* createViewWidget(const smtk::view::Information& info);
  pqTaskControlView(const smtk::view::Information& info);
  ~pqTaskControlView() override;

  /// Returns true if the view does not contain any information to display.
  bool isEmpty() const override;

  /// Returns false when users must use the view to adjust the workflow.
  ///
  /// Currently this always returns true, even when the active task is incomplete.
  bool isValid() const override;

public Q_SLOTS:
  void updateUI() override;
  void showAdvanceLevelOverlay(bool show) override;
  void onShowCategory() override;
  void returnToDiagram();
  void updateTaskCompletion(bool completed);

protected:
  void buildUI() override;
  void createWidget() override;
  void updateWithActiveTask(smtk::task::Task* task);

private:
  class Internal;
  Internal* m_p;
};

} // namespace extension
} // namespace smtk

#endif
