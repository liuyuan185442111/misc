#ifndef _L_PERFORMANCE_H
#define _L_PERFORMANCE_H

#include <stdio.h>

namespace lcore {

class Performance
{
	typedef unsigned int size32_t;
	friend class Page;
	friend class PageCache;
	size32_t _page_alloc;
	size32_t _page_free;
	size32_t _find_key;
	size32_t _insert;
	size32_t _insert_replace;
	size32_t _insert_reject;
	size32_t _split;
	size32_t _insert_leaf_adjust_left;
	size32_t _insert_leaf_adjust_right;
	size32_t _insert_internal_adjust_left;
	size32_t _insert_internal_adjust_right;
	size32_t _remove_found;
	size32_t _remove_not_found;
	size32_t _remove_internal;
	size32_t _l_shrink_leaf;
	size32_t _l_shrink_internal;
	size32_t _r_shrink_leaf;
	size32_t _r_shrink_internal;
	size32_t _l_shrink_leaf_adjust;
	size32_t _l_shrink_internal_adjust;
	size32_t _r_shrink_leaf_adjust;
	size32_t _r_shrink_internal_adjust;
	size32_t _merge_sibling;
	size32_t _merge_sibling_adjust;
	size32_t _merge_sibling_nest;
	size32_t _page_read;
	size32_t _page_write;
	size32_t _page_sync;
	size32_t _cache_high;
	size32_t _cache_low;
	size32_t _cache_peak;
	size32_t _dirty_peak;

	void page_alloc()                   { _page_alloc++;                   }
	void page_free()                    { _page_free++;                    }
	void find_key()                     { _find_key++;                     }
	void insert()                       { _insert++;                       }
	void insert_replace()               { _insert_replace++;               }
	void insert_reject()                { _insert_reject++;                }
	void remove_found()                 { _remove_found++;                 }
	void remove_not_found()             { _remove_not_found++;             }
	void split()                        { _split++;                        }
	void insert_leaf_adjust_left()      { _insert_leaf_adjust_left++;      }
	void insert_leaf_adjust_right()     { _insert_leaf_adjust_right++;     }
	void insert_internal_adjust_left()  { _insert_internal_adjust_left++;  }
	void insert_internal_adjust_right() { _insert_internal_adjust_right++; }
	void remove_internal()              { _remove_internal++;              }
	void l_shrink_leaf()                { _l_shrink_leaf++;                }
	void r_shrink_leaf()                { _r_shrink_leaf++;                }
	void l_shrink_internal()            { _l_shrink_internal++;            }
	void r_shrink_internal()            { _r_shrink_internal++;            }
	void l_shrink_leaf_adjust()         { _l_shrink_leaf_adjust++;         }
	void r_shrink_leaf_adjust()         { _r_shrink_leaf_adjust++;         }
	void l_shrink_internal_adjust()     { _l_shrink_internal_adjust++;     }
	void r_shrink_internal_adjust()     { _r_shrink_internal_adjust++;     }
	void merge_sibling()                { _merge_sibling++;                }
	void merge_sibling_adjust()         { _merge_sibling_adjust++;         }
	void merge_sibling_nest()           { _merge_sibling_nest++;           }
	void set_page_read (size_t c)       { _page_read  = c;                 }
	void set_page_write(size_t c)       { _page_write = c;                 }
	void set_page_sync (size_t c)       { _page_sync  = c;                 }
	void set_cache_peak(size_t peak)    { if(peak > _cache_peak) _cache_peak = peak; }
	void set_dirty_peak(size_t peak)    { if(peak > _dirty_peak) _dirty_peak = peak; }
	void reset_peak()                   { _cache_peak = 0, _dirty_peak = 0; }
	void set_cache_high(size_t cache_high) { _cache_high = cache_high; }
	void set_cache_low (size_t cache_low ) { _cache_low  = cache_low ; }

public:
	Performance& operator -= (const Performance &rhs);
	size_t record_count() const { return _insert - _remove_found; }
	void dump(float e = 1.0, FILE *fp = stdout) const;
};

} //namespace lcore

#endif
