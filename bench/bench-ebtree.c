/* ==========================================================================
 * ebtree from haproxy.
 * ==========================================================================
 */
#include <stdlib.h>
#include <stdio.h>

#include "timeout.h"
#include "bench.h"
#include "ebtree.h"
#include "eb32tree.h"

struct ebtimeout {
	struct eb32_node node;/* ebtree node used to hold the timer in the timer tree */
	timeout_t expires;
};

struct ebtimeouts {
	timeout_t curtime;
	struct eb_root root;
};

static int timeoutcmp(struct ebtimeout *a, struct ebtimeout *b) {
	if (a->expires < b->expires) {
		return -1;
	} else if (a->expires > b->expires) {
		return 1;
	} else if (a < b) {
		return -1;
	} else if (a > b) {
		return 1;
	} else {
		return 0;
	}
} /* timeoutcmp() */

static void *init(struct timeout *timeout, size_t count, int verbose) {
	struct ebtimeouts *T;
	size_t i;

	T = malloc(sizeof *T);
	T->curtime = 0;
	T->root = EB_ROOT;

	for (i = 0; i < count; i++) {
		struct ebtimeout *to = (void *)&timeout[i];
		to->expires = 0;
	}

	return T;
} /* init() */


static void add(void *ctx, struct timeout *_to, timeout_t expires) {
	struct ebtimeouts *T = ctx;
	struct ebtimeout *to = (void *)_to;

	to->expires = T->curtime + expires;
	to->node.key = to->expires;
	eb32_insert(&T->root, &to->node);
	//fprintf(stdout, "insert a timer with expires %lu\n", to->expires);
} /* add() */


static void del(void *ctx, struct timeout *_to) {
	//struct ebtimeouts *T = ctx;
	struct ebtimeout *to = (void *)_to;

	eb32_delete(&to->node);
	to->expires = 0;
} /* del() */


static struct timeout *get(void *ctx) {
	struct ebtimeouts *T = ctx;
	struct ebtimeout *to;

	struct eb32_node *n = eb32_lookup_le(&T->root, T->curtime);

	if (n) {
		to = container_of(n, struct ebtimeout, node);
		//fprintf(stdout, "current time: %lu should be greater than expires: %lu\n", T->curtime, to->expires);
		to->expires = 0;
		eb32_delete(&to->node);

		return (void *)to;
	}
	return NULL;
} /* get() */

static void update(void *ctx, timeout_t ts) {
	struct ebtimeouts *T = ctx;
	T->curtime = ts;
} /* update() */


static void check(void *ctx) {
	return;
} /* check() */


static int empty(void *ctx) {
	struct ebtimeouts *T = ctx;

	return eb_is_empty(&T->root);
} /* empty() */


static void destroy(void *ctx) {
	free(ctx);
	return;
} /* destroy() */


const struct benchops benchops = {
	.init    = &init,
	.add     = &add,
	.del     = &del,
	.get     = &get,
	.update  = &update,
	.check   = &check,
	.empty   = &empty,
	.destroy = &destroy,
};

