#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define WORD 16
#define READ_END 0
#define WRITE_END 1
#define INPUT_SIZE 4000

char *get_line() 
{
    char *line = (char *)malloc(WORD * sizeof(char));
    char c = (char)getchar();
    int p = 0;
    while (c != '\n') 
    {
        if (p % WORD == 0) 
        {
            line = realloc(line, p + WORD);
        }
        line[p] = c;
        c = (char)getchar();
        p++;
    }
    line[p] = '\0';
    return line;
}

char **parse_line(char *line, int *total) 
{
    int capacity = 2;  // Initial capacity for words array
    char **words = (char **)malloc(sizeof(char *) * capacity);
    if (!words) {
        perror("malloc failed");
        exit(1);
    }

    char *word = strtok(line, " ");
    int word_count = 0;

    while (word != NULL) 
    {
        // Handle '>' redirection operator with the file name
        if (strcmp(word, ">") == 0) 
        {
            // Add '>' to words array
            words[word_count] = (char *)malloc(sizeof(char) * (strlen(word) + 1));
            if (!words[word_count]) {
                perror("malloc failed");
                exit(1);
            }
            strcpy(words[word_count], word);
            word_count++;

            // Expand words array for filename
            char **temp = realloc(words, (word_count + 2) * sizeof(char *));
            if (!temp) {
                perror("realloc failed");
                exit(1);
            }
            words = temp;

            // Move to next token (assumed to be the filename)
            word = strtok(NULL, " ");
            if (word != NULL) 
            {
                words[word_count] = (char *)malloc(sizeof(char) * (strlen(word) + 1));
                if (!words[word_count]) {
                    perror("malloc failed");
                    exit(1);
                }
                strcpy(words[word_count], word);
                word_count++;
            }
            break;
        }

        // Allocate memory for the current word
        words[word_count] = (char *)malloc(sizeof(char) * (strlen(word) + 1));
        if (!words[word_count]) {
            perror("malloc failed");
            exit(1);
        }
        strcpy(words[word_count], word);
        word_count++;

        // Resize words array if needed
        if (word_count >= capacity) {
            capacity *= 2;
            char **temp = realloc(words, capacity * sizeof(char *));
            if (!temp) {
                perror("realloc failed");
                exit(1);
            }
            words = temp;
        }

        word = strtok(NULL, " ");
    }

    words[word_count] = NULL;  // NULL-terminate the array
    *total = word_count;

    return words;
}


void simple_cmd(char **command) 
{
    pid_t pid = fork();
    if (pid == 0) {
        execvp(command[0], command);
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);
    } else {
        perror("fork failed");
    }
}

int main() 
{

    int running = 1;
    while (running) {
        printf("> ");
        char *line = get_line();
        int type = 0;

        if (strncmp(line, "exit", 4) == 0) 
        {
            running = 0;
            free(line);
            continue;
        }

        if (strchr(line, '$') != NULL) 
        {
            type = 1;
        } 
        else if (strchr(line, '=') != NULL) 
        {
            type = 2;
        } 
        else if (strchr(line, '>') != NULL) 
        {
            type = 3;
        } 
        else if (strchr(line, '|') != NULL) 
        {
            type = 4;
        }

        int word_count = 0;
        char **command = parse_line(line, &word_count);

        if (type == 0) 
        {
            simple_cmd(command);
        } 
        else if (type == 1) 
        {
            memmove(command[1], command[1] + 1, strlen(command[1]));
            char *value = getenv(command[1]);
            if (value) {
                printf("%s\n", value);
            } else {
                printf("Variable not found\n");
            }
        } 
        else if (type == 2) 
        {
            char *equal_pos = strchr(line, '=');
            *equal_pos = '\0';
            char *name = line;
            char *value = equal_pos + 1;
            if (setenv(name, value, 1) == 0) 
            {
                printf("Environment variable set: %s=%s\n", name, value);
            } else 
            {
                perror("setenv failed");
            }
        }
         else if (type == 3) 
         {


            int saved_stdout = dup(STDOUT_FILENO);
            if (saved_stdout == -1) 
            {
                perror("dup");
                return 1;
            }
            char *filename = NULL;
            for (int i = 0; i < word_count; i++) 
            {
                if (strcmp(command[i], ">") == 0 && i + 1 < word_count) 
                {
                filename = command[i + 1];
                command[i] = NULL;  // Terminate command before `>`
                break;
                }
            }

        if (filename) 
        {
            int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd == -1) 
            {
                perror("File open failed");
                close(saved_stdout);
                return 1;
            }

            // Redirect stdout to the file
            if (dup2(fd, STDOUT_FILENO) == -1) 
            {
                perror("Redirection failed");
                close(fd);
                close(saved_stdout);
                return 1;
            }

        simple_cmd(command);  // Execute the command

        // Restore the original stdout
        dup2(saved_stdout, STDOUT_FILENO);
        close(fd);
        close(saved_stdout);
        } 
        else 
            {
            printf("Redirection error: No output file specified\n");
            }
        } 
        else if (type == 4) 
        {
            char **cmd1 = malloc(sizeof(char *) * word_count);
            char **cmd2 = malloc(sizeof(char *) * word_count);
            int i = 0, j = 0, in_second_cmd = 0;
            while (command[i] != NULL) 
            {
                if (strcmp(command[i], "|") == 0) 
                {
                    in_second_cmd = 1;
                    cmd1[j] = NULL;
                    j = 0;
                } 
                else if (in_second_cmd) 
                {
                    cmd2[j++] = strdup(command[i]);
                } 
                else {
                    cmd1[j++] = strdup(command[i]);
                }
                i++;
            }
            cmd2[j] = NULL;

            int pipefd[2];
            if (pipe(pipefd) == -1) 
            {
                perror("pipe");
                return 1;
            }

            pid_t pid1 = fork();
            if (pid1 == 0) 
            {
                dup2(pipefd[WRITE_END], STDOUT_FILENO);
                close(pipefd[READ_END]);
                close(pipefd[WRITE_END]);
                execvp(cmd1[0], cmd1);
                perror("execvp failed");
                exit(1);
            }

            pid_t pid2 = fork();
            if (pid2 == 0) 
            {
                dup2(pipefd[READ_END], STDIN_FILENO);
                close(pipefd[WRITE_END]);
                close(pipefd[READ_END]);
                execvp(cmd2[0], cmd2);
                perror("execvp failed");
                exit(1);
            }

            close(pipefd[READ_END]);
            close(pipefd[WRITE_END]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);

            for (int k = 0; cmd1[k] != NULL; k++) free(cmd1[k]);
            for (int k = 0; cmd2[k] != NULL; k++) free(cmd2[k]);
            free(cmd1);
            free(cmd2);
        }

        free(line);
        for (int i = 0; i < word_count; i++) 
        {
            free(command[i]);

        }
        free(command);
    }
    return 0;
}