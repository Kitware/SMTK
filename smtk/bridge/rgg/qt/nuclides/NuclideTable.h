//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_bridge_rgg_qt_nuclides_NuclideTable_h
#define __smtk_bridge_rgg_qt_nuclides_NuclideTable_h

#include "smtk/bridge/rgg/qt/nuclides/Exports.h"
#include "smtk/bridge/rgg/qt/nuclides/Nuclide.h"

#include <QGraphicsScene>
#include <QMap>
#include <QSet>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

namespace smtk
{
namespace bridge
{
namespace rgg
{

/**\brief Convert between atomic number and chemical symbol.
  */
class SMTKQTRGGNUCLIDES_EXPORT NuclideSymbolConverter
{
public:
  unsigned int operator()(const QString& symbol) const { return m_symbolToZ[symbol]; }
  QString operator()(const unsigned int& z) const { return m_zToSymbol[z]; }

  void insert(const QString& symbol, const unsigned int& z);

private:
  QMap<QString, unsigned int> m_symbolToZ;
  QMap<unsigned int, QString> m_zToSymbol;
};

/**\brief A graphics scene depicting the table of nuclides.

   This class depicts a 2-dimensional interactive table of nuclides. It also
   contains the signal for nuclide selection that is used internally by the
   NuclideTableView and NuclideTable.
*/
class SMTKQTRGGNUCLIDES_EXPORT NuclideTableScene : public QGraphicsScene
{
  Q_OBJECT
public:
  NuclideTableScene(QWidget* parent);

signals:
  /// Internal signal to indicate when a nuclide is selected. This signal is
  /// repeated for external use by NuclideTable, and users are encouraged to
  /// connect to NuclideTable::nuclideSelected instead.
  void nuclideSelected(Nuclide*);

protected:
  virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
  virtual void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;
  virtual void dropEvent(QGraphicsSceneDragDropEvent* event) override;

protected slots:
  void onNuclideSelected(Nuclide*);

private:
  Nuclide* m_activeNuclide;
};

/**\brief A widget depicting the table of nuclides.

   This class depicts an interactive table of nuclides. It is the class that
   users should implement in their projects when they want a table of nuclides.
   It contains a signal for when a nuclide has been selected and a means for
   converting to and from atomic number and chemical symbol. Nuclide data is
   read in from Nuclear Wallet Cards (www.nndc.bnl.gov).
*/
class SMTKQTRGGNUCLIDES_EXPORT NuclideTable : public QWidget
{
  Q_OBJECT
public:
  NuclideTable(QWidget* parent = 0);

  QGraphicsScene* scene() const { return m_scene; }

  /// Retrieve the symbols of all selectable nuclides in the table.
  QSet<QString> symbols() const;

  /// Given a symbol, retrieve the mass numbers of all selectable nuclides
  /// corresponding to this symbol.
  QSet<unsigned int> massNumbers(const QString& symbol) const;

  /// Retrieve a nuclide via its name. This is not performant (O[N]).
  Nuclide* nuclide(const QString& name) const;

  /// Retrieve a nuclide via its id. This is not performant (O[N]).
  Nuclide* nuclide(const unsigned int& id) const;

  /// Retrieve a nuclide via its pretty name. This is not performant (O[N]).
  Nuclide* nuclideFromPrettyName(const QString& prettyName) const;

  /// Access a symbol converter that was populated using the same data that was
  /// used to generate the table.
  const NuclideSymbolConverter& symbolConverter() const { return m_symbolConverter; }

signals:
  /// Client-facing signal for when a nuclide is selected. This signal simply
  /// echoes its similarly named signal from NuclideTableScene, but none of the
  /// internal mechanisms of the NuclideTable and NuclideTableView depend on
  /// this signal for correct functionality. A user is therefore able to
  /// disconnect all connections from this signal without breaking the table's
  /// interactivity.
  void nuclideSelected(Nuclide*);

private:
  void populateScene();

  NuclideSymbolConverter m_symbolConverter;

  QGraphicsScene* m_scene;
  QSet<Nuclide*> m_nuclides;
};
}
}
}

#endif
