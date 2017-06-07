//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_operators_smtkModelIOView_h
#define smtk_extension_paraview_operators_smtkModelIOView_h

#include "smtk/extension/paraview/operators/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"
#include <vtk_jsoncpp.h> // for Json::Value; must be in header due to VTK mangling

class QColor;
class QIcon;
class QMouseEvent;

class SMTKPQOPERATORVIEWSEXT_EXPORT smtkModelIOView : public smtk::extension::qtBaseView
{
  Q_OBJECT

public:
  smtkModelIOView(const smtk::extension::ViewInfo& info)
    : smtk::extension::qtBaseView(info)
  {
  }
  virtual ~smtkModelIOView() {}
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
    Save,   //!< Save to pre-existing URL, overwriting by default.
    SaveAs, //!< Save to new URL; overwrite, but if user employs file browser, ask first.
    Export  //!< Save to new URL; overwrite, but if user employs file browser ask first.
            //!< The model is not marked as clean.
  };

  virtual void setEmbedData(bool doEmbed) = 0;
  virtual void setRenameModels(bool doRename) = 0;

public slots:
  virtual void setModeToPreview(const std::string& mode) = 0;

  virtual void setModelToSave(const smtk::model::Model& model) = 0;
  virtual bool canSave() const = 0;

  virtual bool onSave() = 0;
  virtual bool onSaveAs() = 0;
  virtual bool onExport() = 0;

  virtual void chooseFile(const std::string& mode) = 0;
  virtual void attemptSave(const std::string& mode) = 0;
};

#endif // smtk_extension_paraview_operators_smtkModelIOView_h
