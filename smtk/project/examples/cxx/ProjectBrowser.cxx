//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/project/examples/cxx/ProjectBrowser.h"

#include "smtk/extension/qt/qtDescriptivePhraseDelegate.h"
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/project/examples/cxx/ui_ProjectBrowser.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/PhraseContent.h"
#include "smtk/view/PhraseModel.h"

#include "smtk/model/Group.h"
#include "smtk/model/Model.h"

#include <QPushButton>
#include <QTreeView>

namespace Ui
{
class qtAttributeAssociation;
}

using namespace smtk::model;

class ProjectBrowser::Internals : public Ui::ProjectBrowser
{
public:
  smtk::extension::qtDescriptivePhraseModel* qmodel;
  smtk::extension::qtDescriptivePhraseDelegate* qdelegate;
};

ProjectBrowser::ProjectBrowser(QWidget* p)
  : QWidget(p)
{
  m_p = new ProjectBrowser::Internals;
  m_p->setupUi(this);
}

ProjectBrowser::~ProjectBrowser()
{
  delete m_p;
}

QTreeView* ProjectBrowser::tree() const
{
  return m_p->modelTree;
}

void ProjectBrowser::setup(
  smtk::resource::ManagerPtr manager,
  smtk::extension::qtDescriptivePhraseModel* qmodel,
  smtk::extension::qtDescriptivePhraseDelegate* qdelegate)
{
  m_manager = manager;
  m_p->modelTree->setItemDelegate(qdelegate);
  m_p->qmodel = qmodel;
  m_p->modelTree->setModel(m_p->qmodel);
  m_p->qdelegate = qdelegate;
  // NB: If we want the ProjectBrowser widget to handle badge
  //     clicks, we would need to install an event filter
  //     on m_p->modelTree->viewport() and call
  //     qtDescriptivePhraseDelegate::processBadgeClick on
  //     mouse button-press events from the event filter.
}
