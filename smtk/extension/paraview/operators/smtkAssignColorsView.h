//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkAssignColorsView - UI component for assigning colors to entities
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef smtkAssignColorsView_h
#define smtkAssignColorsView_h

#include "smtk/extension/paraview/operators/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"
#include <vtk_jsoncpp.h> // for Json::Value; must be in header due to VTK mangling

class QColor;
class QIcon;
class smtkAssignColorsViewInternals;

class SMTKPQOPERATORVIEWSEXT_EXPORT smtkAssignColorsView : public smtk::extension::qtBaseView
{
  Q_OBJECT

public:
  smtkAssignColorsView(const smtk::extension::ViewInfo& info);
  virtual ~smtkAssignColorsView();

  static smtk::extension::qtBaseView* createViewWidget(const smtk::extension::ViewInfo& info);
  static QIcon renderColorSwatch(const QColor& color, int radius);
  static QIcon renderPaletteSwatch(const QList<QColor>& color, int width, int radius);
  static QIcon renderInvalidSwatch(int radius);

  virtual bool displayItem(smtk::attribute::ItemPtr);

public slots:
  virtual void updateUI() {} // NB: Subclass implementation causes crashes.
  virtual void showAdvanceLevelOverlay(bool show);
  virtual void requestModelEntityAssociation();
  virtual void onShowCategory() { this->updateAttributeData(); }
  // This will be triggered by selecting different type
  // of construction method in create-edge op.
  virtual void valueChanged(smtk::attribute::ItemPtr optype);

protected slots:
  virtual void requestOperation(const smtk::model::OperatorPtr& op);
  virtual void cancelOperation(const smtk::model::OperatorPtr&);
  virtual void clearSelection();

  virtual void chooseDefaultColorAndApply();
  virtual void applyDefaultColor();
  virtual void setDefaultPaletteAndApply(const Json::Value& preset);
  virtual void applyDefaultPalette();
  virtual void removeColors();

  // This slot is used to indicate that the underlying attribute
  // for the operation should be checked for validity
  virtual void attributeModified();

protected:
  virtual void updateAttributeData();
  virtual void createWidget();

private:
  smtkAssignColorsViewInternals* Internals;
};

#endif // smtkAssignColorsView_h
