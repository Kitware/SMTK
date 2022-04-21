//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/operators/Signal.h"

#include "smtk/operation/Manager.h"

#include "smtk/io/Logger.h"

#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/view/Configuration.h"

#include <QApplication>
#include <QWidget>

using namespace smtk::extension;

bool qtBaseView::validateInformation(const smtk::view::Information& info)
{
  return info.contains<smtk::view::ConfigurationPtr>() && info.contains<QWidget*>();
}

qtBaseView::qtBaseView(const smtk::view::Information& info)
{
  m_viewInfo = info;
  this->Widget = nullptr;
  m_advOverlayVisible = false;
  m_isTopLevel = false;
  m_useSelectionManager = false;
  const auto& view = this->configuration();
  if (view)
  {
    m_isTopLevel = view->details().attributeAsBool("TopLevel");
    m_useSelectionManager = view->details().attributeAsBool("UseSelectionManager");
  }
}

qtBaseView::~qtBaseView()
{
  Q_EMIT aboutToDestroy();
}

void qtBaseView::makeTopLevel()
{
  smtk::view::ConfigurationPtr view = this->configuration();
  auto* uiMgr = this->uiManager();
  if (!view || !uiMgr)
  {
    return;
  }

  int pos;

  pos = view->details().findChild("AdvancedFontEffects");
  if (pos != -1)
  {
    bool val;
    if (!view->details().child(pos).attributeAsBool("Bold", val))
    {
      uiMgr->setAdvanceFontStyleBold(val);
    }
    if (!view->details().child(pos).attributeAsBool("Italic", val))
    {
      uiMgr->setAdvanceFontStyleItalic(val);
    }
  }

  pos = view->details().findChild("MaxValueLabelLength");
  if (pos != -1)
  {
    int l;
    if (view->details().child(pos).contentsAsInt(l))
    {
      uiMgr->setMaxValueLabelLength(l);
    }
  }

  pos = view->details().findChild("MinValueLabelLength");
  if (pos != -1)
  {
    int l;
    if (view->details().child(pos).contentsAsInt(l))
    {
      uiMgr->setMinValueLabelLength(l);
    }
  }
}

void qtBaseView::onInfo()
{
  if (!m_infoDialog)
  {
    // Try to get the dialog to be displayed on top - note that in the
    // case of dock widgets this can be an issue.  In that case to at least get the dialog
    // not to be completely hidden by the operator widget when it is undocked
    // we need to parent the dialog on something else
    QWidgetList l = QApplication::topLevelWidgets();
    m_infoDialog = new qtViewInfoDialog(l.value(0));
  }
  this->setInfoToBeDisplayed();
  m_infoDialog->show();
  m_infoDialog->raise();
  m_infoDialog->activateWindow();
}

void qtBaseView::setInfoToBeDisplayed()
{
  m_infoDialog->displayInfo(this->configuration());
}
