//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "jobview.h"
#include "jobtablemodel.h"
#include "cumulusproxy.h"

#include <QMenu>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QFileDialog>

namespace cumulus
{

JobView::JobView(QWidget *parent) : QTableView(parent)
{

}

JobView::~JobView()
{

}

void JobView::contextMenuEvent(QContextMenuEvent *e)
{
  QVariant jobVariant = model()->data(indexAt(e->pos()),
      Qt::UserRole);
  Job job = jobVariant.value<Job>();

  QMenu *menu = new QMenu(this);
  QAction *del = new QAction("Delete", this);
  QAction *terminate = new QAction("Terminate", this);
  QAction *download = new QAction("Download output", this);
  del->setData(jobVariant);
  terminate->setData(jobVariant);
  download->setData(jobVariant);


  if (job.status() == "running" || job.status() == "queued" ||
      job.status() == "terminating" || job.status() == "uploading" ||
      job.status() == "terminating") {
    del->setEnabled(false);
    download->setEnabled(false);
  }
  // Delete
  connect(del, SIGNAL(triggered()), this, SLOT(deleteJob()));
  menu->addAction(del);

  // Terminate
  if (job.status() != "running" && job.status() != "queued" &&
      job.status() != "uploading") {
    terminate->setEnabled(false);
  }
  connect(terminate, SIGNAL(triggered()), this, SLOT(terminateJob()));
  menu->addAction(terminate);

  // Download
  connect(download, SIGNAL(triggered()), this, SLOT(downloadJob()));
  menu->addAction(download);

  menu->exec(QCursor::pos());
}

void JobView::deleteJob()
{
  QMessageBox::StandardButton reply = QMessageBox::question(this->parentWidget(),
      tr("Delete job?"),
      tr("Are you sure you want to delete this job?\n\n"
         "Deleting the job will remove all output files from the cluster."),
      QMessageBox::Yes|QMessageBox::No);

  if (reply == QMessageBox::No) {
      return;
  }

  QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
      return;

  Job job = action->data().value<Job>();

  this->m_cumulusProxy->deleteJob(job);
}

void JobView::terminateJob()
{
  QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
      return;

  Job job = action->data().value<Job>();

  this->m_cumulusProxy->terminateJob(job);
}

void JobView::downloadJob()
{
  QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
      return;

  Job job = action->data().value<Job>();

  QString downloadDir = QFileDialog::getExistingDirectory(
      NULL, tr("Select download directory"), QString(),
      QFileDialog::ShowDirsOnly);


  this->m_cumulusProxy->downloadJob(downloadDir, job);
}

void JobView::setCumulusProxy(CumulusProxy *cumulusProxy)
{
  this->m_cumulusProxy = cumulusProxy;
}


} // end namespace
