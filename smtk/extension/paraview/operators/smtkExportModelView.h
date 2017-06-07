//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtkExportModelView_h
#define smtkExportModelView_h

#include "smtk/extension/paraview/operators/smtkModelIOView.h"

class QColor;
class QIcon;
class QMouseEvent;
class smtkExportModelViewInternals;

/// A view for exporting SMTK "packages" (SMTK files with data saved to the same directory).
class SMTKPQOPERATORVIEWSEXT_EXPORT smtkExportModelView : public smtkModelIOView
{
  Q_OBJECT

public:
  smtkExportModelView(const smtk::extension::ViewInfo& info);
  virtual ~smtkExportModelView();

  static smtk::extension::qtBaseView* createViewWidget(const smtk::extension::ViewInfo& info);

  virtual bool displayItem(smtk::attribute::ItemPtr);

  virtual void setEmbedData(bool doEmbed);
  virtual void setRenameModels(bool doRename);

public slots:
  virtual void updateUI();
  virtual void showAdvanceLevelOverlay(bool show);
  virtual void requestModelEntityAssociation();
  virtual void onShowCategory() { this->updateAttributeData(); }
  // This will be triggered by selecting different type
  // of construction method in create-edge op.
  virtual void valueChanged(smtk::attribute::ItemPtr optype);

  virtual void setModeToPreview(const std::string& mode);

  virtual void setModelToSave(const smtk::model::Model& model);
  virtual bool canSave() const;

  virtual bool onSave();
  virtual bool onSaveAs();
  virtual bool onExport();

  virtual void chooseFile(const std::string& mode);
  virtual void attemptSave(const std::string& mode);

protected slots:
  virtual void requestOperation(const smtk::model::OperatorPtr& op);
  virtual void cancelOperation(const smtk::model::OperatorPtr&);
  virtual void clearSelection();

  // This slot is used to indicate that the underlying attribute
  // for the operation should be checked for validity
  virtual void attributeModified();

  virtual void refreshSummary();

  virtual void widgetDestroyed(QObject* w);

protected:
  virtual void updateAttributeData();
  virtual void createWidget();
  virtual bool eventFilter(QObject* obj, QEvent* evnt);
  virtual void updateSummary(const std::string& mode);
  virtual void updateActions();

  template <typename T>
  bool updateOperatorFromUI(const std::string& mode, const T& action);

private:
  smtkExportModelViewInternals* Internals;
};

#endif // smtkExportModelView_h
