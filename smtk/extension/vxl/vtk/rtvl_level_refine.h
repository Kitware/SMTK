//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

/* Copyright 2007-2009 Brad King, Chuck Stewart
   Distributed under the Boost Software License, Version 1.0.
   (See accompanying file rtvl_license_1_0.txt or copy at
   http://www.boost.org/LICENSE_1_0.txt) */
#ifndef rtvl_level_refine_h
#define rtvl_level_refine_h

template <unsigned int N>
class rtvl_level_refine_internal;
template <unsigned int N>
class rtvl_tokens;

template <unsigned int N>
class rtvl_level_refine
{
public:
  rtvl_level_refine(unsigned int num_points, double* points, double* bounds);
  ~rtvl_level_refine();

  /** Set the fraction of the scale to use as the masking sphere size
      during downsampling.  */
  void set_mask_size(double f);

  /** Compute the initial scale, which, if called (may instead "set") must done
      only "immediately" after construction.  */
  double compute_scale() const;

  /** Set the initial scale to a user specified value.  Only do before starting
      the refine process.  Must be > 0. */
  void set_scale(double scale);

  /** Build the quad tree.  Don't expect this to be the final resting place for
      this call, since it is currently required! */
  void build_tree();

  /** Compute the # of chunks based on the memory limit (in KB) and a minimum
      of chunks */
  int initialize_refine_level(unsigned int memory_limit, int requested_min_depth) const;

  int refine_next_block(const char* base_filename);

  /** Get refined tokens for the current scale.  */
  void get_tokens(int level, double* bounds, rtvl_tokens<N>& tokens) const;

  /** Get scale of the specified level  */
  double get_level_scale(int level);

  /** Get number of votes cast during token refinement.  */
  unsigned int get_vote_count() const;

  /** Move to the next-larger scale.  */
  bool next_scale();

  unsigned int estimate_refine_levels();

private:
  friend class rtvl_level_refine_internal<N>;
  typedef rtvl_level_refine_internal<N> internal_type;
  internal_type* internal_;
};

#endif
