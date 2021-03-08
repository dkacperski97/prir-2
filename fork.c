#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t parent_pid;

void on_usr1(int signal) {
    printf("Otrzymałem USR1. PID = %d\n", getpid());
    sleep(2);
}
void on_usr2(int signal) {
    printf("Otrzymałem USR2. PID = %d\n", getpid());
    sleep(2);
}
void on_term(int signal) {
    printf("Otrzymałem TERM. PID = %d\n", getpid());
    kill(parent_pid, SIGUSR1);
    exit(0);
}

int main()
{
    pid_t fork_pid;
    sigset_t mask;
    parent_pid = getpid();

    /* Konfiguracja obsługi sygnału USR1 */
    struct sigaction usr1;
    sigemptyset(&mask); /* Wyczyść maskę */
    usr1.sa_handler = (&on_usr1);
    usr1.sa_mask = mask;
    usr1.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &usr1, NULL);

    fork_pid = fork();
    if (fork_pid == -1)
    {
        fprintf(stderr, "Blad w fork\n");
        return EXIT_FAILURE;
    }
    else if (fork_pid == 0)
    {
        printf("Jestem procesem potomnym.\n");
        printf(
            "PID = %d fork() = %d\n", 
            getpid(),fork_pid
            
        );

        struct sigaction usr2;
        sigemptyset(&mask); /* Wyczyść maskę */
        usr2.sa_handler = (&on_usr2);
        usr2.sa_mask = mask;
        usr2.sa_flags = SA_SIGINFO;
        sigaction(SIGUSR2, &usr2, NULL);

        struct sigaction term;
        sigemptyset(&mask); /* Wyczyść maskę */
        term.sa_handler = (&on_term);
        term.sa_mask = mask;
        term.sa_flags = SA_SIGINFO;
        sigaction(SIGTERM, &term, NULL);
    }
    else
    {
        printf("Jestem procesem macierzystym.\n");
        printf(
            "PID = %d fork() = %d\n",
            getpid(), fork_pid
        );

        sleep(1);

        kill(fork_pid, SIGUSR1);
        kill(fork_pid, SIGUSR2);

        sleep(5);

        kill(fork_pid, SIGTERM);
    }

    while(1)
        pause();
}
