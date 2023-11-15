//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtTaskViewConfiguration_h
#define smtk_extension_qtTaskViewConfiguration_h

#include "smtk/extension/qt/task/qtPreviewArc.h"
#include "smtk/extension/qt/task/qtTaskArc.h"
#include "smtk/task/State.h"

#include <QColor>

#include <array>
#include <unordered_map>

namespace smtk
{
namespace view
{
class Configuration;
}
namespace extension
{

/**\brief An object to hold view configuration settings.
  *
  * This class extracts settings used to render the task view from a
  * `view::Configuration` instance and makes them quickly accessible
  * to the Qt classes that render tasks.
  */
class SMTKQTEXT_EXPORT qtTaskViewConfiguration
{
public:
  qtTaskViewConfiguration(const smtk::view::Configuration& viewConfig);

  QColor backgroundFillColor() const { return m_backgroundFillColor; }
  QColor backgroundGridColor() const { return m_backgroundGridColor; }

  QColor activeTaskColor() const { return m_activeTaskColor; }

  QColor colorForArc(qtTaskArc::ArcType arcType) const
  {
    return m_colorForArc[static_cast<int>(arcType)];
  }
  QColor colorForState(smtk::task::State state) const
  {
    return m_colorForState[static_cast<int>(state)];
  }

  QColor colorForArcType(smtk::string::Token arcType) const
  {
    auto it = m_colorForArcType.find(arcType);
    if (it == m_colorForArcType.end())
    {
      return QColor();
    }
    return it->second;
  }
  QColor colorForArcStatus(qtPreviewArc::ArcStatus status) const
  {
    return m_colorForArcStatus[static_cast<int>(status)];
  }

  qreal nodeWidth() const { return m_nodeWidth; }
  qreal nodeRadius() const { return m_nodeRadius; }
  qreal nodeHeadlineHeight() const { return m_nodeHeadlineHeight; }
  qreal nodeHeadlinePadding() const { return m_nodeHeadlinePadding; }
  qreal nodeBorderThickness() const { return m_nodeBorderThickness; }
  int nodeFontSize() const { return m_nodeFontSize; }
  int nodeLayer() const { return m_nodeLayer; }

  qreal arcWidth() const { return m_arcWidth; }
  qreal arcOutline() const { return m_arcOutline; }
  int arcLayer() const { return m_arcLayer; }

  qreal arrowStemLength() const { return m_arrowStemLength; }
  qreal arrowHeadLength() const { return m_arrowHeadLength; }
  qreal arrowTipAspectRatio() const { return m_arrowTipAspectRatio; }

  int constructionLayer() const { return m_constructionLayer; }

protected:
  QColor m_backgroundFillColor;
  QColor m_backgroundGridColor;

  QColor m_activeTaskColor;

  std::array<QColor, static_cast<int>(qtTaskArc::ArcType::Adaptor) + 1> m_colorForArc;
  std::array<QColor, static_cast<int>(smtk::task::State::Completed) + 1> m_colorForState;
  std::array<QColor, static_cast<int>(qtPreviewArc::ArcStatus::Valid) + 1> m_colorForArcStatus;

  std::unordered_map<smtk::string::Token, QColor> m_colorForArcType;

  qreal m_nodeWidth{ 300. };
  qreal m_nodeRadius{ 4. };
  qreal m_nodeHeadlineHeight{ 13 };
  qreal m_nodeHeadlinePadding{ 4. };
  qreal m_nodeBorderThickness{ 4. };
  int m_nodeFontSize{ 13 };
  int m_nodeLayer{ 10 };

  qreal m_arcWidth{ 4. };
  qreal m_arcOutline{ 1. };
  int m_arcLayer{ 5 };

  qreal m_arrowStemLength{ 16. };    // The length of the path guaranteed to be a straight line.
  qreal m_arrowHeadLength{ 12. };    // The length of the arrow head along the linear stem.
  qreal m_arrowTipAspectRatio{ 2. }; // The width of the arrow head as a fraction of head length.

  int m_constructionLayer{ 15 };
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtTaskViewConfiguration_h
