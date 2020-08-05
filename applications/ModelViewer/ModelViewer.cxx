//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "ModelViewer.h"
#include "ui_ModelViewer.h"

class ModelViewer::Internals : public Ui::ModelViewer
{
public:
};

ModelViewer::ModelViewer(QWidget* p)
  : QWidget(p)
{
  m_p = new ModelViewer::Internals;
  m_p->setupUi(this);
}

ModelViewer::~ModelViewer()
{
  delete m_p;
}
