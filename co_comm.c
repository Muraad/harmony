#include <stdlib.h>
#include <string.h>

#include "co_comm.h"
#include "co_err.h"

int co_buf_alloc(struct co_buf_t *buf, int size)
{
	if(!buf || size <= 0) return CO_ERR_INVALID_ARG;
	buf->base = malloc(size);
	return 0;
}

int co_buf_free(struct co_buf_t *buf)
{
	if(!buf) return CO_ERR_INVALID_ARG;
	free(buf->base);
	buf->base = NULL;
	return 0;
}

PT_THREAD(co_send_recv(struct pt *pt))
{
}
