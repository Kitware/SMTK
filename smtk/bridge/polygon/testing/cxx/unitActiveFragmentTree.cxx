//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/polygon/internal/ActiveFragmentTree.h"
#include "smtk/bridge/polygon/internal/Fragment.h"

#include "smtk/model/testing/cxx/helpers.h"

using namespace smtk::bridge::polygon;

namespace {

void dump(ActiveFragmentTree& activeFragments)
{
  ActiveFragmentTree::const_iterator ait;
  for (ait = activeFragments.begin(); ait != activeFragments.end(); ++ait)
    {
    std::cerr << "  " << *ait << "\n";
    }
  std::cerr << "--\n";
}

// Create a test dataset with known ordering for active segments.
template<typename F, typename P, typename A>
void setup(F& fragments, P& posn, A& activeFragments)
{
  // NB: See doc/unitActiveFragmentTree.svg in the SMTK test data repo
  //     for an illustration of this dataset. Be sure to update it if
  //     you change the fcoords array below!
  int fcoords[][4] = {
			{   0,    0,  240,    0 },
			{   0,   30,   90,  120 },
			{   0,  120,   90,  120 },
			{  50,  100,  120,  170 },
			{  50,  100,  120,   30 },
			{ 120,  170,  190,  100 },
			{ 110,   90,  110,  110 },
			{ 110,   90,  130,   90 },
			{ 110,  110,  130,  110 },
			{ 130,   90,  130,  110 },
			{ 160,    0,  160,  100 },
			{ 160,  100,  220,  160 },
			{ 120,   30,  190,  100 }
	};

  unsigned nf = sizeof(fcoords) / sizeof(fcoords[0]);
  fragments.reserve(nf);
  EdgeFragment entry;
  for (unsigned i = 0; i < nf; ++i)
    {
    entry.lo() = internal::Point(fcoords[i][0], fcoords[i][1]);
    entry.hi() = internal::Point(fcoords[i][2], fcoords[i][3]);
    entry.m_segment = static_cast<int>(i);
    fragments.push_back(entry);
    }

  posn.advance(fragments[0].lo());
}

// Verify that inserting fragments preserves the proper ordering.
template<typename F, typename P, typename A>
bool verify_insertion(F& fragments, P& posn, A& activeFragments, int imax)
{
  if (imax < 0)
    {
    std::cout << "+++ verify insertion +++\n";
    }
  // Insert fragments in a known-to-be-good left->right traversal order
  // and verify that the resulting tree at each step produces the
  // proper bottom-to-top ordering.
  const long long insertionsRemovals[] = {
    0,
    1,
    2,
    4, 3,
    -1, -2,
    7, 6,
    -6, 8,
    -4, 12,
    -3, 5,
    -7, 9,
    -8, -9,
    10,
    -10, 11,
    -12, -5,
    -11,
    0
  };
  long long actives[] = {
    0, 1, 4, 3, 2, -1,
    0, 4, 7, 6, 3, -1,
    0, 4, 7, 8, 3, -1,
    0, 12, 7, 8, 3, -1,
    0, 12, 7, 8, 5, -1,
    0, 12, 9, 8, 5, -1,
    0, 10, 12, 5, -1,
    0, 12, 11, 5, -1,
    0, 11, -1
  };

  int i = 0;
  int a = 0;
  int lastA = 0;
  long long irid = insertionsRemovals[i++];
  internal::Point pt;
  activeFragments.insert(static_cast<FragmentId>(irid)); // Do this before loop since insertionsRemovals[0] == 0.

  while (insertionsRemovals[i] != 0)
    {
    // Make some fragments active
    for (; (irid = insertionsRemovals[i]) > 0; ++i)
      {
      if (imax > 0 && i == imax)
        {
        return true;
        }
      else if (imax < 0)
        {
        std::cout << "Inserting " << irid << " at " << fragments[irid].lo().x() << " " << fragments[irid].lo().y() << "\n";
        }
      posn.advance(fragments[irid].lo());
      activeFragments.insert(static_cast<FragmentId>(irid));
      if (imax < 0)
        {
        dump(activeFragments);
        }
      }
    // Check that the active fragments match the entries in "actives."
    ActiveFragmentTree::const_iterator ait = activeFragments.begin();
    for (; actives[a] != -1; ++a, ++ait)
      {
      if (ait == activeFragments.end())
        {
        std::cerr << "ERROR: Active fragments too short; expected " << actives[a] << " at " << a << "\n";
        dump(activeFragments);
        return false;
        }
      else if (*ait != actives[a])
        {
        std::cerr << "ERROR: Active fragment " << *ait << " when " << actives[a] << " expected at " << a << "\n";
        dump(activeFragments);
        return false;
        }
      }
    if (ait != activeFragments.end())
      {
      std::cerr << "ERROR: " << (activeFragments.size() - a + lastA) << " more active fragments than expected\n";
      dump(activeFragments);
      return false;
      }
    ++a; // Skip past -1 record terminator
    lastA = a;

    // Remove some fragments
    for (; (irid = -insertionsRemovals[i]) > 0; ++i)
      {
      if (imax > 0 && i == imax)
        {
        return true;
        }
      else if (imax < 0)
        {
        std::cout << "Removing  " << irid << " at " << fragments[irid].lo().x() << " " << fragments[irid].lo().y() << "\n";
        }
      activeFragments.erase(static_cast<FragmentId>(irid));
      pt = fragments[irid].hi();
      }
    posn.advance(pt);
    }
  activeFragments.erase(0); // Should be the last edge out
  if (!activeFragments.empty())
    {
    std::cerr << "ERROR: Ended with " << activeFragments.size() << " active fragments\n";
    return false;
    }
  if (imax < 0)
    {
    std::cout << "--- verify insertion ---\n";
    }
  return true;
}

template<typename T>
bool boundingFragmentsShouldBe(const T& activeFragments, const internal::Point& pt, FragmentId a, FragmentId b)
{
  std::pair<FragmentId, FragmentId> bds;
  bds = activeFragments.boundingFragments(pt);
  std::cout << "  (" << pt.x() << " " << pt.y() << ") bounded by " << bds.first << " " << bds.second << "\n";
  if (bds.first != a || bds.second != b)
    {
    std::cerr << "ERROR: Expected bounds to be " << a << " " << b << "\n";
    return false;
    }
  return true;
}

template<typename F, typename P, typename A>
bool verify_bounds(F& fragments, P& posn, A& activeFragments)
{
  std::cout << "+++ verify bounds +++\n";
  bool ok = true;
  // Load up some active segments:
  posn.position() = fragments[0].lo();
  verify_insertion(fragments, posn, activeFragments, 5);

  // Find which active segments bound some test points:
  ok &= boundingFragmentsShouldBe(activeFragments, internal::Point(55,  -5), -1,  0);
  ok &= boundingFragmentsShouldBe(activeFragments, internal::Point(55,   0),  0,  1);
  ok &= boundingFragmentsShouldBe(activeFragments, internal::Point(55,  50),  0,  1);
  ok &= boundingFragmentsShouldBe(activeFragments, internal::Point(56,  90),  1,  4);
  ok &= boundingFragmentsShouldBe(activeFragments, internal::Point(56, 100),  4,  3);
  ok &= boundingFragmentsShouldBe(activeFragments, internal::Point(58, 110),  3,  2);
  ok &= boundingFragmentsShouldBe(activeFragments, internal::Point(58, 120),  3,  2);
  ok &= boundingFragmentsShouldBe(activeFragments, internal::Point(58, 150),  2, -1);

  // Load up some different active segments:
  activeFragments.clear();
  posn.position() = fragments[0].lo();
  verify_insertion(fragments, posn, activeFragments, 7);

  // Find which active segments bound some test points:

  //  Clean up and clear out.
  activeFragments.clear();
  std::cout << "--- verify bounds ---\n";
  return ok;
}

} // dummy namespace

int unitActiveFragmentTree(int argc, char* argv[])
{
  internal::Point startPt(-10,0);
  FragmentArray fragments;
  SweeplinePosition posn(startPt);
  ActiveFragmentTree activeFragments(fragments, posn);

  bool ok = true;
  setup(fragments, posn, activeFragments);
  ok &= verify_insertion(fragments, posn, activeFragments, -1);
  ok &= verify_bounds(fragments, posn, activeFragments);

  return ok ? 0 : -1;
}
