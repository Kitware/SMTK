//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR //  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkRGGEditCoreView - UI component for Edit RGG Core
// .SECTION Description
// .SECTION See Also

#ifndef smtkRGGEditCoreView_h
#define smtkRGGEditCoreView_h

#include "smtk/bridge/rgg/operators/CreateModel.h"
#include "smtk/bridge/rgg/plugin/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"

class QColor;
class QComboBox;
class QIcon;
class QTableWidget;

namespace smtk
{
namespace extension
{
class qtItem;
}
}

class smtkRGGEditCoreViewInternals;

class SMTKRGGSESSIONPLUGIN_EXPORT smtkRGGEditCoreView : public smtk::extension::qtBaseView
{
  Q_OBJECT

public:
  smtkRGGEditCoreView(const smtk::extension::ViewInfo& info);
  virtual ~smtkRGGEditCoreView();

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
  void onAttItemModified(smtk::extension::qtItem* item);
  void apply();

  void launchSchemaPlanner();
  void onOperationFinished(const smtk::model::OperatorResult& result);

protected:
  void updateAttributeData() override;
  void createWidget() override;
  // When the core item has been modified, this function would populate the edit
  // core panel
  void updateEditCorePanel();
  virtual void setInfoToBeDisplayed() override;

private:
  smtkRGGEditCoreViewInternals* Internals;
};

#endif // smtkRGGEditCoreView_h
