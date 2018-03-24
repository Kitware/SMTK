//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkRGGEditAssemblyView - UI component for Edit RGG assemblies
// .SECTION Description
// .SECTION See Also

#ifndef smtkRGGEditAssemblyView_h
#define smtkRGGEditAssemblyView_h

#include "smtk/bridge/rgg/operators/CreateAssembly.h"
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
class smtkRGGEditAssemblyViewInternals;

class SMTKRGGSESSIONPLUGIN_EXPORT smtkRGGEditAssemblyView : public smtk::extension::qtBaseView
{
  Q_OBJECT

public:
  smtkRGGEditAssemblyView(const smtk::extension::ViewInfo& info);
  virtual ~smtkRGGEditAssemblyView();

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

  void calculatePitches();
  void launchSchemaPlanner();
  void onOperationFinished(const smtk::model::OperatorResult& result);

protected:
  void updateAttributeData() override;
  void createWidget() override;
  // When assembly item has been modified, this function would populate the edit assembly
  // panel
  void updateEditAssemblyPanel();
  virtual void setInfoToBeDisplayed() override;

private:
  smtkRGGEditAssemblyViewInternals* Internals;
};

#endif // smtkRGGEditAssemblyView_h
