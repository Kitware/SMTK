//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtResourceBrowserP.h"

#include "smtk/extension/qt/qtDescriptivePhraseDelegate.h"
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"
#include "smtk/extension/qt/qtSMTKUtilities.h"

#include "smtk/view/ResourcePhraseModel.h"

#include "smtk/io/Logger.h"

#include <QAbstractProxyModel>
#include <QMetaObject>
#include <QTreeView>
#include <QVBoxLayout>

using namespace smtk::extension;

/// @relates smtk::extension::qtResourceBrowser::Internal
qtResourceBrowser::Internal::Internal()
  : m_container(nullptr)
  , m_layout(nullptr)
  , m_view(nullptr)
  , m_selnSource("resource panel")
  , m_selnLabel("selected")
  , m_hoverLabel("hovered")
  , m_resourceTreeStyle(-1)
  , m_updatingPanelSelectionFromSMTK(false)

{
}

/// @relates smtk::extension::qtResourceBrowser::Internal
qtResourceBrowser::Internal::~Internal()
{
  // Unregister our decorator before we become invalid.
  m_phraseModel->setDecorator([](smtk::view::DescriptivePhrasePtr) {});
  delete m_view;
  m_view = nullptr;
  delete m_container;
}

/// @relates smtk::extension::qtResourceBrowser::Internal
void qtResourceBrowser::Internal::setup(qtResourceBrowser* self,
  const smtk::view::PhraseModelPtr& phraseModel, const std::string& viewName,
  QAbstractItemModel* qmodel, QWidget* parent)
{
  m_self = self;
  if (m_container)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "qtResourceBrowser internal setup called more than once.");
    return;
  }

  // Keep or create a phrase model to present to users:
  m_phraseModel = phraseModel ? phraseModel : smtk::view::ResourcePhraseModel::create();

  // Create a QAbstractItemView subclass given viewName:
  auto viewMap = qtSMTKUtilities::modelViewConstructors();
  auto vcit = viewMap.find(viewName);
  if (vcit == viewMap.end())
  {
    vcit = viewMap.find(""); // The default constructor.
  }
  qtModelViewConstructor ctor = vcit->second;
  if (!ctor)
  {
    ctor = qtResourceBrowser::createDefaultView;
  }
  m_container = new QWidget(parent);
  m_container->setObjectName("qtResourceBrowser");
  m_layout = new QVBoxLayout(m_container);
  m_layout->setObjectName("m_layout");

  m_view = ctor(parent);

  m_layout->addWidget(m_view);
  m_view->installEventFilter(self);
  m_viewName = viewName;

  // Keep or create a QAbstractItemModel subclass (which had better be
  // related somehow to a qtDescriptivePhraseModel).
  m_model = qmodel ? qmodel : new qtDescriptivePhraseModel;
  auto dpmodel = this->descriptivePhraseModel();
  if (dpmodel)
  {
    dpmodel->setPhraseModel(m_phraseModel);
  }
  else
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "qtResourceBrowser was passed an invalid QAbstractItemModel"
        << " of type \"" << m_model->metaObject()->className() << "\"");
  }

  // Create a default item delegate for rendering rows from m_model into m_view:
  m_delegate = new smtk::extension::qtDescriptivePhraseDelegate;
  m_delegate->setTextVerticalPad(6);
  m_delegate->setTitleFontWeight(1);
  m_delegate->setDrawSubtitle(false);

  m_view->setModel(m_model);
  m_view->setItemDelegate(m_delegate);
  m_view->setMouseTracking(true); // Needed to receive hover events.
  // Connect signals
  if (dpmodel)
  {
    QObject::connect(m_delegate, SIGNAL(requestVisibilityChange(const QModelIndex&)), dpmodel,
      SLOT(toggleVisibility(const QModelIndex&)));
  }
  QObject::connect(m_delegate, SIGNAL(requestColorChange(const QModelIndex&)), m_self,
    SLOT(editObjectColor(const QModelIndex&)));

  QObject::connect(m_view->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), m_self,
    SLOT(sendPanelSelectionToSMTK(const QItemSelection&, const QItemSelection&)));
}

/// @relates smtk::extension::qtResourceBrowser::Internal
qtDescriptivePhraseModel* qtResourceBrowser::Internal::descriptivePhraseModel() const
{
  auto dpmodel = dynamic_cast<qtDescriptivePhraseModel*>(m_model.data());
  if (!dpmodel)
  {
    auto sfmodel = dynamic_cast<QAbstractProxyModel*>(m_model.data());
    if (sfmodel)
    {
      dpmodel = dynamic_cast<qtDescriptivePhraseModel*>(sfmodel->sourceModel());
    }
  }
  return dpmodel;
}

/// @relates smtk::extension::qtResourceBrowser::Internal
void qtResourceBrowser::Internal::setDescriptivePhraseModel(QAbstractItemModel* qmodel)
{
  m_model = qmodel;
  m_view->setModel(m_model);
}
