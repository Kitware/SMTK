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

#include "smtk/extension/qt/qtDescriptivePhraseDelegate.h"
#include "smtk/extension/qt/qtDescriptivePhraseEditor.h"
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/PhraseList.h"
#include "smtk/view/SubphraseGenerator.h"

#include "ui_pqSMTKResourcePanel.h"

#include <QPointer>

class pqSMTKResourcePanel::Internal : public Ui::pqSMTKResourcePanel
{
public:
  void setup(::pqSMTKResourcePanel* parent)
  {
    QWidget* ww = new QWidget(parent);
    parent->setWindowTitle("SMTK");
    this->setupUi(ww);
    parent->setWidget(ww);
    m_model = new smtk::extension::qtDescriptivePhraseModel;
    m_delegate = new smtk::extension::qtDescriptivePhraseDelegate;
    m_model->setRoot(
      smtk::view::PhraseList::create()->setDelegate(smtk::view::SubphraseGenerator::create()));

    m_delegate->setTextVerticalPad(6);
    m_delegate->setTitleFontWeight(1);
    m_view->setModel(m_model);
    m_view->setItemDelegate(m_delegate);

    QObject::connect(m_searchText, SIGNAL(textChanged(const QString&)), parent,
      SLOT(searchTextChanged(const QString&)));
  }

  QPointer<smtk::extension::qtDescriptivePhraseModel> m_model;
  QPointer<smtk::extension::qtDescriptivePhraseDelegate> m_delegate;
};

pqSMTKResourcePanel::pqSMTKResourcePanel(QWidget* parent)
  : Superclass(parent)
{
  m_p = new Internal;
  m_p->setup(this);
  auto spg = smtk::view::SubphraseGenerator::create();
  this->setPhraseGenerator(spg);
}

pqSMTKResourcePanel::~pqSMTKResourcePanel()
{
  delete m_p;
}

smtk::view::SubphraseGeneratorPtr pqSMTKResourcePanel::phraseGenerator() const
{
  auto root = m_p->m_model->getItem(QModelIndex());
  return root ? root->findDelegate() : nullptr;
}

void pqSMTKResourcePanel::setPhraseGenerator(smtk::view::SubphraseGeneratorPtr spg)
{
  m_p->m_model->clear();
  auto root = m_p->m_model->getItem(QModelIndex());
  root->setDelegate(spg);
  m_p->m_model->rebuildSubphrases(QModelIndex());
}

void pqSMTKResourcePanel::updateTopLevel()
{
  m_p->m_model->rebuildSubphrases(QModelIndex());
}

void pqSMTKResourcePanel::searchTextChanged(const QString& searchText)
{ // For now, just rebuild.
  (void)searchText;
  m_p->m_model->rebuildSubphrases(QModelIndex());
}
