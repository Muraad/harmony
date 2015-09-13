#ifndef __CO_COMM_H__
#define __CO_COMM_H__

#include "pt.h"

#define CO_ACCESSOR(type, field) \
	public: \
	void set_##field(const type &o) { field##_ = o; } \
	const type &field() const { return field##_; } \
	type *mutable_##field() { return &field##_; } \
	private: \
	type field##_;

//co_buf_t
struct co_buf_t {
	char *base;
	int len;
};

int co_buf_alloc(struct co_buf_t *buf, int size);
int co_buf_free(struct co_buf_t *buf);

//co_send_recv_t
struct co_send_recv_t {
	struct pt pt;
	struct co_buf_t send_buf;
	struct co_buf_t recv_buf;
	int timeout;
	int error;
	void *data;
};

PT_THREAD(co_send_recv(struct pt *pt));

#endif
