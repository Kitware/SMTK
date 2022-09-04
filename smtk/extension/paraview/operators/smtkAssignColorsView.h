//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtkAssignColorsView_h
#define smtkAssignColorsView_h

#include "smtk/extension/paraview/operators/smtkPQOperationViewsExtModule.h"
#include "smtk/extension/qt/qtOperationView.h"
#include <vtk_jsoncpp.h> // for Json::Value; must be in header due to VTK mangling

class QColor;
class QIcon;
class smtkAssignColorsViewInternals;

/// UI component for assigning colors to entities
class SMTKPQOPERATIONVIEWSEXT_EXPORT smtkAssignColorsView
  : public smtk::extension::qtBaseAttributeView
{
  Q_OBJECT

public:
  smtkTypenameMacro(smtkAssignColorsView);

  smtkAssignColorsView(const smtk::view::Information& info);
  ~smtkAssignColorsView() override;

  static smtk::extension::qtBaseView* createViewWidget(const smtk::view::Information& info);
  static QIcon renderColorSwatch(const QColor& color, int radius);
  static QIcon renderPaletteSwatch(const QList<QColor>& color, int width, int radius);
  static QIcon renderInvalidSwatch(int radius);

  bool displayItem(smtk::attribute::ItemPtr) const override;

public Q_SLOTS:
  void updateUI() override;
  void onShowCategory() override;
  /// This will be triggered by selecting different type
  /// of construction method in create-edge op.
  void valueChanged(smtk::attribute::ItemPtr optype) override;

protected Q_SLOTS:
  virtual void requestOperation(const smtk::operation::OperationPtr& op);

  virtual void chooseDefaultColorAndApply();
  virtual void applyDefaultColor();
  virtual void applyDefaultPalette();
  virtual void processOpacitySlider(int);
  virtual void processOpacityValue(double);
  virtual void removeColors();
  virtual void setDefaultPaletteAndApply();
  /// This slot is used to indicate that the underlying attribute
  /// for the operation should be checked for validity
  virtual void attributeModified();

protected:
  void createWidget() override;
  void setInfoToBeDisplayed() override;
  void prepPaletteChooser();

private:
  smtkAssignColorsViewInternals* Internals;
};

#endif // smtkAssignColorsView_h
