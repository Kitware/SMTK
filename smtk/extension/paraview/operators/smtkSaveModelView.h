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

#include "smtk/extension/paraview/operators/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"
#include <vtk_jsoncpp.h> // for Json::Value; must be in header due to VTK mangling

class QColor;
class QIcon;
class QMouseEvent;
class smtkSaveModelViewInternals;

class SMTKPQOPERATORVIEWSEXT_EXPORT smtkSaveModelView : public smtk::extension::qtBaseView
{
  Q_OBJECT

public:
  smtkSaveModelView(const smtk::extension::ViewInfo& info);
  virtual ~smtkSaveModelView();

  static smtk::extension::qtBaseView* createViewWidget(const smtk::extension::ViewInfo& info);

  virtual bool displayItem(smtk::attribute::ItemPtr);

  /**\brief Indicate what effects saving the model will have and what inputs are reuqired.
    *
    * The mode indicates whether
    * + a filename (URL) is required or the pre-existing filename should be used
    * + whether the model(s) being saved should be marked as clean after the operation
    *   or whether the session will still consider them dirty (and thus require user
    *   confirmation before closing the model, session, or application.
    *
    * Both "Save" and "SaveAs" will mark the saved models clean but "SaveACopy" will not.
    * Both "SaveAs" and "SaveACopy" require a filename that, if it already exists, will
    * cause the file browser to prompt whether to overwrite.
    */
  enum SaveMode
  {
    Save,     //!< Save to pre-existing URL, overwriting by default.
    SaveAs,   //!< Save to new URL; overwrite, but if user employs file browser, ask first.
    SaveACopy //!< Save to new URL; overwrite, but if user employs file browser ask first.
              //!< The model is not marked as clean.
  };

  void setEmbedData(bool doEmbed);
  void setRenameModels(bool doRename);

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
  virtual bool onSaveACopy();

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
  virtual void mouseMoveEvent(QMouseEvent* event);
  virtual void updateSummary(const std::string& mode);
  virtual void updateActions();

  template <typename T>
  bool updateOperatorFromUI(const std::string& mode, const T& action);

private:
  smtkSaveModelViewInternals* Internals;
};

#endif // smtkSaveModelView_h
