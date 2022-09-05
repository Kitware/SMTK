//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_paraview_operators_EditPropertiesView_h
#define smtk_paraview_operators_EditPropertiesView_h

#include "smtk/extension/paraview/operators/Exports.h"
#include "smtk/extension/qt/qtOperationView.h"

/// User interface for editing properties of components
class SMTKPQOPERATIONVIEWSPLUGIN_EXPORT smtkEditPropertiesView
  : public smtk::extension::qtBaseAttributeView
{
  Q_OBJECT;
  using Superclass = smtk::extension::qtBaseAttributeView;

public:
  smtkTypenameMacro(smtkEditPropertiesView);

  smtkEditPropertiesView(const smtk::view::Information& info);
  ~smtkEditPropertiesView() override;

  static bool validateInformation(const smtk::view::Information& info);
  static smtk::extension::qtBaseView* createViewWidget(const smtk::view::Information& info);

  bool displayItem(smtk::attribute::ItemPtr item) const override;

public Q_SLOTS: // NOLINT(readability-redundant-access-specifiers)
  void updateUI() override;
  void requestModelEntityAssociation() override;
  void onShowCategory() override;
  /// This will be triggered by selecting different type
  /// of construction method in create-edge op.
  void valueChanged(smtk::attribute::ItemPtr valItem) override;

protected Q_SLOTS:
  virtual void requestOperation(const smtk::operation::OperationPtr& op);
  virtual void tableSelectionChanged();
  virtual void addOrReplaceAttribute();
  virtual void removeSelectedAttribute();
  virtual void propertyTypeChanged(int index);

protected: // NOLINT(readability-redundant-access-specifiers)
  void createWidget() override;
  void setInfoToBeDisplayed() override;

private:
  class Internals;
  Internals* m_p;
  Q_DISABLE_COPY(smtkEditPropertiesView);
};

#endif // smtk_paraview_operators_EditPropertiesView_h
