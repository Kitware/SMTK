//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtInstancedView - UI components for attribute Instanced View
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef smtk_extension_qtInstancedView_h
#define smtk_extension_qtInstancedView_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"

#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"

class qtInstancedViewInternals;
class QScrollArea;

namespace smtk
{
namespace extension
{
class SMTKQTEXT_EXPORT qtInstancedView : public qtBaseAttributeView
{
  Q_OBJECT

public:
  smtkTypenameMacro(qtInstancedView);

  ///\brief Create an instance view using an optionally  specified attribute resource instead of the one
  /// associated with the UI Manager

  static qtBaseView* createViewWidget(const smtk::view::Information& info);

  qtInstancedView(const smtk::view::Information& info);
  ~qtInstancedView() override;
  // Returns true if all attributes in the view are valid
  bool isValid() const override;

  bool isEmpty() const override;

public Q_SLOTS:
  void updateUI() override;
  void showAdvanceLevelOverlay(bool show) override;
  void onShowCategory() override;

Q_SIGNALS:
  // emitted when an attribute is modified
  void itemModified(qtItem*);

protected:
  void createWidget() override;
  // This View needs to handle changes made to resources as a result of an operation.
  // This method is used by the observation mechanism to address these changes
  virtual int handleOperationEvent(
    const smtk::operation::Operation& op,
    smtk::operation::EventType event,
    smtk::operation::Operation::Result result);

private:
  qtInstancedViewInternals* Internals;

}; // class
}; // namespace extension
}; // namespace smtk

#endif
