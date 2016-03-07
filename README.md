TOSAMO - syncronise parts of configuration files across linux servers.

When you need to synchronize "parts" of a config file look no further...

Imagine you have a file.conf on server A and file.conf on server B 
and you want to synchronize just parts of the file.conf simply envelop
that part in a tag "#%%" at begining and "%%#" at the end and run tosamo.

It operates as master-slave to assuming A is master and B - slave,

run "tosamo -m file.conf" on server A
and "tosamo -s file.conf" on server B

run "tosamo -h" for help

This is still fresh project made mainly for fun and to get some experience with C
but if someone finds it usefull - AWESOME!!!