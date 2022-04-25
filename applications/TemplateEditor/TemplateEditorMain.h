//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __TemplateEditorMain_h
#define __TemplateEditorMain_h
#include <memory>

#include <QMainWindow>

#include <smtk/PublicPointerDefs.h>

namespace Ui
{
class TemplateEditorMain;
}

class AttributeBrowser;
class AttDefInformation;
class PreviewPanel;

/**
 * \brief TemplateEditor's main window.
 *
 * TemplateEditor is a Qt-based application that serves as a UI to create,
 * edit and preview SMTK sbt files.
 */

class TemplateEditorMain : public QMainWindow
{
  Q_OBJECT

public:
  TemplateEditorMain();
  ~TemplateEditorMain() override;

  TemplateEditorMain(const TemplateEditorMain&) = delete;
  TemplateEditorMain& operator=(const TemplateEditorMain&) = delete;

  /**
   * Load a template file to edit. This method is also used for commmand
   * line arguments.
   */
  void load(char const* fileName);

public Q_SLOTS:
  ///@{
  /**
   * Handlers for QAction signals to load, create new, save, etc.
   */
  void onNew();
  void onLoad();
  void onSaveAs();
  void onSave();

  /**
   * Modify the window title if there has been an Attribute Resource change.
   */
  void updateTitle(bool needsSaving);
  ///@}

private:
  void connectActions();

  /**
   * Initialize panels and other resources.
   */
  void initialize();

  /**
   * Resets application to its initial state. Destorys current panels,
   * etc.
   */
  void reset();

  /**
   * Save template file to filePath. Currently saves the current
   * attribute resource to a single file.
   */
  void save(const QString& filePath);

  smtk::attribute::ResourcePtr AttributeResource;
  QString ActiveFilePath;

  std::unique_ptr<Ui::TemplateEditorMain> Ui;
  AttributeBrowser* AttDefBrowser = nullptr;
  AttDefInformation* AttDefInfo = nullptr;
  PreviewPanel* AttPreviewPanel = nullptr;
};
#endif //__TemplateEditorMain_h
