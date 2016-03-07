#ifndef _CONFIG_H
#define _CONFIG_H

L_HEAD * obj_file_parse(char *, char *, bool);
L_HEAD * obj_buffer_parse(char *, int, char *);
int obj_file_replace_tagged_parts(L_HEAD *, L_HEAD *);
int obj_write_to_file(char const *, L_HEAD const *);

#endif
