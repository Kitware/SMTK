//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/rgg/qt/nuclides/NuclideTable.h"
#include "smtk/session/rgg/qt/nuclides/Nuclide.h"
#include "smtk/session/rgg/qt/nuclides/NuclideTableView.h"

#include <QDebug>
#include <QFile>
#include <QGraphicsSceneDragDropEvent>
#include <QHBoxLayout>
#include <QMimeData>

#include <set>
#include <utility>

void initNuclideResource()
{
  Q_INIT_RESOURCE(NuclideResources);
}

namespace smtk
{
namespace session
{
namespace rgg
{
void NuclideSymbolConverter::insert(const QString& symbol, const unsigned int& z)
{
  m_symbolToZ.insert(symbol, z);
  m_zToSymbol.insert(z, symbol);
}

NuclideTableScene::NuclideTableScene(QWidget* parent)
  : QGraphicsScene(parent)
  , m_activeNuclide(nullptr)
{
  connect(this, SIGNAL(nuclideSelected(Nuclide*)), this, SLOT(onNuclideSelected(Nuclide*)));
}

void NuclideTableScene::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
  bool acceptDrag = false;
  const QMimeData* mime = event->mimeData();

  if (mime->hasText())
  {
    QString text = mime->text();
    acceptDrag = !text.isEmpty();
  }

  event->setAccepted(acceptDrag);
}

void NuclideTableScene::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
  if (event->mimeData()->hasText())
  {
    event->acceptProposedAction();
  }
  else
  {
    event->ignore();
  }
}

void NuclideTableScene::dropEvent(QGraphicsSceneDragDropEvent* event)
{
  if (event->mimeData()->hasText())
  {
    QString nuclideName = event->mimeData()->text();
    NuclideTable* table = static_cast<NuclideTable*>(parent());
    Nuclide* nuclide = table->nuclide(nuclideName);
    if (nuclide != nullptr)
    {
      nuclide->setSelected(true);
      emit nuclideSelected(nuclide);
    }
  }
}

void NuclideTableScene::onNuclideSelected(Nuclide* nuclide)
{
  if (m_activeNuclide != nullptr)
  {
    m_activeNuclide->setSelected(false);
  }
  m_activeNuclide = nuclide;
}

NuclideTable::NuclideTable(QWidget* parent)
  : QWidget(parent)
{
  initNuclideResource();

  populateScene();

  connect(m_scene, SIGNAL(nuclideSelected(Nuclide*)), this, SIGNAL(nuclideSelected(Nuclide*)));

  NuclideTableView* view = new NuclideTableView("View", this);
  view->view()->setScene(m_scene);

  QHBoxLayout* layout = new QHBoxLayout;
  layout->addWidget(view);
  setLayout(layout);

  setWindowTitle(tr("Table of Nuclides"));
}

QSet<QString> NuclideTable::symbols() const
{
  QSet<QString> symbols_;

  QSet<Nuclide*>::const_iterator i;
  for (i = m_nuclides.begin(); i != m_nuclides.end(); ++i)
  {
    if ((*i)->flags() & QGraphicsItem::ItemIsSelectable)
    {
      symbols_.insert((*i)->symbol());
    }
  }

  return symbols_;
}

QSet<unsigned int> NuclideTable::massNumbers(const QString& symbol) const
{
  QSet<unsigned int> isotopes_;

  QSet<Nuclide*>::const_iterator i;
  for (i = m_nuclides.begin(); i != m_nuclides.end(); ++i)
  {
    if ((*i)->symbol() == symbol && (*i)->flags() & QGraphicsItem::ItemIsSelectable)
      isotopes_.insert((*i)->A());
  }

  return isotopes_;
}

Nuclide* NuclideTable::nuclide(const QString& name) const
{
  QSet<Nuclide*>::const_iterator i;
  for (i = m_nuclides.begin(); i != m_nuclides.end(); ++i)
  {
    if ((*i)->name() == name && (*i)->flags() & QGraphicsItem::ItemIsSelectable)
      return *i;
  }

  return nullptr;
}

Nuclide* NuclideTable::nuclide(const unsigned int& id) const
{
  QSet<Nuclide*>::const_iterator i;
  for (i = m_nuclides.begin(); i != m_nuclides.end(); ++i)
  {
    if ((*i)->id() == id && (*i)->flags() & QGraphicsItem::ItemIsSelectable)
      return *i;
  }

  return nullptr;
}

void NuclideTable::populateScene()
{
  m_scene = new NuclideTableScene(this);

  {
    QFile file(":/rgg/qt/nuclide/nuclear-wallet-cards.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      qDebug() << "Unable to open file: " << file.fileName() << " besause of error "
               << file.errorString() << endl;
    }

    std::set<std::pair<unsigned int, unsigned int> > registeredNuclides;

    while (!file.atEnd())
    {
      QString line = file.readLine();

      // skip metastable nuclides
      if (line[0] == "!")
      {
        continue;
      }

      auto toDecayMode = [](const QString& ln) {
        auto input = QStringRef(&ln, 30, 4).trimmed();
        if (input == "2B-" || input == "B-" || input == "BN")
        {
          return MainDecayMode::beta_m;
        }
        else if (input == "2EC" || input == "EC" || input == "EP")
        {
          return MainDecayMode::electronCapture;
        }
        else if (input == "2N" || input == "2N?" || input == "N")
        {
          return MainDecayMode::n;
        }
        else if (input == "2P" || input == "P")
        {
          return MainDecayMode::p;
        }
        else if (input == "A")
        {
          return MainDecayMode::alpha;
        }
        else if (input == "SF")
        {
          return MainDecayMode::spontaneousFission;
        }
        else if (QStringRef(&ln, 63, 9).trimmed() != "STABLE")
        {
          return MainDecayMode::unknown;
        }
        else
        {
          return MainDecayMode::stable;
        }
      };

      auto A = QStringRef(&line, 1, 3).toInt();
      auto Z = QStringRef(&line, 6, 3).toInt();
      auto N = A - Z;
      auto symbol = QStringRef(&line, 10, 2).trimmed().toString();
      if (symbol.size() > 1)
      {
        for (int i = 1; i < symbol.size(); i++)
          symbol[i] = symbol[i].toLower();
      }
      auto j_n = QStringRef(&line, 16, 7).trimmed().toString();

      auto decayMode = toDecayMode(line);
      auto halfLife = QStringRef(&line, 124, 9).toFloat();

      Nuclide* nuclide = new Nuclide(N, Z, symbol, j_n, decayMode, halfLife);
      if (registeredNuclides.insert(std::make_pair(N, Z)).second == false)
        continue;
      m_nuclides.insert(nuclide);
      m_symbolConverter.insert(symbol, Z);
      nuclide->setSelectable(true);
      nuclide->setPos(QPointF(N * 100, Z * -100));
      m_scene->addItem(nuclide);
    }
  }
}
}
}
}
