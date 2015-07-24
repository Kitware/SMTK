#include "ProcessSet.hpp"

#include <assert.h>

namespace moab {

ProcessSet::ProcessSet()
{
  this->clear();
}

ProcessSet::ProcessSet( const unsigned char* psetbits )
{
  for ( int i = 0; i < SHARED_PROC_BYTES; ++ i )
    this->processes[i] = psetbits[i];
}

ProcessSet::~ProcessSet()
{
}

void ProcessSet::unite( const ProcessSet& other )
{
  for ( int i = 0; i < SHARED_PROC_BYTES; ++ i )
    {
    this->processes[i] |= other.processes[i];
    }
}

void ProcessSet::intersect( const ProcessSet& other )
{
  for ( int i = 0; i < SHARED_PROC_BYTES; ++ i )
    {
    this->processes[i] &= other.processes[i];
    }
}

void ProcessSet::clear()
{
  memset( this->processes, 0, SHARED_PROC_BYTES );
}

/**\brief Add a process to this process set.
  *
  * This does not verify that \a proc is within the range [0,MAX_SHARING_PROCS[ .
  * You are responsible for that.
  */
void ProcessSet::set_process_member( int proc )
{
  int byte = proc / 8;
  int bitmask = 1 << ( proc % 8 );
  this->processes[byte] |= bitmask;
}

/**\brief Add each process in the input vector to this process set.
  *
  * This is a convenience routine that calls set_process_member() for each entry in the vector.
  * This does not verify that \a proc is within the range [0,MAX_SHARING_PROCS[ .
  * You are responsible for that.
  */
void ProcessSet::set_process_members( const std::vector<int>& procs )
{
  for ( std::vector<int>::const_iterator it = procs.begin(); it != procs.end() && *it != -1; ++ it )
    {
    this->set_process_member( *it );
    }
}

/**\brief Retrieve a vector containing processes in this set.
  *
  * @param [in] rank The rank of the local process. This integer will not be included in the output list.
  * @param [out] procs The vector in which the list of sharing processes is listed.
  * @return   True when \a rank is the owning process and false otherwise.
  */
bool ProcessSet::get_process_members( int rank, std::vector<int>& procs )
{
  int i = 0;
  assert( rank >= 0 );
  procs.clear();
  bool rank_owner = false;
  for ( int byte = 0; byte < SHARED_PROC_BYTES; ++ byte )
    {
    i = byte * 8;
    for ( unsigned char val = this->processes[byte]; val; ++ i, val >>= 1 )
      {
      if ( val & 0x1 )
        {
        if ( i != rank )
          {
          //std::cout << " " << i;
          procs.push_back( i );
          }
        else if ( ! procs.size() )
          {
          rank_owner = true;
          }
        }
      }
    }
  for ( i = procs.size(); i < MAX_SHARING_PROCS; ++ i )
    {
    procs.push_back( -1 ); // pad with invalid values
    }
  return rank_owner;
}

bool ProcessSet::is_process_member( int i ) const
{
  int byte = i / 8;
  int bitmask = 1 << ( i % 8 );
  return ( this->processes[byte] & bitmask ) ? true : false;
}

const unsigned char* ProcessSet::data() const
{
  return this->processes;
}

bool ProcessSet::operator < ( const ProcessSet& other ) const
{
  for ( int i = 0; i < SHARED_PROC_BYTES; ++ i )
    {
    if ( this->processes[i] < other.processes[i] )
      return true;
    else if ( this->processes[i] > other.processes[i] )
      return false;
    }
  return false; // equality
}

std::ostream& operator << ( std::ostream& os, const ProcessSet& pset )
{
  for ( int i = 0; i < MAX_SHARING_PROCS; ++ i )
    {
    os << ( pset.is_process_member( i ) ? "1" : "0" );
    }
  return os;
}

} // namespace moab

