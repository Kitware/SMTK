#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/SubphraseGenerator.h"

namespace smtk {
  namespace model {

DescriptivePhrase::DescriptivePhrase()
  : m_type(INVALID_DESCRIPTION), m_subphrasesBuilt(false)
{
}

DescriptivePhrasePtr DescriptivePhrase::setup(DescriptivePhraseType ptype, Ptr parnt)
{
  this->m_parent = parnt;
  this->m_type = ptype;
  this->m_subphrasesBuilt = false;
  return shared_from_this();
}

DescriptivePhrasePtr DescriptivePhrase::setDelegate(SubphraseGeneratorPtr delegate)
{
  this->m_delegate = delegate;
  return shared_from_this();
}

DescriptivePhrases& DescriptivePhrase::subphrases()
{
  this->buildSubphrases();
  return this->m_subphrases;
}

/// Return the index of the given phrase in this instance's subphrases (or -1).
int DescriptivePhrase::argFindChild(DescriptivePhrase* child) const
{
  int i = 0;
  DescriptivePhrases::const_iterator it;
  for (it = this->m_subphrases.begin(); it != this->m_subphrases.end(); ++it, ++i)
    {
    if (it->get() == child)
      return i;
    }
  return -1;
}

/// Find the subphrase generator to use. It may be held by a parent phrase.
SubphraseGeneratorPtr DescriptivePhrase::findDelegate()
{
  DescriptivePhrasePtr phr;
  for (
    phr = shared_from_this();
    phr && !phr->m_delegate;
    phr = phr->parent())
    /* do nothing */ ;
  return phr ? phr->m_delegate : SubphraseGeneratorPtr();
}

/// Build (if required) the cached subphrases of this phrase.
void DescriptivePhrase::buildSubphrases()
{
  if (!this->m_subphrasesBuilt)
    {
    this->m_subphrasesBuilt = true;
    SubphraseGeneratorPtr delegate = this->findDelegate();
    if (delegate)
      this->m_subphrases =
        delegate->subphrases(shared_from_this());
    }
}

  } // model namespace
} // smtk namespace
