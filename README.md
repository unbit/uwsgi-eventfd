uwsgi-eventfd
=============

uWSGI plugin for Linux eventfd() syscall integration

This is another plugin used by uwsgi.it structure (work in progress)

You can use it to raise an alarm whenever the file descriptor created by eventfd() is ready.

Currently it is used to monitor memory thresholds and OOM (out of memory) events in memory cgroups.

The plugin exposes a single option: ``alarm-eventfd <alarm> <control_file> <subject_file> [extra]``

Supposing you have a cgroup directory named /sys/fs/cgroup/foobar with memory controller enabled you can receive alarms whenever the memory usage is higher then the specified threshold:

```ini
[uwsgi]
plugins = eventfd
; place the instance in the specified cgroup
cgroup = /sys/fs/cgroup/foobar
; create an alarm (simple log line, use something better in production like mail or jabber/xmpp)
alarm = too_much_memory log:your instance is using too much memory
; raise too_much_memory whenever %(cgroup)/memory.usage_in_bytes is higher than 100MB
alarm-eventfd = too_much_memory %(cgroup)/cgroup.%(cgroup)/memory.usage_in_bytesevent_control 100000000
...
```

you can obviously have more thresholds:

```ini
[uwsgi]
plugins = eventfd
; place the instance in the specified cgroup
cgroup = /sys/fs/cgroup/foobar
; create an alarm (simple log line, use something better in production like mail or jabber/xmpp)
alarm = too_much_memory100 log:your instance is using more than 100MB

alarm = too_much_memory200 log:your instance is using more than 200MB
alarm = too_much_memory300 log:your instance is using more than 300MB

; raise too_much_memory whenever %(cgroup)/memory.usage_in_bytes is higher than 100MB
alarm-eventfd = too_much_memory100 %(cgroup)/cgroup.%(cgroup)/memory.usage_in_bytesevent_control 100000000

alarm-eventfd = too_much_memory200 %(cgroup)/cgroup.%(cgroup)/memory.usage_in_bytesevent_control 200000000
alarm-eventfd = too_much_memory300 %(cgroup)/cgroup.%(cgroup)/memory.usage_in_bytesevent_control 300000000

...
```

Monitoring OOM is easier (no need to specify a threshold)

```ini
[uwsgi]
plugins = eventfd
; place the instance in the specified cgroup
cgroup = /sys/fs/cgroup/foobar
; create an alarm (simple log line, use something better in production like mail or jabber/xmpp)
alarm = oom log:your instance is out of memory !!!
alarm-eventfd = oom %(cgroup)/cgroup.%(cgroup)/memory.oom_control
...
```

The plugin can be built (once cloned) with:

```sh
uwsgi --build-plugin uwsgi-eventfd
```
