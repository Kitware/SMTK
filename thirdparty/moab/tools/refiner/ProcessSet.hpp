/*
 * MOAB, a Mesh-Oriented datABase, is a software component for creating,
 * storing and accessing finite element mesh data.
 * 
 * Copyright 2007 Sandia Corporation.  Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 */

#ifndef MOAB_PROCESS_SET_HPP
#define MOAB_PROCESS_SET_HPP

#include "moab/Types.hpp"
#include "moab/ParallelComm.hpp"

#include <iostream>
#include <vector>

namespace moab {

/**\brief Represent a set of processes using a bit vector.
  *
  * This is used by the mesh refiner when determining where to record
  * split vertices so that labeling can be inferred across process
  * boundaries without communicating anything other than the number of
  * entities in a given partition.
  */
class ProcessSet
{
public:
  enum
    {
    SHARED_PROC_BYTES = (MAX_SHARING_PROCS / 8 + (MAX_SHARING_PROCS % 8 ? 1 : 0))
    };

  ProcessSet();
  ProcessSet( const unsigned char* psetbits );
  ~ProcessSet();

  void unite( const ProcessSet& other );
  void intersect( const ProcessSet& other );

  void clear();

  void set_process_member( int i );
  void set_process_members( const std::vector<int>& procs );

  bool get_process_members( int rank, std::vector<int>& procs );
  bool is_process_member( int i ) const;

  const unsigned char* data() const;

  bool operator < ( const ProcessSet& other ) const;

  friend std::ostream& operator << ( std::ostream& os, const ProcessSet& pset );

protected:
  unsigned char processes[SHARED_PROC_BYTES];
};

} // namespace moab

#endif /* MOAB_PROCESS_SET_HPP */
