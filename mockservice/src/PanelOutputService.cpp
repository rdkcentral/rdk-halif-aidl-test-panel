#include <signal.h>

#include "PanelOutput.h"

int main()
{
    signal(SIGPIPE, SIG_IGN);
    com::rdk::hal::panel::PanelOutput::publishAndJoinThreadPool();
    return 0;
}
