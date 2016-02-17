#include "jobview.h"
#include "jobtablemodel.h"
#include "cumulusproxy.h"

#include <QtGui/QMenu>
#include <QtGui/QContextMenuEvent>


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
  del->setData(jobVariant);
  terminate->setData(jobVariant);

  // Delete
  if (job.status() == "running" || job.status() == "queued" ||
      job.status() == "terminating" || job.status() == "uploading" ||
      job.status() == "terminating") {
    del->setEnabled(false);
  }
  connect(del, SIGNAL(triggered()), this, SLOT(deleteJob()));
  menu->addAction(del);

  // Terminate
  if (job.status() != "running" && job.status() != "queued" &&
      job.status() != "uploading") {
    terminate->setEnabled(false);
  }
  connect(terminate, SIGNAL(triggered()), this, SLOT(terminateJob()));
  menu->addAction(terminate);

  menu->exec(QCursor::pos());
}

void JobView::deleteJob()
{
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

void JobView::setCumulusProxy(CumulusProxy *cumulusProxy)
{
  this->m_cumulusProxy = cumulusProxy;
}


} // end namespace
