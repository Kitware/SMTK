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

#ifndef __smtk_extension_qtInstancedView_h
#define __smtk_extension_qtInstancedView_h

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
  static qtBaseView* createViewWidget(const ViewInfo& info);

  qtInstancedView(const ViewInfo& info);
  virtual ~qtInstancedView();
  // Returns true if all attributes in the view are valid
  bool isValid() const;

  bool isEmpty() const override;

public slots:
  void showAdvanceLevelOverlay(bool show) override;
  void onShowCategory() override { this->updateAttributeData(); }

signals:
  // emitted when an attribute is modified
  void modified();
  void itemModified(qtItem*);

protected:
  void updateAttributeData() override;
  void createWidget() override;
  // This View needs to handle changes made to resources as a result of an operation.
  // This method is used by the observation mechanism to address these changes
  virtual int handleOperationEvent(const smtk::operation::Operation& op,
    smtk::operation::EventType event, smtk::operation::Operation::Result result);

private:
  qtInstancedViewInternals* Internals;

}; // class
}; // namespace attribute
}; // namespace smtk

#endif
