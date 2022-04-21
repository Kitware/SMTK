//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __AttributeBrowser_h
#define __AttributeBrowser_h
#include <memory>
#include <vector>

#include <QDockWidget>

#include <smtk/PublicPointerDefs.h>

namespace Ui
{
class AttributeBrowser;
}

class QModelIndex;
class QStringListModel;
class AttDefDataModel;

/**
 * \brief Widget displaying the hierarchy of attribute definitions.
 *
 * It will eventually be extended to also display analysis, categories,
 * etc. (each on a separate tab). Tab selection will then control the
 * central widget in TemplateEditorMain.
 */
class AttributeBrowser : public QDockWidget
{
  Q_OBJECT

public:
  AttributeBrowser(QWidget* parent = nullptr);
  ~AttributeBrowser() override;

  /**
   * Populate the UI with attribute definitions.
   */
  void populate(smtk::attribute::ResourcePtr resource);

  AttributeBrowser(const AttributeBrowser&) = delete;
  AttributeBrowser& operator=(const AttributeBrowser&) = delete;

public Q_SLOTS:
  /**
   * Emits attDefChanged() with whatever the current selection is. Forcing
   * the emission of this signal can be useful to trigger a sequence of
   * events dependent on the signal (e.g. preview update, etc.).
   */
  void emitAttDefChanged();

Q_SIGNALS:
  void attDefChanged(const QModelIndex& currentIndex, const QModelIndex& previousIndex);
  void resourceChanged(bool needsSaving);

private Q_SLOTS:
  /**
   * Show add definition dialog.
   */
  void onAddDefinition();

  /**
   * Remove the currently selected AttDef from the tree. This also removes
   * all of its children AttDefs.
   */
  void onDeleteDefinition();

  /**
   * Adjust UI (enable/disable buttons, etc.) and emit attDefChanged().
   */
  void onAttDefSelectionChanged(const QModelIndex& currentIndex, const QModelIndex& previousIndex);

  /**
   * Trigger model search.
   */
  void onSearchAttDef(const QString& text);

private:
  void clear();

  void populateDefinitions(smtk::attribute::ResourcePtr resource);

  smtk::attribute::ResourcePtr Resource;
  std::unique_ptr<Ui::AttributeBrowser> Ui;
  AttDefDataModel* AttDefModel = nullptr;
};
#endif
