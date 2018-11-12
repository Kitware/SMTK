//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_rgg_qt_nuclides_Nuclide_h
#define __smtk_session_rgg_qt_nuclides_Nuclide_h

#include "smtk/session/rgg/qt/nuclides/Exports.h"

#include <QColor>
#include <QGraphicsItem>

namespace smtk
{
namespace session
{
namespace rgg
{

/// Primary decay modes for nuclides. Used for nuclide graphics coloring.
enum class SMTKQTRGGNUCLIDES_EXPORT MainDecayMode
{
  alpha = 0,
  beta_m,
  p,
  n,
  electronCapture,
  spontaneousFission,
  stable,
  unknown
};

/**\brief A graphical depiction of a nuclide within the table of nuclides.

   This class holds the information about a single nuclide and it's graphical
    representation.
  */
class SMTKQTRGGNUCLIDES_EXPORT Nuclide : public QGraphicsItem
{
public:
  Nuclide(unsigned int N, unsigned int Z, const QString& symbol, const QString& jn,
    MainDecayMode decayMode, double halfLife);

  /// Toggle the selectability of the nuclide. If the nuclide is not available
  /// within the current solver, it should be disabled.
  void setSelectable(bool choice);

  /// Methods for visually representing the nuclide in the table of nuclides.
  QRectF boundingRect() const override;
  QPainterPath shape() const override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget) override;

  /// Construct an icon for this nuclide.
  QPixmap toPixmap() const;

  /// Neutron number
  unsigned int N() const { return m_N; }

  /// Atomic number
  unsigned int Z() const { return m_Z; }

  /// Mass number
  unsigned int A() const { return N() + Z(); }

  /// Chemical symbol for the element
  const QString& symbol() const { return m_symbol; }

  /// Nuclide name as described by pyarc
  QString name() const;

  /// Nuclide id as described by pyarc
  unsigned int id() const { return Z() * 1000 + A(); }

  /// A more display-friendly name
  QString prettyName() const;

  /// Nuclear spin
  const QString& jn() const { return m_jn; }

  /// Primary decay mode
  MainDecayMode decayMode() const { return m_decayMode; }

  /// Half life (0.0 if stable or unknown)
  double halfLife() const { return m_halfLife; }

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

  // Background color for this nuclide (determined by decay mode).
  QColor color() const;

private:
  unsigned int m_N;
  unsigned int m_Z;
  QString m_symbol;
  QString m_jn;
  MainDecayMode m_decayMode;
  double m_halfLife;
};
}
}
}

#endif
