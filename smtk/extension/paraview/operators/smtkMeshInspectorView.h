//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtkMeshInspectorView_h
#define smtkMeshInspectorView_h

#include "smtk/extension/paraview/operators/smtkPQOperationViewsExtModule.h"
#include "smtk/extension/qt/qtOperationView.h"
#include <vtk_jsoncpp.h> // for Json::Value; must be in header due to VTK mangling

class QColor;
class QIcon;

/// UI component for assigning colors to entities
class SMTKPQOPERATIONVIEWSEXT_EXPORT smtkMeshInspectorView : public smtk::extension::qtOperationView
{
  Q_OBJECT

public:
  smtkTypenameMacro(smtkMeshInspectorView);

  smtkMeshInspectorView(const smtk::view::Information& info);
  ~smtkMeshInspectorView() override;

  static smtk::extension::qtBaseView* createViewWidget(const smtk::view::Information& info);

  bool displayItem(smtk::attribute::ItemPtr) const override;

public Q_SLOTS:
  void updateUI() override;
  void onShowCategory() override;
  void valueChanged(smtk::attribute::ItemPtr optype) override;

protected Q_SLOTS:
  virtual void requestOperation(const smtk::operation::OperationPtr& op);

  /// This slot is used to indicate that the underlying attribute
  /// for the operation should be checked for validity
  virtual void attributeModified();

protected:
  void createWidget() override;
  void setInfoToBeDisplayed() override;
  void prepPaletteChooser();

private:
  class Internals;
  Internals* m_p;
};

#endif // smtkMeshInspectorView_h
