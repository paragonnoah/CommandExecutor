#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

// Define maximum number of command arguments and maximum length of command
#define MAX_ARGS 256
#define MAX_CMD_LEN 1024

// This function takes a command string and an index for the command, and executes the command.
// It also creates a file to capture the command's standard output and standard error.
void execute_command(char *command, int cmd_index) {

    // Parse the command string into an array of arguments
    char *args[MAX_ARGS];
    int arg_count = 0;
    char *token = strtok(command, " ");
    while (token != NULL && arg_count < MAX_ARGS - 1) {
        args[arg_count] = token;
        arg_count++;
        token = strtok(NULL, " ");
    }
    args[arg_count] = NULL;

    // Create a file for capturing the command's standard output
    char out_file_name[256];
    sprintf(out_file_name, "%d.out", getpid());
    int out_file = open(out_file_name, O_RDWR | O_CREAT | O_APPEND, 0777);
    if (out_file == -1) {
        perror("Failed to create output file");
        exit(EXIT_FAILURE);
    }

    // Create a file for capturing the command's standard error
    char err_file_name[256];
    sprintf(err_file_name, "%d.err", getpid());
    int err_file = open(err_file_name, O_RDWR | O_CREAT | O_APPEND, 0777);
    if (err_file == -1) {
        perror("Failed to create error file");
        exit(EXIT_FAILURE);
    }

    // Fork a new process to execute the command
    pid_t pid = fork();
    if (pid == -1) {
        perror("Failed to fork process");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // This is the child process, which will execute the command

        // Redirect standard output and standard error to the appropriate files
        dup2(out_file, STDOUT_FILENO);
        dup2(err_file, STDERR_FILENO);

        // Execute the command
        int ret = execvp(args[0], args);
        if (ret == -1) {
            perror("Failed to execute command");
            exit(EXIT_FAILURE);
        }
    } else {
        // This is the parent process, which will wait for the child to finish

        printf("Starting command %d: child %d pid of parent %d\n", cmd_index, pid, getpid());

        int status;
        pid_t child_pid = waitpid(pid, &status, 0);
        if (child_pid == -1) {
            perror("Failed to wait for child process");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status)) {
            printf("Finished child %d pid of parent %d\n", pid, getpid());
            printf("Exited with exitcode = %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Killed with signal %d\n", WTERMSIG(status));
        }
    }

    // Close the output and error files
    close(out_file);
    close(err_file);
}

int main() {
    // Read commands from stdin until EOF is reached
    char cmd[MAX_CMD_LEN];
    int cmd_index = 0;
    while (fgets(cmd, MAX_CMD_LEN, stdin) != NULL) {
        // Remove newline character from end of command
        int len = strlen(cmd);
            if (len > 0 && cmd[len-1] == '\n') {
        cmd[len-1] = '\0';
    }

    // Fork a new process to execute the command
    pid_t pid = fork();
    if (pid == -1) {
        perror("Failed to fork process");
        exit(EXIT_FAILURE);
    }

    // If this is the child process, execute the command
    if (pid == 0) {
        // Execute the command and capture its output and error
        execute_command(cmd, cmd_index);
        exit(EXIT_SUCCESS);
    }

    // If this is the parent process, print a message indicating the start of the command
    printf("Starting command %d: child %d pid of parent %d\n", cmd_index+1, pid, getpid());

    // Wait for the child process to finish
    int status;
    pid_t child_pid = wait(&status);

    // If the child process exited normally, print a message indicating its completion
    if (WIFEXITED(status)) {
        printf("Finished child %d pid of parent %d\n", child_pid, getpid());
        // Print a message indicating the child process's exit status
        printf("Exited with exitcode = %d\n", WEXITSTATUS(status));
    }
    // If the child process was killed by a signal, print a message indicating the signal
    else if (WIFSIGNALED(status)) {
        printf("Killed with signal %d\n", WTERMSIG(status));
    }

    // Increment the command index
    cmd_index++;
}

return 0;
}
