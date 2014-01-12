#include <uwsgi.h>
#include <sys/eventfd.h>

/*

alarm = oom log:OUT OF MEMORY for
alarm-eventfd = oom /run/cgroup/cgroup.event_control /run/cgroup/memory.usage_in_bytes 100000000

*/

static struct uwsgi_eventfd {
	struct uwsgi_string_list *alarms;
} uefd;

static struct uwsgi_option eventfd_options[] = {
	{ "alarm-eventfd", required_argument, 0, "raise an alarm every time the eventfd() object is ready, syntax: <alarm> <control_file> <subject_file> [extra]", uwsgi_opt_add_string_list, &uefd.alarms, UWSGI_OPT_MASTER},
	UWSGI_END_OF_OPTIONS
};

static int uwsgi_eventfd_init() {

	struct uwsgi_string_list *usl = NULL;
	uwsgi_foreach(usl, uefd.alarms) {
		size_t rlen = 0;
		char **argv = uwsgi_split_quoted(usl->value, usl->len, " \t", &rlen); 
		if (rlen < 3) {
			uwsgi_log("invalid alarm-eventfd syntax, must be <alarm> <control_file> <subject_file> [extra]\n");
			exit(1);
		}
		int fd = eventfd(0, EFD_CLOEXEC|EFD_NONBLOCK);
		if (fd < 0) {
			uwsgi_error("uwsgi_eventfd_init()/eventfd()");
			exit(1);
		}

		int subject_fd = open(argv[2], O_RDONLY);
		if (subject_fd < 0) {
			uwsgi_error_open(argv[2]);
			exit(1);
		}

		int control_fd = open(argv[1], O_WRONLY);
		if (control_fd < 0) {
			uwsgi_error_open(argv[1]);
			exit(1);
		}

		// allocate space for at least 2 integers and a space
		size_t len = 13 + 13 + 1;
		if (rlen > 3) {
			len += strlen(argv[3]);
		}
		char *buf = uwsgi_malloc(len);
		int ret = -1;

		if (rlen > 3) {
			ret = snprintf(buf, len, "%d %d %s", fd, subject_fd, argv[3]);
		}
		else {
			ret = snprintf(buf, len, "%d %d", fd, subject_fd);
		}

		if (ret <= 0 || ret >= (int) len) {
			uwsgi_log("uwsgi_eventfd_init() -> snprintf() fatal error\n");
			exit(1);
		}

		if (write(control_fd, buf, ret) != ret) {
			uwsgi_error("uwsgi_eventfd_init()/write()");
			exit(1);
		}

		// we can safely close files
		close(control_fd);
		close(subject_fd);

		//does it contain a message ?
		char *colon = strchr(argv[0], ':');
		if (colon) {
			*colon = 0;
			uwsgi_add_alarm_fd(fd, argv[0], 8, uwsgi_str(colon+1), strlen(colon+1));
		}
		else {
			// register eventfd to the fd handler (make a copy of argv[0] so we safely free it)
			uwsgi_add_alarm_fd(fd, argv[0], 8, uwsgi_str(argv[0]), strlen(argv[0]));
		}


		// free memory allocated by the split_quoted parser
		size_t i;
		for(i=0;i<rlen;i++) {
			free(argv[i]);
		}
		free(argv);
	}

	return 0;
}


struct uwsgi_plugin eventfd_plugin = {
	.name = "eventfd",
	.options = eventfd_options,
	.init = uwsgi_eventfd_init,
};
