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
  // NB: We could call
  //     qtSMTKUtilities::registerModelViewConstructor(modelViewName, ...);
  // here to ensure a Qt model-view class in the same plugin is
  // registered before telling the pqSMTKResourceBrowser to use it.

  m_browser = new pqSMTKResourceBrowser(phraseModel, modelViewName, qtPhraseModel, this);
  m_browser->setObjectName("pqSMTKResourceBrowser");
  this->setWindowTitle("Resources");
  this->setWidget(m_browser);
}

pqSMTKResourcePanel::~pqSMTKResourcePanel()
{
  // deletion of m_browser is handled when parent widget is deleted.
}
