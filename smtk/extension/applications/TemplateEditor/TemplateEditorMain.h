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
 * \brief GUI to edit and visualize smtk attribute template files.
 */

class TemplateEditorMain : public QMainWindow
{
  Q_OBJECT

public:
  TemplateEditorMain();
  ~TemplateEditorMain();

  /**
 * Load a template to edit.
 */
  void load(char const* fileName);

public slots:
  /**
 * Create a new and empty template to work on.
 */
  void onNew();
  void onLoad();
  void onSaveAs();
  void onSave();

private:
  TemplateEditorMain(const TemplateEditorMain&) = delete;
  void operator=(const TemplateEditorMain&) = delete;

  void connectActions();

  void initialize();

  void reset();

  void save(const QString& filePath);

  smtk::attribute::SystemPtr AttributeSystem;
  QString ActiveFilePath;

  std::unique_ptr<Ui::TemplateEditorMain> Ui;
  AttributeBrowser* AttDefBrowser = nullptr;
  AttDefInformation* AttDefInfo = nullptr;
  PreviewPanel* AttPreviewPanel = nullptr;
};
#endif //__TemplateEditorMain_h
