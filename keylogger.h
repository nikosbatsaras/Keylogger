#ifndef KEYLOGGER_H
#define KEYLOGGER_H

#define KEYBOARD "/dev/input/event3"

extern void keylogger_init (char *output_file);
extern void keylogger_exit (void);
extern void keylogger_run  (void);

#endif /* KEYLOGGER_H */
