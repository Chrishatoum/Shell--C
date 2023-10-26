// Chris hatoum - 260968630 - Assignment 1
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>

#define MAX_ARGS 20
#define MAX_INPUT_LENGTH 100
#define MAX_JOBS 50

// Structure to hold job information
struct Job {
    pid_t pid;
    char command[MAX_INPUT_LENGTH];
};

struct Job jobs[MAX_JOBS];
int num_jobs = 0;

// Function to execute a piped system command
int pipe_system(char *args[], int num_args, int pipe_position);

// Function to add a job to the jobs array
void add_job(pid_t pid, const char *command);

// Function to print all jobs
void print_jobs();

// Function to bring a job to the foreground
void bring_to_foreground(int job_number);

// Function to clean up jobs array
void clean_up_jobs();

// Function to clean up and exit the program
void clean_up_and_exit();

// Function to execute an external command
pid_t execute_external_command(char *args[], int num_args, bool background);

int main(void) {
    char *args[MAX_ARGS];
    int background;

    while (1) {
        background = 0;
        printf("\n>> ");
        char input[MAX_INPUT_LENGTH];

        // Read input from the user
        if (fgets(input, MAX_INPUT_LENGTH, stdin) == NULL) {
            printf("\nReceived EOF (Control+D). Exiting...\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        // Skip if empty input
        if (strlen(input) == 0) {
            continue;
        }

        int num_args = 0;
        char *token = strtok(input, " ");

        // Tokenize the input
        while (token != NULL && num_args < MAX_ARGS - 1) {
            args[num_args++] = strdup(token);
            token = strtok(NULL, " ");
        }

        args[num_args] = NULL;

        if (num_args > 0) {
            // Check for background execution
            if (strcmp(args[num_args - 1], "&") == 0) {
                background = 1;
                if (args[num_args - 1] != NULL) {
                    free(args[num_args - 1]);  // Remove "&"
                    args[num_args - 1] = NULL;
                    num_args--;
                }
            }

            // Handle built-in commands
            if (strcmp(args[0], "echo") == 0) {
                for (int i = 1; i < num_args; i++) {
                    printf("%s ", args[i]);
                }
                printf("\n");

                // Clean up memory
                for (int i = 0; i < num_args; i++) {
                    if (args[i] != NULL) {
                        free(args[i]);
                        args[i] = NULL;
                    }
                }
            } else if (strcmp(args[0], "cd") == 0) {
                // Handle change directory
                if (num_args > 1) {
                    if (chdir(args[1]) != 0) {
                        perror("cd failed");
                    }
                } else {
                    char cwd[1024];
                    if (getcwd(cwd, sizeof(cwd)) != NULL) {
                        printf("Current directory: %s\n", cwd);
                    } else {
                        perror("getcwd failed");
                    }
                }
            } else if (strcmp(args[0], "pwd") == 0) {
                // Print current directory
                char cwd[1024];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("Current directory: %s\n", cwd);
                } else {
                    perror("getcwd failed");
                }
            } else if (strcmp(args[0], "exit") == 0) {
                // Exit the shell
                for (int i = 0; i < num_args; i++) {
                    if (args[i] != NULL) {
                        free(args[i]);
                        args[i] = NULL;
                    }
                }
                clean_up_and_exit();
            } else if (strcmp(args[0], "jobs") == 0) {
                // Print all jobs
                print_jobs();
            } else if (strcmp(args[0], "fg") == 0) {
                // Bring a job to the foreground
                if (num_args > 1) {
                    int job_number = atoi(args[1]);
                    bring_to_foreground(job_number);
                } else {
                    fprintf(stderr, "Usage: fg <job_number>\n");
                }
            } else {
                // Handle piped and external commands
                int pipe_position = -1;
                for (int i = 0; i < num_args; i++) {
                    if (strcmp(args[i], "|") == 0) {
                        pipe_position = i;
                        break;
                    }
                }

                if (pipe_position != -1) {
                    if (pipe_system(args, num_args, pipe_position)) {
                        // Successfully executed piped commands
                        // Clean up allocated memory
                        for (int i = 0; i < num_args; i++) {
                            if (args[i] != NULL) {
                                free(args[i]);
                                args[i] = NULL;  // to avoid double freeing
                            }
                        }
                        continue;
                    } else {
                        fprintf(stderr, "Piping failed.\n");
                    }
                }

                // Execute external command and handle background processes
                execute_external_command(args, num_args, background);
            }
        }

        // Clean up allocated memory
        for (int i = 0; i < num_args; i++) {
            if (args[i] != NULL) {
                free(args[i]);
                args[i] = NULL;  // to avoid double freeing
            }
        }
    }

    return 0;
}

// Function to execute a piped system command
int pipe_system(char *args[], int num_args, int pipe_position) {
    char *args1[MAX_ARGS];
    char *args2[MAX_ARGS];

    for (int i = 0; i < pipe_position; i++) {
        args1[i] = args[i];
    }
    args1[pipe_position] = NULL;

    int j = 0;
    for (int i = pipe_position + 1; i < num_args; i++) {
        args2[j] = args[i];
        j++;
    }
    args2[j] = NULL;

    int fd[2];
    if (pipe(fd) == -1) {
        perror("Pipe creation failed");
        return 0;
    }

    pid_t pid1 = fork();

    if (pid1 < 0) {
        perror("Fork failed");
        return 0;
    } else if (pid1 == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        if (execvp(args1[0], args1) == -1) {
            perror("Command execution failed");
            exit(EXIT_FAILURE);
        }
    } else {
        pid_t pid2 = fork();

        if (pid2 < 0) {
            perror("Fork failed");
            return 0;
        } else if (pid2 == 0) {
            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            if (execvp(args2[0], args2) == -1) {
                perror("Command execution failed");
                exit(EXIT_FAILURE);
            }
        } else {
            close(fd[0]);
            close(fd[1]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        }
    }

    return 1;
}

// Function to add a job to the jobs array
void add_job(pid_t pid, const char *command) {
    if (num_jobs < MAX_JOBS) {
        jobs[num_jobs].pid = pid;
        strncpy(jobs[num_jobs].command, command, MAX_INPUT_LENGTH - 1);
        jobs[num_jobs].command[MAX_INPUT_LENGTH - 1] = '\0';
        printf("[%d] %s (PID: %d) - Job created successfully.\n", num_jobs + 1, jobs[num_jobs].command, pid);
        num_jobs++;
    } else {
        fprintf(stderr, "Maximum number of jobs reached. Job creation failed.\n");
    }
}

// Function to print all jobs
void print_jobs() {
    for (int i = 0; i < num_jobs; i++) {
        printf("[%d] %s (PID: %d)\n", i + 1, jobs[i].command, jobs[i].pid);
    }
}

// Function to bring a job to the foreground
void bring_to_foreground(int job_number) {
    if (job_number >= 1 && job_number <= num_jobs) {
        pid_t pid = jobs[job_number - 1].pid;
        printf("Bringing job %d to the foreground: %s (PID: %d)\n", job_number, jobs[job_number - 1].command, pid);
        waitpid(pid, NULL, 0);

        for (int i = job_number - 1; i < num_jobs - 1; i++) {
            jobs[i] = jobs[i + 1];
        }
        num_jobs--;
    } else {
        fprintf(stderr, "Invalid job number.\n");
    }
}

// Function to clean up jobs array
void clean_up_jobs() {
    num_jobs = 0;
}

// Function to clean up and exit the program
void clean_up_and_exit() {
    clean_up_jobs();
    exit(EXIT_SUCCESS);
}

// Function to execute an external command
pid_t execute_external_command(char *args[], int num_args, bool background) {
    pid_t pid = fork();

    if (pid == 0) {
        for (int i = 0; i < num_args; i++) {
            if (strcmp(args[i], ">") == 0 && i < num_args - 1) {
                freopen(args[i + 1], "w", stdout);
                args[i] = NULL;
            }
        }
        if (execvp(args[0], args) == -1) {
            perror("Command execution failed");
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {
        perror("Fork failed");
    } else {
        if (background) {
            add_job(pid, args[0]);
        } else {
            waitpid(pid, NULL, 0);
        }
    }

    return pid;
}
