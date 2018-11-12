//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkRGGEditDuctView - UI component for Edit RGG ducts
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef smtkRGGEditDuctView_h
#define smtkRGGEditDuctView_h

#include "smtk/model/FloatData.h"
#include "smtk/model/IntegerData.h"

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/session/rgg/operators/CreateDuct.h"
#include "smtk/session/rgg/plugin/Exports.h"

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
class smtkRGGEditDuctViewInternals;

class SMTKRGGSESSIONPLUGIN_EXPORT smtkRGGEditDuctView : public smtk::extension::qtBaseView
{
  Q_OBJECT

public:
  smtkRGGEditDuctView(const smtk::extension::ViewInfo& info);
  virtual ~smtkRGGEditDuctView();

  static smtk::extension::qtBaseView* createViewWidget(const smtk::extension::ViewInfo& info);

  bool displayItem(smtk::attribute::ItemPtr) override;

public slots:
  void updateUI() override {} // NB: Subclass implementation causes crashes.
  void requestModelEntityAssociation() override;
  void onShowCategory() override { this->updateAttributeData(); }
  void valueChanged(smtk::attribute::ItemPtr optype) override;

protected slots:
  virtual void requestOperation(const smtk::operation::OperationPtr& op);
  virtual void cancelOperation(const smtk::operation::OperationPtr&);
  virtual void clearSelection();

  // This slot is used to indicate that the underlying attribute
  // for the operation should be checked for validity
  virtual void attributeModified();
  void onAttItemModified(smtk::extension::qtItem* item);
  void apply();

protected:
  void updateAttributeData() override;
  void createWidget() override;
  // When duct item has been modified, this function would populate the edit duct
  // panel
  void updateEditDuctPanel();
  virtual void setInfoToBeDisplayed() override;
  // Helper functions for createWidget
  void createDuctSegmentsTable();
  void addSegmentToTable(int row, double z1, double z2);
  void onSegmentSplit();

  // Create a materialLayersTable and add it to the material stackedWidget
  void createMaterialLayersTable(const size_t index, const size_t numberOfMaterials,
    const size_t offSet, const smtk::model::IntegerList& materials,
    const smtk::model::FloatList& thicknessesN);
  void onAddMaterialLayerBefore();
  void onAddMaterialLayerAfter();
  void onDeleteMaterialLayer();
  void addMaterialLayerToTable(
    QTableWidget* table, int row, int subMaterial, double thick1, double thick2);

  void updateButtonStatus();
  void setupMaterialComboBox(QComboBox* box, bool isCell = false);

private:
  smtkRGGEditDuctViewInternals* Internals;
};

#endif // smtkRGGEditDuctView_h
