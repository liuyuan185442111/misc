#include "performance.h"

namespace lcore {

Performance& Performance::operator -= (const Performance &rhs)
{
	_page_alloc                   -= rhs._page_alloc;
	_page_free                    -= rhs._page_free;
	_find_key                     -= rhs._find_key;
	_insert                       -= rhs._insert;
	_insert_replace               -= rhs._insert_replace;
	_insert_reject                -= rhs._insert_reject;
	_remove_found                 -= rhs._remove_found;
	_remove_not_found             -= rhs._remove_not_found;
	_split                        -= rhs._split;
	_insert_leaf_adjust_left      -= rhs._insert_leaf_adjust_left;
	_insert_leaf_adjust_right     -= rhs._insert_leaf_adjust_right;
	_insert_internal_adjust_left  -= rhs._insert_internal_adjust_left;
	_insert_internal_adjust_right -= rhs._insert_internal_adjust_right;
	_remove_internal              -= rhs._remove_internal;
	_l_shrink_leaf                -= rhs._l_shrink_leaf;
	_r_shrink_leaf                -= rhs._r_shrink_leaf;
	_l_shrink_internal            -= rhs._l_shrink_internal;
	_r_shrink_internal            -= rhs._r_shrink_internal;
	_l_shrink_leaf_adjust         -= rhs._l_shrink_leaf_adjust;
	_r_shrink_leaf_adjust         -= rhs._r_shrink_leaf_adjust;
	_l_shrink_internal_adjust     -= rhs._l_shrink_internal_adjust;
	_r_shrink_internal_adjust     -= rhs._r_shrink_internal_adjust;
	_merge_sibling                -= rhs._merge_sibling;
	_merge_sibling_adjust         -= rhs._merge_sibling_adjust;
	_merge_sibling_nest           -= rhs._merge_sibling_nest;
	_page_read                    -= rhs._page_read;
	_page_write                   -= rhs._page_write;
	_page_sync                    -= rhs._page_sync;
	return *this;
}

//(#): maybe not very exact
//(*): available while running
void Performance::dump(float e, FILE *fp) const
{
	fprintf(fp, "page_alloc                   : %f\n", _page_alloc / e                  );
	fprintf(fp, "page_free                    : %f\n", _page_free / e                   );
	fprintf(fp, "find_key(#)                  : %f\n", _find_key / e                    );
	fprintf(fp, "insert                       : %f\n", _insert / e                      );
	fprintf(fp, "insert_replace               : %f\n", _insert_replace / e              );
	fprintf(fp, "insert_reject                : %f\n", _insert_reject / e               );
	fprintf(fp, "insert_leaf_adjust_left      : %f\n", _insert_leaf_adjust_left / e     );
	fprintf(fp, "insert_leaf_adjust_right     : %f\n", _insert_leaf_adjust_right / e    );
	fprintf(fp, "insert_internal_adjust_left  : %f\n", _insert_internal_adjust_left / e );
	fprintf(fp, "insert_internal_adjust_right : %f\n", _insert_internal_adjust_right / e);
	fprintf(fp, "split                        : %f\n", _split / e                       );
	fprintf(fp, "remove_not_found(#)          : %f\n", _remove_not_found / e            );
	fprintf(fp, "remove_found(leaf)           : %f\n", _remove_found / e                );
	fprintf(fp, "remove_internal              : %f\n", _remove_internal / e             );
	fprintf(fp, "l_shrink_leaf                : %f\n", _l_shrink_leaf / e               );
	fprintf(fp, "r_shrink_leaf                : %f\n", _r_shrink_leaf / e               );
	fprintf(fp, "l_shrink_internal            : %f\n", _l_shrink_internal / e           );
	fprintf(fp, "r_shrink_internal            : %f\n", _r_shrink_internal / e           );
	fprintf(fp, "l_shrink_leaf_adjust         : %f\n", _l_shrink_leaf_adjust / e        );
	fprintf(fp, "r_shrink_leaf_adjust         : %f\n", _r_shrink_leaf_adjust / e        );
	fprintf(fp, "l_shrink_internal_adjust     : %f\n", _l_shrink_internal_adjust / e    );
	fprintf(fp, "r_shrink_internal_adjust     : %f\n", _r_shrink_internal_adjust / e    );
	fprintf(fp, "merge_sibling                : %f\n", _merge_sibling / e               );
	fprintf(fp, "merge_sibling_adjust         : %f\n", _merge_sibling_adjust / e        );
	fprintf(fp, "merge_sibling_nest           : %f\n", _merge_sibling_nest / e          );

	fprintf(fp, "page_read(*)                 : %f\n", _page_read / e                   );
	fprintf(fp, "page_write(*)                : %f\n", _page_write / e                  );
	fprintf(fp, "page_sync(*)                 : %f\n", _page_sync / e                   );
	fprintf(fp, "cache_peak(*)                : %u\n", _cache_peak                      );
	fprintf(fp, "dirty_peak(*)                : %u\n", _dirty_peak                      );
	fprintf(fp, "cache_high(#)                : %u\n", _cache_high                      );
	fprintf(fp, "cache_low(#)                 : %u\n", _cache_low                       );

	fprintf(fp, "record_count(insert-remove_found) = %d\n", (int)record_count());
}

} //namespace lcore
