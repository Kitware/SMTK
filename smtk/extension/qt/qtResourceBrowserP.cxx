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
#include <QLineEdit>
#include <QMetaObject>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QVBoxLayout>

#include <sstream>

using namespace smtk::extension;

/// @relates smtk::extension::qtResourceBrowser::Internal
qtResourceBrowser::Internal::Internal()
  : m_selnLabel("selected")
  , m_hoverLabel("hovered")
{
  std::ostringstream name;
  name << "resource panel " << this;
  m_selnSource = name.str();
}

/// @relates smtk::extension::qtResourceBrowser::Internal
qtResourceBrowser::Internal::~Internal()
{
  delete m_view;
  m_view = nullptr;
  delete m_container;
}

/// @relates smtk::extension::qtResourceBrowser::Internal
void qtResourceBrowser::Internal::setup(
  qtResourceBrowser* self,
  const smtk::view::PhraseModelPtr& phraseModel,
  const std::string& viewName,
  QAbstractItemModel* qmodel,
  QWidget* parent,
  const std::shared_ptr<smtk::view::Selection>& selection,
  bool searchBar)
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
  // Always use for sorting, but not always for search (filtering)
  m_filter = new QSortFilterProxyModel(m_container);
  m_filter->setFilterCaseSensitivity(Qt::CaseInsensitive);
  m_filter->setSortCaseSensitivity(Qt::CaseInsensitive);
  // so we can see parents if children are matched.
  m_filter->setRecursiveFilteringEnabled(true);

  if (searchBar)
  {
    m_search = new QLineEdit(m_container);
    m_search->setObjectName("Search");
    m_search->setPlaceholderText("Search");
    auto* controlLayout = new QHBoxLayout();
    controlLayout->setObjectName("controlLayout");
    m_layout->addItem(controlLayout);
    controlLayout->addWidget(m_search);
    QObject::connect(
      m_search, &QLineEdit::textChanged, m_filter, &QSortFilterProxyModel::setFilterWildcard);
  }

  m_view = ctor(parent);

  m_layout->addWidget(m_view);
  m_view->installEventFilter(self);
  m_view->viewport()->installEventFilter(self);
  m_viewName = viewName;

  // Keep or create a QAbstractItemModel subclass (which had better be
  // related somehow to a qtDescriptivePhraseModel).
  m_model = qmodel ? qmodel : new qtDescriptivePhraseModel;
  auto* dpmodel = this->descriptivePhraseModel();
  if (dpmodel)
  {
    dpmodel->setPhraseModel(m_phraseModel);
  }
  else
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "qtResourceBrowser was passed an invalid QAbstractItemModel"
        << " of type \"" << m_model->metaObject()->className() << "\"");
  }

  // Create a default item delegate for rendering rows from m_model into m_view:
  m_delegate = new smtk::extension::qtDescriptivePhraseDelegate;
  m_delegate->setTextVerticalPad(6);
  m_delegate->setTitleFontWeight(1);
  m_delegate->setDrawSubtitle(false);
  m_delegate->setSelection(selection); // Used for badge actions

  m_filter->setSourceModel(m_model);
  m_view->setModel(m_filter);
  m_view->setItemDelegate(m_delegate);
  m_view->setMouseTracking(true); // Needed to receive hover events.
  // Connect signals
  QObject::connect(
    m_view->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    m_self,
    SLOT(sendPanelSelectionToSMTK(const QItemSelection&, const QItemSelection&)));
}

/// @relates smtk::extension::qtResourceBrowser::Internal
qtDescriptivePhraseModel* qtResourceBrowser::Internal::descriptivePhraseModel() const
{
  auto* dpmodel = dynamic_cast<qtDescriptivePhraseModel*>(m_model.data());
  if (!dpmodel)
  {
    auto* sfmodel = dynamic_cast<QAbstractProxyModel*>(m_model.data());
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
  if (m_filter)
  {
    m_filter->setSourceModel(m_model);
    m_view->setModel(m_filter);
  }
  else
  {
    m_view->setModel(m_model);
  }
  QObject::connect(
    m_view->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    m_self,
    SLOT(sendPanelSelectionToSMTK(const QItemSelection&, const QItemSelection&)));
}
