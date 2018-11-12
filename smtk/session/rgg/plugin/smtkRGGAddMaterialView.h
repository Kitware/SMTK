//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkRGGAddMaterialView - UI component for Edit RGG pins
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef smtkRGGAddMaterialView_h
#define smtkRGGAddMaterialView_h

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/session/rgg/operators/AddMaterial.h"
#include "smtk/session/rgg/plugin/Exports.h"

class QColor;
class QComboBox;
class QIcon;
class QString;
namespace smtk
{
namespace extension
{
class qtItem;
}
}
class smtkRGGAddMaterialViewInternals;

class SMTKRGGSESSIONPLUGIN_EXPORT smtkRGGAddMaterialView : public smtk::extension::qtBaseView
{
  Q_OBJECT

public:
  smtkRGGAddMaterialView(const smtk::extension::ViewInfo& info);
  virtual ~smtkRGGAddMaterialView();

  static smtk::extension::qtBaseView* createViewWidget(const smtk::extension::ViewInfo& info);

  bool displayItem(smtk::attribute::ItemPtr) override;

public slots:
  void updateUI() override {} // NB: Subclass implementation causes crashes.
  void requestModelEntityAssociation() override;
  void onShowCategory() override { this->updateAttributeData(); }

  void launchNuclideTable();

protected slots:
  virtual void requestOperation(const smtk::operation::OperationPtr& op);
  virtual void cancelOperation(const smtk::operation::OperationPtr&);
  virtual void clearSelection();

  // This slot is used to indicate that the underlying attribute
  // for the operation should be checked for validity
  virtual void attributeModified();
  void apply();

protected:
  void updateAttributeData() override;
  void createWidget() override;
  bool ableToOperate();
  void updateAddMaterialPanel();
  virtual void setInfoToBeDisplayed() override;
  void setupDensityTypeComboBox(QComboBox* box);
  void setupCompositionTypeComboBox(QComboBox* box);

private:
  smtkRGGAddMaterialViewInternals* Internals;
};

#endif // smtkRGGAddMaterialView_h
