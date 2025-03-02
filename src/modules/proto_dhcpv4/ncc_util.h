#pragma once
/*
 *	ncc_util.h
 */

#include <freeradius-devel/server/base.h>


#define NCC_ENDPOINT_STRLEN       (FR_IPADDR_STRLEN + 5)
#define NCC_ETHADDR_STRLEN        (17 + 1)
#define NCC_UINT32_STRLEN         (10 + 1)
#define NCC_UINT64_STRLEN         (20 + 1)
#define NCC_TIME_STRLEN           (15 + 1)
#define NCC_DATETIME_STRLEN       (19 + 1)

#define NCC_DATE_FMT              "%Y-%m-%d"
#define NCC_TIME_FMT              "%H:%M:%S"
#define NCC_DATETIME_FMT          NCC_DATE_FMT" "NCC_TIME_FMT


/*
 *	Using rad_assert defined in include/rad_assert.h
 */
#define ncc_assert rad_assert


/*
 *	Trace / logging.
 */
extern FILE *ncc_log_fp;
extern int ncc_debug_lvl;
#define NCC_LOG_ENABLED          (ncc_log_fp)
#define NCC_DEBUG_ENABLED(_p)    (ncc_log_fp && ncc_debug_lvl >= _p)
#define NCC_DEBUG(_p, _f, ...)   do { if (NCC_DEBUG_ENABLED(_p)) ncc_log_dev_printf(__FILE__, __LINE__, _f, ## __VA_ARGS__); } while(0)
#define NCC_LOG(_f, ...)         do { if (NCC_LOG_ENABLED) ncc_printf_log(_f "\n", ## __VA_ARGS__); } while(0)


/*	After a call to snprintf and similar functions, check if we have enough remaining buffer space.
 *
 *	These functions return the number of characters printed (excluding the null byte used to end output to strings).
 *	If the output was truncated due to this limit then the return value is the number of characters (excluding the
 *	terminating null byte) which would have been written to the final string if enough space had been available.
 *	Thus, a return value of size or more means that the output was truncated.
 */

/* Push error about insufficient buffer size. */
#define ERR_BUFFER_SIZE(_need, _size, _info) \
	fr_strerror_printf("%s buffer too small (needed: %zu bytes, have: %zu)", _info, (size_t)(_need), (size_t)(_size))

/* Check buffer size, if insufficient: push error and return.
 * _size is the buffer size, _need what we need (including the terminating '\0' if relevant)
 */
#define CHECK_BUFFER_SIZE(_ret, _need, _size, _info) \
	if (_size < _need) { \
		ERR_BUFFER_SIZE(_need, _size, _info); \
		return _ret; \
	}

/* Check if we have enough remaining buffer space. If not push an error and return NULL.
 * Otherwise, update the current char pointer.
 */
#define ERR_IF_TRUNCATED(_p, _ret, _max) \
do { \
	if (is_truncated(_ret, _max)) { \
		ERR_BUFFER_SIZE(_ret, _max, ""); \
		return NULL; \
	} \
	_p += _ret; \
} while (0)


/* Check that endpoint is not undefined. */
#define is_ipaddr_defined(_x) (_x.af != AF_UNSPEC)
#define is_endpoint_defined(_e) (is_ipaddr_defined(_e.ipaddr) && _e.port)


/* Verify that a VP is "data" (i.e. not "xlat"). */
#define IS_VP_DATA(_vp) (_vp && _vp->type == VT_DATA)


typedef struct ncc_list_item ncc_list_item_t;

/*
 *	Chained list of data elements.
 */
typedef struct ncc_list {
	ncc_list_item_t *head;
	ncc_list_item_t *tail;
	uint32_t size;
} ncc_list_t;

/*
 *	Chained list item.
 */
struct ncc_list_item {
	ncc_list_t *list;       //!< The list to which this entry belongs (NULL for an unchained entry).
	ncc_list_item_t *prev;
	ncc_list_item_t *next;

	void *data;             //!< User-specific item data.
};

/*
 *	Dynamic array of strings, reallocated as needed.
 */
typedef struct ncc_str_array {
	char **strings;
	uint32_t size;
} ncc_str_array_t;

/*
 *	Transport endpoint (IP address, port).
 */
typedef struct ncc_endpoint {
	fr_ipaddr_t ipaddr;
	uint16_t port;
} ncc_endpoint_t;

/*
 *	List of endpoints (used in round-robin fashion).
 */
typedef struct ncc_endpoint_list {
	ncc_endpoint_t *eps;   //<! List of endpoints.
	uint32_t num;          //<! Number of endpoints in the list.
	uint32_t next;         //<! Number of next endpoint to be used.
} ncc_endpoint_list_t;


/* Get visibility on fr_event_timer_t opaque struct (fr_event_timer is defined in lib/util/event.c) */
typedef struct ncc_fr_event_timer {
	fr_event_list_t		*el;			//!< because talloc_parent() is O(N) in number of objects
	fr_time_t		when;			//!< When this timer should fire.
	/* We don't need anything beyond that. */
} ncc_fr_event_timer_t;

/* Get visibility on fr_event_list_t opaque struct (fr_event_list is defined in lib/util/event.c) */
typedef struct ncc_fr_event_list {
	fr_heap_t		*times;			//!< of timer events to be executed.
	/* We don't need anything beyond that. */
} ncc_fr_event_list_t;


