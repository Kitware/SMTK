//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtDiagramLegend_h
#define smtk_extension_qtDiagramLegend_h

#include "smtk/extension/qt/diagram/qtDiagram.h"

#include "smtk/PublicPointerDefs.h"

#include <QGroupBox>
#include <QPointer>

class QSortFilterProxyModel;
class QStandardItemModel;
class QTableView;

namespace smtk
{
namespace extension
{

class qtDiagramLegendEntry;
class qtDiagram;
class qtLegendDelegate;

/**\brief An interactive legend to control what is displayed in a qtDiagram.
  *
  * Each qtDiagramGenerator may add entries to the legend for the types of
  * nodes and arcs it presents. Users may then toggle the visibility of
  * these via the legend.
  */
class SMTKQTEXT_EXPORT qtDiagramLegend : public QGroupBox
{
  Q_OBJECT

public:
  smtkSuperclassMacro(QGroupBox);

  /// An enumerant describing the columns of the legend's model.
  enum Column
  {
    Group,      //!< The group (arc, node, etc.) of a type in the diagram.
    Symbol,     //!< The visual appearance of a type in the diagram.
    Description //!< A text description of a type in the diagram.
  };

  qtDiagramLegend(const QString& title, qtDiagram* parent = nullptr);
  ~qtDiagramLegend() override;

  using EntriesByType = std::unordered_map<smtk::string::Token, qtDiagramLegendEntry*>;
  using GroupedEntries = std::unordered_map<smtk::string::Token, EntriesByType>;

  const EntriesByType& typesInGroup(smtk::string::Token group) const;

  /// Add an entry to the legend.
  bool addEntry(qtDiagramLegendEntry* entry);

  /// Remove an entry from the legend.
  bool removeEntry(qtDiagramLegendEntry* entry);

  /// Return the diagram this summary widget belongs to.
  qtDiagram* diagram() const;

protected Q_SLOTS:
  /// This is called when model changes or user interactions requires a relayout.
  ///
  /// Specifically, when (1) rows are inserted, (2) rows are modified, or
  /// (3) the user changes a column width, this method is invoked.
  /// This method calls resizeRowsToContents() so that word-wrapping is
  /// performed and re-sorts the model as needed.
  /// Note that we do not display the horizontal header (with its sort indicators)
  /// because we always sort by two columns (description then group) so that
  /// entries for the same types of things (arcs, nodes, etc.) are kept together.
  void legendUpdated();

protected:
  /// Called by removeEntry() to update m_index so \a entry is no longer indexed.
  void eraseIndex(qtDiagramLegendEntry* entry);

  /// Watch m_view's mouse events to tell m_symbolDelegate where to draw controls for visibility.
  // bool eventFilter(QObject* object, QEvent* event) override;

  /// The parent qtResourceDiagram
  QPointer<qtDiagram> m_diagram;
  /// The unsorted model, in order of insertion.
  QStandardItemModel* m_keys{ nullptr };
  /// A sorted version of m_keys.
  QSortFilterProxyModel* m_sortModel{ nullptr };
  /// The view of m_sortModel.
  QTableView* m_view{ nullptr };
  /// An index of legend entries.
  GroupedEntries m_index;
  /// The delegate used to render the Symbol column of m_view.
  qtLegendDelegate* m_symbolDelegate;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtDiagramLegend_h
