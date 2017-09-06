//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __PreviewPanel_h
#define __PreviewPanel_h
#include <memory>

#include <QDockWidget>

#include <smtk/PublicPointerDefs.h>

namespace smtk
{
namespace attribute
{
class Collection;
}

namespace extension
{
class qtUIManager;
}
}

namespace Ui
{
class PreviewPanel;
}

class QModelIndex;

/**
 * \brief Panel widget to display smtk::common::View previews.
 *
 * Receives a signal from AttributeBrowser on an Att. Definition selection
 * change. This creates an Attribute to render a View for the current
 * definition.
 */
class PreviewPanel : public QDockWidget
{
  Q_OBJECT

public:
  PreviewPanel(QWidget* parent, smtk::attribute::CollectionPtr collection);
  ~PreviewPanel();

public slots:
  /**
   * This method should be connected to the signal that will trigger an
   * AttDef preview update. The model contained in the QModelIndex
   * will be used to query the current Definition.
   */
  void updateCurrentView(const QModelIndex& currentDef, const QModelIndex& previousDef);

private:
  PreviewPanel(const PreviewPanel&) = delete;
  void operator=(const PreviewPanel&) = delete;

  /**
 * Sample code to create a View for each AttDef (taken form qtAttributePreview).
 * \note TODO This is deprecated and will be deleted after finishing this class.
 */
  void createViewForAllAttributes(smtk::common::ViewPtr& root);

  /**
 * Instances a smtk::common::View for the input AttDef. Returns a nullptr if the
 * Definition is invalid. Make sure SMTK's specific View "types" are used on
 * View construction for it to construct/display correctly.
 *
 * \sa smtk::extension::qtUIManager qtUIManager::registerViewConstructor
 */
  smtk::common::ViewPtr createView(const smtk::attribute::DefinitionPtr& def);

  /**
 * Populate widget with SMTK-generated attribute widgets.
 */
  void createViewWidget(const smtk::common::ViewPtr& view);

  std::unique_ptr<Ui::PreviewPanel> Ui;
  QWidget* PreviewWidget = nullptr;
  smtk::attribute::CollectionPtr AttributeCollection;
  smtk::attribute::AttributePtr CurrentViewAttr;
  std::unique_ptr<smtk::extension::qtUIManager> UIManager;
};
#endif //__PreviewPanel_h