int ncc_fr_event_timer_peek(fr_event_list_t *fr_el, fr_time_t *when);

void ncc_log_init(FILE *log_fp, int debug_lvl, int debug_dev);
void ncc_printf_log(char const *fmt, ...);
void ncc_log_dev_printf(char const *file, int line, char const *fmt, ...);

VALUE_PAIR *ncc_pair_find_by_da(VALUE_PAIR *head, fr_dict_attr_t const *da);
VALUE_PAIR *ncc_pair_create(TALLOC_CTX *ctx, VALUE_PAIR **vps,
			                unsigned int attribute, unsigned int vendor);
VALUE_PAIR *ncc_pair_create_by_da(TALLOC_CTX *ctx, VALUE_PAIR **vps, fr_dict_attr_t const *da);
int ncc_pair_copy_value(VALUE_PAIR *to, VALUE_PAIR *from);
int ncc_pair_value_from_str(VALUE_PAIR *vp, char const *value);
VALUE_PAIR *ncc_pair_copy(TALLOC_CTX *ctx, VALUE_PAIR const *vp);
int ncc_pair_list_copy(TALLOC_CTX *ctx, VALUE_PAIR **to, VALUE_PAIR *from);
VALUE_PAIR *ncc_pair_list_append(TALLOC_CTX *ctx, VALUE_PAIR **to, VALUE_PAIR *from);
void ncc_pair_list_fprint(FILE *fp, VALUE_PAIR *vps);

char *ncc_endpoint_sprint(char *out, ncc_endpoint_t *ep);
char *ncc_ether_addr_sprint(char *out, const uint8_t *addr);
char *ncc_delta_time_sprint(char *out, struct timeval *from, struct timeval *when, uint8_t decimals);
char *ncc_fr_delta_time_sprint(char *out, fr_time_t *from, fr_time_t *when, uint8_t decimals);
char *ncc_absolute_time_sprint(char *out, bool with_date);

int ncc_host_addr_resolve(ncc_endpoint_t *host_ep, char const *host_arg);

double ncc_timeval_to_float(struct timeval *in);
int ncc_float_to_timeval(struct timeval *tv, double in);
double ncc_fr_time_to_float(fr_time_delta_t in);
fr_time_t ncc_float_to_fr_time(double in);
bool ncc_str_to_float(double *out, char const *in, bool allow_negative);
bool ncc_str_to_uint32(uint32_t *out, char const *in);
size_t ncc_str_trim(char *out, char const *in, size_t inlen);

void ncc_list_add(ncc_list_t *list, ncc_list_item_t *entry);
ncc_list_item_t *ncc_list_item_draw(ncc_list_item_t *entry);
ncc_list_item_t *ncc_list_get_head(ncc_list_t *list);
ncc_list_item_t *ncc_list_index(ncc_list_t *list, uint32_t index);

#define NCC_LIST_ENQUEUE(_l, _e) ncc_list_add(_l, (ncc_list_item_t *)_e);
#define NCC_LIST_DEQUEUE(_l) (void *)ncc_list_get_head(_l);
#define NCC_LIST_INDEX(_l, _i) (void *)ncc_list_index(_l, _i);
#define NCC_LIST_DRAW(_e) (void *)ncc_list_item_draw((ncc_list_item_t *)_e);

ncc_endpoint_t *ncc_ep_list_add(TALLOC_CTX *ctx, ncc_endpoint_list_t *ep_list, char *addr, ncc_endpoint_t *default_ep);
ncc_endpoint_t *ncc_ep_list_get_next(ncc_endpoint_list_t *ep_list);
char *ncc_ep_list_snprint(char *out, size_t outlen, ncc_endpoint_list_t *ep_list);

bool ncc_stdin_peek();

void ncc_str_array_alloc(TALLOC_CTX *ctx, ncc_str_array_t **pt_array);
uint32_t ncc_str_array_add(TALLOC_CTX *ctx, ncc_str_array_t **pt_array, char *value);
uint32_t ncc_str_array_index(TALLOC_CTX *ctx, ncc_str_array_t **pt_array, char *value);


/* This is now in protocol/radius/list.h - which we might not want to depend on, so... */
#define fr_packet2myptr(TYPE, MEMBER, PTR) (TYPE *) (((char *)PTR) - offsetof(TYPE, MEMBER))

/* Same as is_integer, but allow to work on a given length. */
static inline bool is_integer_n(char const *value, ssize_t len)
{
	if (*value == '\0' || len == 0) return false;

	char const *p = value;
	while (*p) {
		if (!isdigit(*p)) return false;

		if (len > 0 && (p - value + 1 >= len)) break;
		p++;
	}

	return true;
}

/* talloc_realloc doesn't zero-initialize the new memory. */
#define TALLOC_REALLOC_ZERO(_ctx, _ptr, _type, _count_pre, _count) \
{ \
	_ptr = talloc_realloc(_ctx, _ptr, _type, _count); \
	if (_count > _count_pre) { \
		memset(&_ptr[_count_pre], 0, sizeof(_type) * (_count - _count_pre)); \
	} \
}

