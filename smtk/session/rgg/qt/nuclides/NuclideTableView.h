//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_rgg_qt_nuclides_NuclideTableView_h
#define __smtk_session_rgg_qt_nuclides_NuclideTableView_h

#include "smtk/session/rgg/qt/nuclides/Exports.h"

#include <QFrame>
#include <QGraphicsView>

QT_BEGIN_NAMESPACE
class QLabel;
class QSlider;
class QComboBox;
class QToolButton;
QT_END_NAMESPACE

namespace smtk
{
namespace session
{
namespace rgg
{

class NuclideTableView;

/**\brief A graphics view that enables custom mouse wheel events.
  */
class SMTKQTRGGNUCLIDES_EXPORT NuclideTableGraphicsView : public QGraphicsView
{
  Q_OBJECT
public:
  NuclideTableGraphicsView(NuclideTableView* v)
    : QGraphicsView()
    , view(v)
  {
  }

protected:
#if QT_CONFIG(wheelevent)
  void wheelEvent(QWheelEvent*) override;
#endif

private:
  NuclideTableView* view;
};

/**\brief A frame that contains the table of nuclides.

   This class holds graphical window for the table of nuclides and some
   additional components to interact with it.
*/
class SMTKQTRGGNUCLIDES_EXPORT NuclideTableView : public QFrame
{
  Q_OBJECT
public:
  explicit NuclideTableView(const QString& name, QWidget* parent);

  QGraphicsView* view() const;

public slots:
  void zoomIn(int level = 1);
  void zoomOut(int level = 1);

private slots:
  void setupMatrix();
  void togglePointerMode();

private:
  NuclideTableGraphicsView* graphicsView;
  QComboBox* symbol;
  QComboBox* massNumber;
  QLabel* label;
  QLabel* label2;
  QToolButton* selectModeButton;
  QToolButton* dragModeButton;
  QSlider* zoomSlider;
};
}
}
}

#endif
