//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_SVGIconEngine_h
#define smtk_extension_SVGIconEngine_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h" // For EXPORT macro.
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/PhraseModel.h"
#include "smtk/view/PhraseModelObserver.h"

#include <QByteArray>
#include <QIcon>
#include <QIconEngine>

#include <map>

namespace smtk
{
namespace extension
{

/**\brief A class to render SVG into a QIcon.
  *
  * This class is adapted from StackOverflow:
  * https://stackoverflow.com/questions/43125339/creating-qicon-from-svg-contents-in-memory
  */
class SMTKQTEXT_EXPORT SVGIconEngine : public QIconEngine
{
public:
  explicit SVGIconEngine(const std::string& iconBuffer);
  void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override;
  QIconEngine* clone() const override;
  QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override;

protected:
  QByteArray data;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_SVGIconEngine_h
