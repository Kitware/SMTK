#include "job.h"
#include "cJSON.h"

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

Job Job::fromJSON(cJSON *obj)
{
  cJSON *idItem = cJSON_GetObjectItem(obj, "_id");
  if (!idItem || idItem->type != cJSON_String) {
    return Job();
  }
  QString id(idItem->valuestring);

  cJSON *nameItem = cJSON_GetObjectItem(obj, "name");
  if (!nameItem || nameItem->type != cJSON_String) {
    return Job();
  }
  QString name(nameItem->valuestring);

  cJSON *statusItem = cJSON_GetObjectItem(obj, "status");
  if (!statusItem || statusItem->type != cJSON_String) {
    return Job();
  }
  QString status(statusItem->valuestring);

  return Job(id, name, status);
}


} // end namespace
