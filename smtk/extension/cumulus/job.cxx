#include "job.h"

#include <QString>

namespace cumulus
{

Job::Job()
{

}

Job::Job(const QString &id, const QString &name, const QString &status)
{
  this->m_id = id;
  this->m_name = name;
  this->m_status = status;
}

Job::Job(const Job &job)
{
  this->m_id = job.id();
  this->m_name = job.name();
  this->m_status = job.status();
}

Job::~Job()
{

}

} // end namespace
