#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

//Define the program's capacity
#define MAX_UNIQUE_NAMES 100
#define MAX_NAME_LENGTH 30

//A struct to hold a name and its occurrence count
typedef struct {
    char name[MAX_NAME_LENGTH + 1];
    int times;
} NameCount;

// Searches for a given name in the table, returns its index if found otherwise -1
static int findIndex(NameCount table[], int used, const char *name) {
    for (int i = 0; i < used; i++) {
        if (strcmp(table[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Reads names line by line from a file (or stream) and counts them
static void process_stream(FILE *in, const char* filename, NameCount table[], int *usedPtr) {
    char line[MAX_NAME_LENGTH + 2];
    long lineNo = 0;

    while (fgets(line, sizeof(line), in) != NULL) {
        lineNo++;
        size_t len = strlen(line);

        if (len > 0 && line[len - 1] == '\n') {
            line[--len] = '\0';
        }
        // If the line is empty print a warning and continue to the next line
        if (len == 0) {
            fprintf(stderr, "Warning - file %s line %ld is empty.\n", filename, lineNo);
            continue;
        }
        //Check if the name is already in the table
        int id = findIndex(table, *usedPtr, line);
        if (id >= 0) {
            table[id].times++;
        } else {
            if (*usedPtr >= MAX_UNIQUE_NAMES) {
                fprintf(stderr, "Error: too many distinct names in %s. Increase MAX_UNIQUE_NAMES.\n", filename);
                continue; 
            }
            // Safely copy the name and initialize its counter
            strncpy(table[*usedPtr].name, line, MAX_NAME_LENGTH);
            table[*usedPtr].name[MAX_NAME_LENGTH] = '\0';
            table[*usedPtr].times = 1;
            (*usedPtr)++;
        }
    }
}

// Merges results from a child process into the parent's final table
void merge_results(NameCount final_table[], int *final_used, NameCount child_table[], int child_used) {
    for (int i = 0; i < child_used; i++) {
        //search for each name from the child in the table
        int id = findIndex(final_table, *final_used, child_table[i].name);
        if (id >= 0) {
            // If the name already exists in the main table, add the counts
            final_table[id].times += child_table[i].times;
        } else {
            // If the name is new add it as a new entry to the main table
            if (*final_used < MAX_UNIQUE_NAMES) {
                strncpy(final_table[*final_used].name, child_table[i].name, MAX_NAME_LENGTH);
                final_table[*final_used].name[MAX_NAME_LENGTH] = '\0';
                final_table[*final_used].times = child_table[i].times;
                (*final_used)++;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    //read from standard input (stdin)
    if (argc == 1) {
        NameCount table[MAX_UNIQUE_NAMES];
        int used = 0;
        process_stream(stdin, "stdin", table, &used);
        
        for (int i = 0; i < used; i++) {
            printf("%s: %d\n", table[i].name, table[i].times);
        }
        return 0;
    }

    int num_files = argc - 1;
    pid_t pids[num_files]; //Array to store the process IDs of child processes
    int pipes[num_files][2]; //A pipe for each child (0 for read end, 1 for write and)

    //Loop to create all child process
    for (int i = 0; i < num_files; i++) {
        // Create a pipe for each child
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        //Create a new child
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        //Child process code
        if (pids[i] == 0) {
            // The child will only write to the pipe so it closes the read end
            close(pipes[i][0]);

            const char *filename = argv[i + 1];
            FILE *in = fopen(filename, "r");
            if (in == NULL) {
                fprintf(stderr, "error: cannot open file %s\n", filename);
                close(pipes[i][1]);
                exit(1);
            }

            //Process the file and collect results in a local table
            NameCount child_table[MAX_UNIQUE_NAMES];
            int child_used = 0;
            process_stream(in, filename, child_table, &child_used);
            fclose(in);

            //Send the results to the parent process through the pipe
            write(pipes[i][1], &child_used, sizeof(int));
            write(pipes[i][1], child_table, child_used * sizeof(NameCount));

            // The child having finished its work, closes the write end and exits 
            close(pipes[i][1]);
            exit(0);
        }
    }
    //parent process code
    NameCount final_table[MAX_UNIQUE_NAMES];
    int final_used = 0;

    //collect results from all children
    for (int i = 0; i < num_files; i++) {
        close(pipes[i][1]);

        int child_used;
        NameCount child_table[MAX_UNIQUE_NAMES];

        //Read data from the pipe
        read(pipes[i][0], &child_used, sizeof(int));
        if (child_used > 0) {
            read(pipes[i][0], child_table, child_used * sizeof(NameCount));
            merge_results(final_table, &final_used, child_table, child_used);
        }
        //Close the read end as well, since we are done with this pipe
        close(pipes[i][0]);
    }
    // Loop to wait for all child processes to terminate
    for (int i = 0; i < num_files; i++) {
        wait(NULL);
    }

    for (int i = 0; i < final_used; i++) {
        printf("%s: %d\n", final_table[i].name, final_table[i].times);
    }

    return 0;
}