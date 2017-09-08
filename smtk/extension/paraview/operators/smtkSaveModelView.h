//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkSaveModelView - UI component for assigning colors to entities
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef smtkSaveModelView_h
#define smtkSaveModelView_h

#include "smtk/extension/paraview/operators/smtkModelIOView.h"

class QColor;
class QIcon;
class QMouseEvent;
class smtkSaveModelViewInternals;

class SMTKPQOPERATORVIEWSEXT_EXPORT smtkSaveModelView : public smtkModelIOView
{
  Q_OBJECT

public:
  smtkSaveModelView(const smtk::extension::ViewInfo& info);
  virtual ~smtkSaveModelView();

  static smtk::extension::qtBaseView* createViewWidget(const smtk::extension::ViewInfo& info);

  bool displayItem(smtk::attribute::ItemPtr) override;

  void setEmbedData(bool doEmbed) override;
  void setRenameModels(bool doRename) override;

public slots:
  void updateUI() override;
  void showAdvanceLevelOverlay(bool show) override;
  void requestModelEntityAssociation() override;
  void onShowCategory() override { this->updateAttributeData(); }
  // This will be triggered by selecting different type
  // of construction method in create-edge op.
  void valueChanged(smtk::attribute::ItemPtr optype) override;

  void setModeToPreview(const std::string& mode) override;

  void setModelToSave(const smtk::model::Model& model) override;
  bool canSave() const override;

  bool onSave() override;
  bool onSaveAs() override;
  bool onExport() override;

  bool chooseFile(const std::string& mode) override;
  bool attemptSave(const std::string& mode) override;

protected slots:
  virtual bool requestOperation(const smtk::model::OperatorPtr& op);
  virtual void cancelOperation(const smtk::model::OperatorPtr&);
  virtual void clearSelection();

  // This slot is used to indicate that the underlying attribute
  // for the operation should be checked for validity
  virtual void attributeModified();

  virtual void refreshSummary();

  virtual void widgetDestroyed(QObject* w);

protected:
  void updateAttributeData() override;
  void createWidget() override;
  bool eventFilter(QObject* obj, QEvent* evnt) override;
  virtual void updateSummary(const std::string& mode);
  virtual void updateActions();
  virtual void setInfoToBeDisplayed() override;

  template <typename T>
  bool updateOperatorFromUI(const std::string& mode, const T& action);

private:
  smtkSaveModelViewInternals* Internals;
};

#endif // smtkSaveModelView_h
