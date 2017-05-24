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
 * \brief Widget displaying the internal structure of an attribute template
 * file.
 *
 */
class AttributeBrowser : public QDockWidget
{
  Q_OBJECT

public:
  AttributeBrowser(QWidget* parent = nullptr);
  ~AttributeBrowser();

  /**
 * Populate the UI with SMTK attributes, definitions, etc.
 */
  void populate(smtk::attribute::SystemPtr system);

signals:
  void attDefChanged(const QModelIndex& currentIndex, const QModelIndex& previousIndex);

private slots:
  /**
 * Show add definition dialog.
 */
  void onAddDefinition();

  /**
 * Remove the currently selected AttDef from the tree. This also removes
 * all of its children AttDefs.
 */
  void onDeleteDefinition();

private:
  AttributeBrowser(const AttributeBrowser&) = delete;
  void operator=(const AttributeBrowser&) = delete;

  void clear();

  void populateDefinitions(smtk::attribute::SystemPtr system);

  smtk::attribute::SystemPtr System;
  std::unique_ptr<Ui::AttributeBrowser> Ui;
  AttDefDataModel* AttDefModel = nullptr;
};
#endif
