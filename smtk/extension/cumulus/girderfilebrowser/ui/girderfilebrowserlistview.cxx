//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <QPaintEvent>
#include <QPainter>

#include "girderfilebrowserlistview.h"

namespace cumulus
{

GirderFileBrowserListView::GirderFileBrowserListView(QWidget* parent)
  : QListView(parent)
{
}

// We overload this method so we can say "Empty" when a folder
// or item is empty.
void GirderFileBrowserListView::paintEvent(QPaintEvent* e)
{
  QListView::paintEvent(e);
  if (model() && model()->rowCount(rootIndex()) > 0)
    return;

  // The view is empty.
  QPainter p(this->viewport());
  p.drawText(rect(), Qt::AlignCenter, "(Empty)");
}

} // end namespace
