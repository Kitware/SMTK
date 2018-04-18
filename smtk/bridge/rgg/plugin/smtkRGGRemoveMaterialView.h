//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkRGGRemoveMaterialView - UI component for Remove RGG pins
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef smtkRGGRemoveMaterialView_h
#define smtkRGGRemoveMaterialView_h

#include "smtk/bridge/rgg/operators/RemoveMaterial.h"
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
class smtkRGGRemoveMaterialViewInternals;

class SMTKRGGSESSIONPLUGIN_EXPORT smtkRGGRemoveMaterialView : public smtk::extension::qtBaseView
{
  Q_OBJECT

public:
  smtkRGGRemoveMaterialView(const smtk::extension::ViewInfo& info);
  virtual ~smtkRGGRemoveMaterialView();

  static smtk::extension::qtBaseView* createViewWidget(const smtk::extension::ViewInfo& info);

  bool displayItem(smtk::attribute::ItemPtr) override;

public slots:
  void updateUI() override {} // NB: Subclass implementation causes crashes.
  void requestModelEntityAssociation() override;
  void onShowCategory() override { this->updateAttributeData(); }
  void valueChanged(smtk::attribute::ItemPtr optype) override;

protected slots:
  virtual void requestOperation(const smtk::model::OperatorPtr& op);
  virtual void cancelOperation(const smtk::model::OperatorPtr&);
  virtual void clearSelection();

  // This slot is used to indicate that the underlying attribute
  // for the operation should be checked for validity
  virtual void attributeModified();
  bool ableToOperate();
  void apply();

protected:
  void updateAttributeData() override;
  void createWidget() override;
  void updateRemoveMaterialPanel();
  virtual void setInfoToBeDisplayed() override;
  void setupMaterialComboBox(QComboBox* box);

private:
  smtkRGGRemoveMaterialViewInternals* Internals;
};

#endif // smtkRGGRemoveMaterialView_h
