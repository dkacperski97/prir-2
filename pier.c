#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int prime_index, children_number = 2, restart_prime = 0;
pid_t *fork_pids;

void on_hup(int signal) {
	printf("Jestem procesem potomnym. Otrzymałem HUP.\n");
    restart_prime = 1;
}

void on_main_hup(int signal) {
	printf("Jestem procesem macierzystym. Otrzymałem HUP.\n");
    for (int i = 0; i < children_number; i++) {
        kill(fork_pids[i], SIGHUP);
    }
}

void prime_numbers() {
    for (prime_index = 2; ; prime_index++) {
        int j = 2;
        for (; j < prime_index; j++) {
            if (prime_index % j == 0) {
                break;
            }
        }
        if (prime_index == j) {
            printf("PID: %d, prime: %d\n", getpid(), prime_index);
        }
        sleep(1);
        if (restart_prime == 1) {
            restart_prime = 0;
            prime_index = 1;
        }
    }
}

int main()
{
    pid_t fork_pid;
    sigset_t mask;
    fork_pids = (pid_t *)malloc(children_number * sizeof(pid_t));
    printf("Moj PID = %d\n", getpid());

    for (int i = 0; i < children_number; i++)
    {
        fork_pid = fork();
        if (fork_pid == -1)
        {
            fprintf(stderr, "Blad w fork\n");
            return EXIT_FAILURE;
        }
        else if (fork_pid == 0)
        {
            printf("Jestem procesem potomnym. PID = %d fork() = %d\n",
                   getpid(), fork_pid);

            struct sigaction hup;
            sigemptyset(&mask);
            hup.sa_handler = (&on_hup);
            hup.sa_mask = mask;
            hup.sa_flags = SA_SIGINFO;
            sigaction(SIGHUP, &hup, NULL);

            prime_numbers();
            break;
        }
        else
        {
            printf("Jestem procesem macierzystym. PID = %d fork() = %d\n",
                   getpid(), fork_pid);

            fork_pids[i] = fork_pid;
        }
    }

    if (fork_pid != 0) {
        struct sigaction hup;
        sigemptyset(&mask);
        hup.sa_handler = (&on_main_hup);
        hup.sa_mask = mask;
        hup.sa_flags = SA_SIGINFO;
        sigaction(SIGHUP, &hup, NULL);
    }

	while(1)
		pause();

	return 0;
}
