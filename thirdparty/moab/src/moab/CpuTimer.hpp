#ifndef CPUTIMER_HPP
#define CPUTIMER_HPP

#include "moab/MOABConfig.h"
#ifdef MOAB_HAVE_MPI
#  include "moab_mpi.h"
#else
#  if defined(_MSC_VER)
#    include <time.h>
#  else
#    include <sys/resource.h>
#  endif
#endif

namespace moab 
{
    
class CpuTimer {
private:
  double tAtBirth, tAtLast;
  double mAtBirth, mAtLast;
  long rssAtBirth, rssAtLast;

  double runtime();
  long runmem();
  
public:
  CpuTimer() : tAtBirth(runtime()), tAtLast(tAtBirth) {}
  double time_since_birth() { return (tAtLast = runtime()) - tAtBirth; };
  double time_elapsed() { double tmp = tAtLast; return (tAtLast = runtime()) - tmp; }
  long mem_since_birth() {return (mAtLast=runmem()) - mAtBirth;}
  long mem_elapsed() {long tmp = mAtLast; return (mAtLast=runmem()) - tmp;}
};

    inline double CpuTimer::runtime() 
    {
#if defined(_MSC_VER) || defined(__MINGW32__)
      return (double)clock() / CLOCKS_PER_SEC;
#elif defined(MOAB_HAVE_MPI)
      return MPI_Wtime();
#else      
      struct rusage r_usage;
      getrusage(RUSAGE_SELF, &r_usage);
      double utime = (double)r_usage.ru_utime.tv_sec +
          ((double)r_usage.ru_utime.tv_usec/1.e6);
      double stime = (double)r_usage.ru_stime.tv_sec +
          ((double)r_usage.ru_stime.tv_usec/1.e6);
      return utime + stime;
#endif
    }

    inline long CpuTimer::runmem() 
    {
#if defined(_MSC_VER) || defined(__MINGW32__)
      return 0;
#elif defined(MOAB_HAVE_MPI)
      return 0;
#else
      struct rusage r_usage;
      getrusage(RUSAGE_SELF, &r_usage);
      mAtLast = r_usage.ru_maxrss;
      return mAtLast;
#endif
    }

}

#endif
