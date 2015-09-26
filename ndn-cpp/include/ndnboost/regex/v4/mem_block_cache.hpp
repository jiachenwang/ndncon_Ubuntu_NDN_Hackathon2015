 /*
 * Copyright (c) 2002
 * John Maddock
 *
 * Use, modification and distribution are subject to the 
 * Boost Software License, Version 1.0. (See accompanying file 
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */

 /*
  *   LOCATION:    see http://www.boost.org for most recent version.
  *   FILE         mem_block_cache.hpp
  *   VERSION      see <ndnboost/version.hpp>
  *   DESCRIPTION: memory block cache used by the non-recursive matcher.
  */

#ifndef NDNBOOST_REGEX_V4_MEM_BLOCK_CACHE_HPP
#define NDNBOOST_REGEX_V4_MEM_BLOCK_CACHE_HPP

#include <new>
#ifdef NDNBOOST_HAS_THREADS
#include <ndnboost/regex/pending/static_mutex.hpp>
#endif

#ifdef NDNBOOST_HAS_ABI_HEADERS
#  include NDNBOOST_ABI_PREFIX
#endif

namespace ndnboost{
namespace re_detail{

struct mem_block_node
{
   mem_block_node* next;
};

struct mem_block_cache
{
   // this member has to be statically initialsed:
   mem_block_node* next;
   unsigned cached_blocks;
#ifdef NDNBOOST_HAS_THREADS
   ndnboost::static_mutex mut;
#endif

   ~mem_block_cache()
   {
      while(next)
      {
         mem_block_node* old = next;
         next = next->next;
         ::operator delete(old);
      }
   }
   void* get()
   {
#ifdef NDNBOOST_HAS_THREADS
      ndnboost::static_mutex::scoped_lock g(mut);
#endif
     if(next)
      {
         mem_block_node* result = next;
         next = next->next;
         --cached_blocks;
         return result;
      }
      return ::operator new(NDNBOOST_REGEX_BLOCKSIZE);
   }
   void put(void* p)
   {
#ifdef NDNBOOST_HAS_THREADS
      ndnboost::static_mutex::scoped_lock g(mut);
#endif
      if(cached_blocks >= NDNBOOST_REGEX_MAX_CACHE_BLOCKS)
      {
         ::operator delete(p);
      }
      else
      {
         mem_block_node* old = static_cast<mem_block_node*>(p);
         old->next = next;
         next = old;
         ++cached_blocks;
      }
   }
};

extern mem_block_cache block_cache;

}
} // namespace ndnboost

#ifdef NDNBOOST_HAS_ABI_HEADERS
#  include NDNBOOST_ABI_SUFFIX
#endif

#endif

