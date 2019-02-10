/*
 * keyTables.h
 *
 */

#ifndef KEYTABLES_H
#define KEYTABLES_H


#define SCAN_CODE_OOR -1

char *char_keys;
char *char_shift_keys;
char *func_keys[58];


bool is_char(unsigned int scan_code);
bool is_func(unsigned int scan_code);

int get_char_index(unsigned int scan_code);
int get_func_index(unsigned int scan_code);

#endif /* !KEYTABLES_H */
