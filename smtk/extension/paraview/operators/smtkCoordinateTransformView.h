//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_paraview_operators_CoordinateTransformView_h
#define smtk_paraview_operators_CoordinateTransformView_h

#include "smtk/extension/paraview/operators/smtkPQOperationViewsExtModule.h"
#include "smtk/extension/qt/qtOperationView.h"

/**\brief User interface for editing the transform of a component.
  *
  * Transforms are specified by providing a "from" coordinate frame
  * and a "to" coordinate frame. The transform acts by translating
  * the origin to move between the two frames and reorienting the
  * part so axes which were aligned with the source frame are now
  * aligned with the destination frame.
  *
  * This custom view provides a tree-widget for you to select
  * components to translate. Beneath it is another tree-widget
  * listing freeform coordinate-frame properties present; these
  * may be copied to the "from" or "to" coordinate frames via a
  * context menu item.
  *
  * The bottom portion of the view allows you to edit the "from"
  * and "to" coordinate frames. When you click "Apply," the
  * transform is saved as a property on the associated components.
  * There is also a drop-down button that allows you to remove
  * the transform from the associated components.
  */
class SMTKPQOPERATIONVIEWSEXT_EXPORT smtkCoordinateTransformView
  : public smtk::extension::qtBaseAttributeView
{
  Q_OBJECT;
  using Superclass = smtk::extension::qtBaseAttributeView;

public:
  smtkTypenameMacro(smtkCoordinateTransformView);

  smtkCoordinateTransformView(const smtk::view::Information& info);
  ~smtkCoordinateTransformView() override;

  static bool validateInformation(const smtk::view::Information& info);
  static smtk::extension::qtBaseView* createViewWidget(const smtk::view::Information& info);

public Q_SLOTS: // NOLINT(readability-redundant-access-specifiers)
  void updateUI() override;
  void requestModelEntityAssociation() override;
  void onShowCategory() override;
  void valueChanged(smtk::attribute::ItemPtr valItem) override;
  /// This slot is triggered by the "remove transform" action.
  /// It will immediately invoke the operator on the associated
  /// items and permanently remove their transforms.
  void removeTransform();
  /// This slot is triggered by clicking the Apply button and
  /// will immediately run the operation if its parameters are valid.
  void applyTransform();

protected Q_SLOTS:
  virtual void requestOperation(const smtk::operation::OperationPtr& op);
  virtual void treeSelectionChanged();
  virtual void togglePreview(bool showPreview);
  /// Invoked by actions to copy a landmark to one of the operation's inputs:
  virtual void useLandmarkAsFrom();
  virtual void useLandmarkAsTo();
  /// Connected to pqSMTKCoordinateFramePropertyWidget::modified to curate operation.
  virtual void userEditedFromFrame();
  virtual void userEditedToFrame();
  /// Connected to qtReferenceItem::modified to curation operation.
  virtual void associationChanged();
  /// Called when the operation's parameters change in order to enable/disable actions.
  virtual void parametersChanged();

protected: // NOLINT(readability-redundant-access-specifiers)
  void createWidget() override;
  void setInfoToBeDisplayed() override;

private:
  class Internals;
  Internals* m_p;
  Q_DISABLE_COPY(smtkCoordinateTransformView);
};

#endif // smtk_paraview_operators_CoordinateTransformView_h
