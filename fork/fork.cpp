#include <iostream>
#include <unistd.h>

int main()
{
    pid_t pid = fork();

    if (pid == -1)
    {
        std::cerr << "Fork failed!" << std::endl;
        return 1;
    }
    else if (pid == 0)
    {
        // child process
        std::cout << "Child process: PID = " << getpid() << std::endl;
        execl("/bin/ls", "ls", "-l", NULL);
    }
    else
    {
        // parent process
        std::cout << "Parent process: PID = " << getpid() << ", Child PID = " << pid << std::endl;
        wait(NULL);
    }

    return 0;
}
