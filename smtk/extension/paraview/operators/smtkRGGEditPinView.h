//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR //  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkRGGEditPinView - UI component for Edit RGG pins
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef smtkRGGEditPinView_h
#define smtkRGGEditPinView_h

#include "smtk/bridge/rgg/operators/CreatePin.h"
#include "smtk/extension/paraview/operators/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"

class QColor;
class QComboBox;
class QIcon;
namespace smtk
{
namespace extension
{
class qtItem;
}
}
class smtkRGGEditPinViewInternals;

class SMTKPQOPERATORVIEWSEXT_EXPORT smtkRGGEditPinView : public smtk::extension::qtBaseView
{
  Q_OBJECT

public:
  smtkRGGEditPinView(const smtk::extension::ViewInfo& info);
  virtual ~smtkRGGEditPinView();

  static smtk::extension::qtBaseView* createViewWidget(const smtk::extension::ViewInfo& info);

  bool displayItem(smtk::attribute::ItemPtr) override;

public slots:
  void updateUI() override {} // NB: Subclass implementation causes crashes.
  void requestModelEntityAssociation() override;
  void onShowCategory() override { this->updateAttributeData(); }
  // This will be triggered by selecting different type
  // of construction method in create-edge op.
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

  // UI related slots
  void pieceTypeChanged();

protected:
  void updateAttributeData() override;
  void createWidget() override;
  // When pin item has been modified, this function would populate the edit pin
  // panel
  void updateEditPinPanel();
  virtual void setInfoToBeDisplayed() override;
  // Helper functions for createWidget
  void createPiecesTable();
  void addPieceToTable(
    int row, smtk::bridge::rgg::RGGType type, double height, double baseR, double topR);
  void createLayersTable();
  void addlayerBefore();
  void addlayerAfter();
  void deletelayer();

  void addLayerToTable(int row, int subMaterial, double rN);
  void updateButtonStatus();
  void setupMaterialComboBox(QComboBox* box, bool isCell = false);

private:
  smtkRGGEditPinViewInternals* Internals;
};

#endif // smtkRGGEditPinView_h
