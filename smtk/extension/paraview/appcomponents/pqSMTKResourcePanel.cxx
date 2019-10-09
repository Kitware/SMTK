//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKResourcePanel.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKResourceBrowser.h"

#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/view/ResourcePhraseModel.h"

pqSMTKResourcePanel::pqSMTKResourcePanel(QWidget* parent)
  : Superclass(parent)
{
  auto phraseModel = smtk::view::ResourcePhraseModel::create();
  std::string modelViewName = "";
  auto qtPhraseModel = new smtk::extension::qtDescriptivePhraseModel;
  smtk::view::ViewPtr newResView(new smtk::view::View("Resource", "Resource View"));
  smtk::extension::ResourceViewInfo resinfo(
    newResView, phraseModel, modelViewName, qtPhraseModel, this);
  // NB: We could call
  //     qtSMTKUtilities::registerModelViewConstructor(modelViewName, ...);
  // here to ensure a Qt model-view class in the same plugin is
  // registered before telling the pqSMTKResourceBrowser to use it.

  m_browser = new pqSMTKResourceBrowser(resinfo);
  m_browser->widget()->setObjectName("pqSMTKResourceBrowser");
  this->setWindowTitle("Resources");
  this->setWidget(m_browser->widget());
}

pqSMTKResourcePanel::~pqSMTKResourcePanel()
{
  delete m_browser;
  // deletion of m_browser->widget() is handled when parent widget is deleted.
}
