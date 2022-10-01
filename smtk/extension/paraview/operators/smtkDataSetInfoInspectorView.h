//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtkDataSetInfoInspectorView_h
#define smtkDataSetInfoInspectorView_h

#include "smtk/extension/paraview/operators/smtkPQOperationViewsExtModule.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"
#include <vtk_jsoncpp.h> // for Json::Value; must be in header due to VTK mangling

class QColor;
class QIcon;

/// UI component for assigning colors to entities
class SMTKPQOPERATIONVIEWSEXT_EXPORT smtkDataSetInfoInspectorView
  : public smtk::extension::qtBaseAttributeView
{
  Q_OBJECT

public:
  smtkTypenameMacro(smtkDataSetInfoInspectorView);
  smtkSuperclassMacro(smtk::extension::qtBaseAttributeView);

  smtkDataSetInfoInspectorView(const smtk::view::Information& info);
  ~smtkDataSetInfoInspectorView() override;

  static smtk::extension::qtBaseView* createViewWidget(const smtk::view::Information& info);

  bool displayItem(smtk::attribute::ItemPtr) const override;

public Q_SLOTS: // NOLINT(readability-redundant-access-specifiers)
  void updateUI() override;
  void onShowCategory() override;

protected Q_SLOTS:
  virtual void requestOperation(const smtk::operation::OperationPtr& op);
  /// Given the result of a DataSetInspector operation, fill a QTableWidget with values.
  void updateInfoTable(const smtk::attribute::AttributePtr& result);

  /// This slot is used to indicate that the associations for the operation
  /// may have changed, so the operation should be re-run.
  virtual void inputsChanged();

protected: // NOLINT(readability-redundant-access-specifiers)
  void createWidget() override;

private:
  class Internals;
  Internals* m_p;
};

#endif // smtkDataSetInfoInspectorView_h
