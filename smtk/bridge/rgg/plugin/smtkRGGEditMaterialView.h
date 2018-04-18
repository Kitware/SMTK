//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkRGGEditMaterialView - UI component for Edit RGG pins
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef smtkRGGEditMaterialView_h
#define smtkRGGEditMaterialView_h

#include "smtk/bridge/rgg/operators/EditMaterial.h"
#include "smtk/bridge/rgg/plugin/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"

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
class smtkRGGEditMaterialViewInternals;

class SMTKRGGSESSIONPLUGIN_EXPORT smtkRGGEditMaterialView : public smtk::extension::qtBaseView
{
  Q_OBJECT

public:
  smtkRGGEditMaterialView(const smtk::extension::ViewInfo& info);
  virtual ~smtkRGGEditMaterialView();

  static smtk::extension::qtBaseView* createViewWidget(const smtk::extension::ViewInfo& info);

  bool displayItem(smtk::attribute::ItemPtr) override;

public slots:
  void updateUI() override {} // NB: Subclass implementation causes crashes.
  void requestModelEntityAssociation() override;
  void onShowCategory() override { this->updateAttributeData(); }
  void valueChanged(smtk::attribute::ItemPtr optype) override;

  void launchNuclideTable();

protected slots:
  virtual void requestOperation(const smtk::model::OperatorPtr& op);
  virtual void cancelOperation(const smtk::model::OperatorPtr&);
  virtual void clearSelection();

  virtual void materialChanged(const QString& text);

  // This slot is used to indicate that the underlying attribute
  // for the operation should be checked for validity
  virtual void attributeModified();
  void apply();

protected:
  void updateAttributeData() override;
  void createWidget() override;
  bool ableToOperate();
  void updateEditMaterialPanel();
  virtual void setInfoToBeDisplayed() override;
  void setupMaterialComboBox(QComboBox* box);
  void setupDensityTypeComboBox(QComboBox* box);
  void setupCompositionTypeComboBox(QComboBox* box);
  void clear();
  void setEnabled(bool);

private:
  smtkRGGEditMaterialViewInternals* Internals;
};

#endif // smtkRGGEditMaterialView_h
