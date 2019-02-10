/*
 * keyTables.c
 *
 *
 * By: Eivind Bergman
 * Date: 2019-02-06
 */

#include <stdbool.h>
#include <linux/input.h>

#include "keyTables.h"

/*
 * Keys ordered as in linux/input-event-codes.h
 * Defalt US keymap, other keymaps may differ.
 */
char *char_keys = "1234567890-=qwertyuiop[]asdfghjkl;'`\\zxcvbnm,./<";
char *char_shift_keys = "!@#$%^&*()_+QWERTYUIOP{}ASDFGHJKL:\"~|ZXCVBNM<>?>";

char *func_keys[58] = {
  "Esc", "BckSp", "Tab", "Enter", "LCtrl", "LShft", "RShft", "KP*", "LAlt", " ", "CpsLk", "F1", "F2", "F3", "F4", "F5",
  "F6", "F7", "F8", "F9", "F10", "NumLk", "ScrLk", "KP7", "KP8", "KP9", "KP-", "KP4", "KP5", "KP6", "KP+", "KP1",
  "KP2", "KP3", "KP0", "KP.", "F11", "F12", "KPEnt", "RCtrl", "KP/", "PrtSc", "AltGr", "Break", "Home", "Up", "PgUp", 
  "Left", "Right", "End", "Down", "PgDn", "Ins", "Del", "Pause", "LMeta", "RMeta", "Menu"
};

/* 
 * Map of scancodes from linux/input-event-codes.h
 */
const char code_map[] = {
/* KEY_RESERVED to KEY_TAB (0-15)
 */
"rfccccccccccccff"
/* KEY_Q to KEY_S (16-31)
 */
"ccccccccccccffcc"
/* KEY_D to KEY_V (32-47)
 */
"ccccccccccfccccc"
/* KEY B to KEY_F5 (48-63)
 */
"ccccccffffffffff"
/* KEY_F6 to KEY_KP1 (64-79)
 */
"ffffffffffffffff"
/* KEY_KP2 to KEY_KPJPCOMMA (80-95)
 */
"ffffrrfffrrrrrrr"
/* KEY_KPENTER to KEY_DELETE (96-111)
 */
"ffffffffffffffff"
/* KEY_MACRO to KEY_COMPOSE (112-127)
 */
"rrrrrrrfrrrrrfff"
};

bool is_char(unsigned int scan_code) {
    if (scan_code < sizeof(code_map)) {
        return (code_map[scan_code] == 'c');
    } else {
        return SCAN_CODE_OOR;
    }
}

bool is_func(unsigned int scan_code) {
    if (scan_code < sizeof(code_map)) {
        return (code_map[scan_code] == 'f');
    } else {
        return SCAN_CODE_OOR;
    }
}

int get_char_index(unsigned int scan_code) {
    if (scan_code >= KEY_1 && scan_code <= KEY_EQUAL) {
        return scan_code - 2;
    }
    if (scan_code >= KEY_Q && scan_code <= KEY_RIGHTBRACE) {
        return scan_code - 4;
    }
    if (scan_code >= KEY_A && scan_code <= KEY_GRAVE) {
        return scan_code - 6;
    }
    if (scan_code >= KEY_BACKSLASH && scan_code <= KEY_SLASH) {
        return scan_code - 7;
    }
    return -1;
}

int get_func_index(unsigned int scan_code) {
    if (scan_code == KEY_ESC) {
        return 0; // Esc is pos 0.
    }
    if (scan_code == KEY_BACKSPACE || scan_code == KEY_TAB) {
        return scan_code - 13; // Backspace is pos 1
    }
    if (scan_code == KEY_ENTER || scan_code == KEY_LEFTCTRL) {
        return scan_code -25; // Enter is pos 3.
    }
    if (scan_code == KEY_LEFTSHIFT) {
        return scan_code - 37; // Leftshift is pos 5.
    }
    if (scan_code >= KEY_RIGHTSHIFT && scan_code <= KEY_KPDOT) {
        return scan_code - 48; // Rightshit is pos 6.
    }
    if (scan_code == KEY_F11 || scan_code == KEY_F12) {
        return scan_code - 51; // F11 is pos 36. 
    }
    if (scan_code >= KEY_KPENTER && scan_code <=KEY_DELETE) {
        return scan_code - 58; //Enter is pos 38.
    }
    if (scan_code == KEY_PAUSE) {
        return scan_code - 65; // Pause is pos 54 
    }
    if (scan_code >= KEY_LEFTMETA && scan_code <= KEY_COMPOSE) {
        return scan_code - 70; // Leftmeta is pos 55
    }
    return -1;
}

