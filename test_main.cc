#include <functional>

#include <cstdio>
#include <unistd.h>
#include "http_conn.h"

#include "uv.h"
#include "misc_util.h"

static uv_thread_t tid_;
static uv_timer_t timer_;
static uv_idle_t idle_;

static void idle_cb(uv_idle_t *handle)
{
	//CLOG("idle callback");
	usleep(50000);
}

static void timer_cb(uv_timer_t *handle)
{
	//CLOG("timer callback");
}

void uv_run(void *arg)
{
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

int main()
{
	uv_loop_init(uv_default_loop());
	//uv_idle_init(uv_default_loop(), &idle_);
	//uv_idle_start(&idle_, idle_cb);
	uv_timer_init(uv_default_loop(), &timer_);
	uv_timer_start(&timer_, timer_cb, 50, 50);

	uv_thread_create(&tid_, uv_run, nullptr);

	auto h = new imcore::HttpConn();

	h->Request("http://www.qq.com/", "GET", "", 
			[=](int code, const std::string &rsp) {
			CLOG("code: %d", code);
			if(code) return;
			CLOG("succ");

			//CLOG("rspbody:\n%s", rsp.c_str());

			});

	while (1) {
		sleep(1);
	}

	return 0;
}
