//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtLegendDelegate_h
#define smtk_extension_qtLegendDelegate_h

#include "smtk/extension/qt/Exports.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include <QStyledItemDelegate>

namespace smtk
{
namespace extension
{

class qtDiagram;
class qtDiagramGenerator;
class qtDiagramLegend;
class qtDiagramLegendEntry;

/**\brief A delegate for rendering symbols into a qtDiagramLegend.
  *
  * An instance of this class should be set as the delegate for
  * the qtDiagramLegend::Column::Symbol column of a qtDiagramLegend's
  * QTableView.
  *
  * The delegate simply fetches the qtDiagramLegendEntry from
  * each cell's data and forwards painting to the legend entry.
  */
class SMTKQTEXT_EXPORT qtLegendDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  smtkSuperclassMacro(QStyledItemDelegate);
  smtkTypeMacroBase(smtk::extension::qtLegendDelegate);

  qtLegendDelegate(QObject* parent = nullptr);
  ~qtLegendDelegate() override;

  /// Called by qtDiagramLegend as the mouse moves across the table:
  bool setHoverIndex(const QModelIndex& index);

  /// Provide a good size for a legend symbol.
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

  /// Paint a symbol into a QTableView cell.
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index)
    const override;

protected:
  QPersistentModelIndex m_hoverIndex;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtLegendDelegate_h
