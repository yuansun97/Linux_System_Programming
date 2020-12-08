/**
 * Shell
 * CS 241 - Spring 2020
 */
#include "format.h"
#include "shell.h"
#include "vector.h"
#include "sstring.h"
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
// #define _GNU_SOURCE

char **get_options(int argc, char *argv[]);
void option_destroy(char **options);
vector *split_cmd(char *cmd);
char *match_prefix(char *prefix, vector *cmds_history);
int exec_builtin(char *command, vector *cmds_history, vector *children_pid, vector *children_cmds, int redir_type, char *path);
int exec_extern(char *command, vector *children_pid, bool is_background, int redir_type, char *path);
int exec_single_cmd(char *command, vector *cmds_history, vector *children_pid, vector *children_cmds, bool is_background, int redir_type, char *path);
int take_in_cmd(char *command, vector *cmds_history, vector *children_pid, vector *children_cmds);
int get_operator_num(char *command);
int get_operator_idx(char *command);
void load_history(FILE *fp_history, vector *cmds_hitory);
void store_history(FILE *fp_history, vector *cmds_history);
void clean_all(char *command, vector *cmds_history, vector *children_pid, vector *children_cmds, char **options, FILE *fp, FILE *fp_history);
void clean_zombies();
process_info *get_pinfo(pid_t pid, vector *children_cmds, int child_idx);
void pinfo_destroy(process_info *pinfo);
int exec_signal(char *command, vector *children_pid);
int redirection(int redir_type, char *path);
void print_ps(vector *children_pid, vector *children_cmds);


// void my_log(char *message1, char *message2);
// void print_string_vector(char *message, vector *str_vec);

/**
 * This function handles the SIGINT signal.
 * It checks if there is a currently running foreground process, 
 * and if so, kills that foreground process using SIGINT. 
 * 
 * DO NOT kill background processes!
 * 
 * (the kill function might come in handy, here and elsewhere).
 * 
 * @param signnum -- The signale number
 * */
void SIGINT_handler(int signum) {
    return;
}

typedef struct process {
    char *command;
    pid_t pid;
} process;

int shell(int argc, char *argv[]) {
    // TODO: This is the entry point for your shell.
    signal(SIGINT, SIGINT_handler);

    // Exit if incorrect number of arguments
    if (argc == 2 || argc == 4 || argc > 5) {
        print_script_file_error();
        exit(0);
    }
    vector *cmds_history = string_vector_create();
    vector *children_pid = int_vector_create();
    vector *children_cmds = string_vector_create();
    // Initialize shell process
    process proc_shell;
    // Get the initial ./shell command
    sstring *initial_sstr = cstr_to_sstring(argv[0]);
    for (int i = 1; i < argc; i++) {
        sstring *arg_sstr = cstr_to_sstring(argv[i]);
        sstring *space_sstr = cstr_to_sstring(" ");
        sstring_append(initial_sstr, space_sstr);
        sstring_append(initial_sstr, arg_sstr);
        sstring_destroy(arg_sstr);
        sstring_destroy(space_sstr);
    }
    proc_shell.command = sstring_to_cstr(initial_sstr);
    sstring_destroy(initial_sstr);
    print_command(proc_shell.command);
    free(proc_shell.command);

    proc_shell.pid = getpid();
    // Extract -h/-f options
    char **options = get_options(argc, argv);
    char *command = NULL;

    // -h file -- load history file to vector and create append FILE pointer
    FILE *fp_history = NULL;
    if (options[0] != NULL) {
        fp_history = fopen(options[0], "r");   // read and wirte
        // load history (FILE pointer will be closed in the function)
        load_history(fp_history, cmds_history);
        // printf("--------%p\n", fp_history);
        // FILE pointer for future writing
        fp_history = fopen(options[0], "w");
    }

    // -f file 
    FILE *fp = (options[1] == NULL) ? stdin : fopen(options[1], "r");
    if (fp == NULL) {
        // Exit if file cannot found
        print_script_file_error();
        clean_all(command, cmds_history, children_pid, children_cmds, options, fp, fp_history);
        exit(0);
    }


    while(1) {
        // flush stdin and stdout
        fflush(NULL);
        clean_zombies();

        // Print prompt in stdard input mode
        if (options[1] == NULL) {
            print_prompt(getenv("PWD"), proc_shell.pid);
        }

        /** -- Get input --**/
        size_t size = 0;
        ssize_t line_size = getline(&command, &size, fp);

        // Clean memory and close file if reaching end of the file
        // printf("--cmd:%s--\n", command);
        if (line_size <= 0 || command[0] == EOF) {
            // Append history to file (close FILE pointer in the function)
            store_history(fp_history, cmds_history);
            fp_history = NULL;
            clean_all(command, cmds_history, children_pid, children_cmds, options, fp, fp_history);
            exit(0);
        } else if (command[line_size - 1] == '\n') {
            // printf("--str lengt:%zu--\n", strlen(command));
            // printf("--line size:%zu--\n", line_size);
            command[line_size - 1] = '\0';  // replace '\n' with '\0'
            // printf("--str lengt:%zu--\n", strlen(command));
        }


        if (strcmp(command, "exit") == 0) {
            // Append history to file
            if (fp != stdin) {
                // Print out exit in -f mode
                print_prompt(getenv("PWD"), getpid());
                print_command(command);
            }
            store_history(fp_history, cmds_history);
            fp_history = NULL;
            clean_all(command, cmds_history, children_pid, children_cmds, options, fp, fp_history);
            exit(0);
        }

        /** -- Print prompt and command read from file -- **/
        if (options[1] != NULL) {
            print_prompt(getenv("PWD"), getpid());
            print_command(command);
        }

        
        // This function takes command with / without logical operators,
        //  extract single command and call exec_single_cmd to execute respectively.
        take_in_cmd(command, cmds_history, children_pid, children_cmds);
        
    }

    return 0;
}




/******************** Modular Functions below **********************/

/**
 * This function returns the arguments of -h and -f options (if any)
 * from user input in argv[]. 
 * options[0] for -h history
 * options[1] for -f file
 * NULL for the non-given option.
 * Eg input "./shell -h history.txt -f file.txt" will yiel
 * 	  options = {"history.txt", "file.txt"}
 * 
 * @param argc -- The arguments count
 * @param argv -- Array of arguments as strings
 * 
 * @return Array of two strings for -h argument and -g argument, NULL as default
 * */
char **get_options(int argc, char *argv[]) {
    char **options = (char **) malloc(2 * sizeof(char *));
	options[0] = NULL;
	options[1] = NULL;
    // ./shell
    if (argc == 1) {return options;}
    int opt;
	while ((opt = getopt(argc, argv, "hf")) != -1) {
		switch (opt)
		{
			case 'h': {
				// printf("history arg:%s\n", argv[optind]);
                options[0] = strdup(argv[optind]);
                // printf("LINE %d\n", __LINE__);
				break;
			}
			case 'f': {
				// printf("file arg:%s\n", argv[optind]);
                options[1] = strdup(argv[optind]);
                // printf("LINE %d\n", __LINE__);
				break;
			}
			default: {
				print_usage();
				break;
            }
		}
	}
    return options;
}

/**
 * This function free the memory of options.
 * Note options must be array of strings with size 2!
 * 
 * @param options -- The option array to destroy
 * */
void option_destroy(char **options) {
	if (options == NULL) {return;};
	if (options[0]) {free(options[0]);}
	if (options[1]) {free(options[1]);}
	free(options);
}

/**
 * This function load commands from a file into the vector,
 *  and close the file after reading.
 * */
void load_history(FILE *fp_history, vector *cmds_hitory) {
    if (fp_history == NULL) {return;}
    char *command = NULL;
    size_t size = 0;
    int line_size = getline(&command, &size, fp_history);
    while( line_size >= 0 ) {
        command[line_size - 1] = '\0';  // replace '\n' in the end with '\0'
        // printf("--%s--\n", command);
        vector_push_back(cmds_hitory, command);
        line_size = getline(&command, &size, fp_history);
    }
    free(command);
    fclose(fp_history);
}

/**
 * This function accepts a valid command input
 *  splits the command by space and return an array of strings.
 * 
 * @param cmd -- The command to split
 * 
 * @return An array of strings that are splitted parts from command
 * */
vector *split_cmd(char *cmd) {
    sstring *sstr_cmd = cstr_to_sstring(cmd);
    vector *ret = sstring_split(sstr_cmd, ' ');
    sstring_destroy(sstr_cmd);
    return ret;
}

/**
 * This function searches for the command matching the given prefix.
 * 
 * @param prefix -- The target prefix
 * @param cmds_history -- The vector of previous commands
 * 
 * @return the matching command, NULL if no match command 
 * */
char *match_prefix(char *prefix, vector *cmds_history) {
    if (vector_size(cmds_history) == 0) {return NULL;} 
    int len = strlen(prefix);
    if (len == 0) {return *vector_back(cmds_history);}
    char *match_cmd = NULL;
    char buffer[len + 1];   // plus 1 for '\0'
    int idx = (int) vector_size(cmds_history) - 1;
    while (idx >= 0) {
        char *iter = vector_get(cmds_history, idx);
        strncpy(buffer, iter, len);
        buffer[len] = '\0';
        // printf("buffer is:%s\n", buffer);
        if (strcmp(prefix, buffer) == 0) {
            match_cmd = iter;
            break;
        }
        idx--;
    }               
    return match_cmd;
}

/**
 * This function accepts a command
 *  returns 0 if it contains logic operator &&
 *          1 if it contains logic operator ||
 *          2 if it contains logic operator ;
 *          3 if it contains background process flag &
 *          4 -- output >    
 *          5 -- append >>      
 *          6 -- input <
 *         -1 if no operators
 * */
int get_operator_num(char *command) {
    int len = strlen(command);
    if (len < 3) { return -1; }
    // Check background process flag
    if (command[len - 1] == '&') { 
        return 3; 
    }
    // Check logic operators
    for (int i = 0; i < len - 1; i++) {
        if (command[i] == '&' && command[i + 1] == '&') {
            return 0;
        } 
        if (command[i] == '|' && command[i + 1] == '|') {
            return 1;
        }
        if (command[i] == ';') {
            return 2;
        }
        if (command[i] == '>') {
            if (command[i + 1] == '>') {
                // append >>
                return 5;
            } else {
                // output >
                return 4;
            }
        }
        if (command[i] == '<') {
            return 6;
        }
    }
    return -1;
}

/**
 * This function accepts a command
 *  returns the index of the operator, -1 if no operators.
 * 
 * Eg: "a && b" will return 2
 * */
int get_operator_idx(char *command) {
    int len = strlen(command);
    if (len < 3) {return -1;}
    for (int i = 0; i < len - 1; i++) {
        if (command[i] == '&' && command[i + 1] == '&') {
            return i;
        } 
        if (command[i] == '|' && command[i + 1] == '|') {
            return i;
        }
        if (command[i] == ';') {
            return i;
        }
        if (command[i] == '>' || command[i] == '<') {
            return i;
        }
    }
    return -1;
}

/**
 * This function accepts the command to execute 
 *  and the previous commands history stored in a vector.
 * It executes the according built-in command 
 *  and returns the number of the command,
 * 
 *  0 -- cd <path>
 *  1 -- !history   
 *  2 -- #<n>       (always return 2 no matter succeed or not)
 *  3 -- !<prefix>  (always return 3 no matter succeed or not)
 *  4 -- ps
 * -1 -- cd <path> but with wrong format (can't be other command either)
 * -2 -- NOT a built-in command (can be other extern commands)
 * 
 * @param command -- The command to execute.
 * @param cmds_history -- The vector of previous commands.
 * 
 * @return the number of executed built-in command, 
 *          -1 if cd <path> with wrong format,
 *          -2 if not a built-in command but might be other extern command.
 * */
int exec_builtin(char *command, vector *cmds_history, vector *children_pid, vector *children_cmds, int redir_type, char *path) {
        int ret = -2;
        vector *cmd_splitted = split_cmd(command);
        // print_string_vector("----cmd_splitted----", cmd_splitted);
        char *cmd_head = vector_get(cmd_splitted, 0);
        // Four built-in commands executing without fork
        // 0 -- "cd <path>"
        if (strcmp(cmd_head, "cd") == 0) {
            if (vector_size(cmd_splitted) == 2) {
                char *path = vector_get(cmd_splitted, 1);
                if (chdir(path) != 0) {
                    // failed if non-zero returned
                    print_no_directory(path);
                    ret = -1;
                } else {
                    char buffer[200];
                    // printf("%s\n", getcwd(buffer, 100));
                    // system("ls");
                    setenv("PWD", getcwd(buffer, 200), 1);
                    ret = 0;
                }
            } else {
                print_invalid_command(command);
                ret = -1;
            }
        } 
        // 1 -- "!history"
        else if (strcmp(cmd_head, "!history") == 0) {
            for (size_t i = 0; i < vector_size(cmds_history); i++) {
                print_history_line(i, vector_get(cmds_history, i));
            }
            ret = 1;
        }
        // 2 -- "#<n>"
        else if (cmd_head[0] == '#') {
            size_t idx = atoi(cmd_head + 1);
            if (idx >= vector_size(cmds_history)) {
                print_invalid_index();
            } else {
                char *old_cmd = vector_get(cmds_history, idx);
                print_command(old_cmd);
                char *copy_cmd = strdup(old_cmd);
                take_in_cmd(copy_cmd, cmds_history, children_pid, children_cmds);
            }
            ret = 2;
        }
        // 3 -- "!<prefix>""
        else if (cmd_head[0] == '!') {
            char *prefix = cmd_head + 1;
            char *last_cmd = match_prefix(prefix, cmds_history);
            if (last_cmd == NULL) {
                print_no_history_match();
            } else {
                print_command(last_cmd);
                char *copy_cmd = strdup(last_cmd);
                take_in_cmd(copy_cmd, cmds_history, children_pid, children_cmds);
            }
            ret = 3;
        }
        // 4 -- "ps"
        else if ( strcmp(cmd_head, "ps") == 0 ) {
            if (redir_type != 0) {
                // redirection
                // 4 -- output >    5 -- append >>     *6 -- input < (Undefined behaivior)
                int stdout_copy = dup(1);
                close(1);
                int fd = redirection(redir_type, path);
                print_ps(children_pid, children_cmds);
                close(fd);
                dup2(stdout_copy, 1);
                close(stdout_copy);
            } else {
                print_ps(children_pid, children_cmds);
            }
            ret = 4;
        }

        vector_destroy(cmd_splitted);
        return ret;
}

/**
 * This function accepts the command to execute 
 *  and the previous commands stored in a vector.
 * 
 * @param command -- The command to execute
 * @param cmds_history -- The previous commands
 * 
 * @return 0 if succeed, 1 if failed
 * */
int exec_extern(char *command, vector *children_pid, bool is_background, int redir_type, char *path) {
    // printf("--extern command:%s--\n", command);
    vector *cmd_splitted = split_cmd(command);
    size_t size = vector_size(cmd_splitted);
    char *args[size + 1];
    for (size_t i = 0; i < size; i++) {
        args[i] = vector_get(cmd_splitted, i);
        // printf("%s ", args[i]);
    }
    args[size] = (char *) NULL;
    // printf("\n");
    fflush(NULL);

    pid_t child = fork();
    if (child == -1) {
        print_fork_failed();
        exit(1);
    }
    if (child == 0) {
        print_command_executed(getpid());
    }

    // Check if redirection required
    int fd = -1;
    int stdout_copy = -1;
    int stdin_copy = -1;
    if (redir_type == 4 || redir_type == 5) {
        // redirection 4 -- output >    5 -- append >> 
        stdout_copy = dup(1);
        close(1);
        fd = redirection(redir_type, path);
        // close(fd);
        // dup2(stdout_copy, 1);
        // close(stdout_copy);
    } 
    else if (redir_type == 6) {
        // redirection 6 -- input < 
        stdin_copy = dup(0);
        close(0);
        fd = redirection(redir_type, path);
    }

    if (child == 0) {
        // I'm the child
        if (is_background) {
            pid_t self_pid = getpid();
            // Set child's group pid to its own pid
            setpgid(0, self_pid);
            // printf("---gpid=%d---\n", getpgid(self_pid));
        }
        execvp(args[0], args);
        print_exec_failed(command);
        vector_destroy(cmd_splitted);
        exit(1);
    } 
    else {
        // I'm the parent
        // No wait for background process
        vector_push_back(children_pid, &child);
        if (is_background) {
            if (redir_type == 4 || redir_type == 5) {
                // restore stdout
                close(fd);
                dup2(stdout_copy, 1);
                close(stdout_copy);
            } else if (redir_type == 6) {
                // restore stdin
                close(fd);
                dup2(stdin_copy, 0);
                close(stdin_copy);
            }
            vector_destroy(cmd_splitted);
            return 0;
        }
        // foreground process
        int status;
        int wait = waitpid(child, &status, 0);
        if (redir_type == 4 || redir_type == 5) {
            // restore stdout
            close(fd);
            dup2(stdout_copy, 1);
            close(stdout_copy);
        } else if (redir_type == 6) {
            // restore stdin
            close(fd);
            dup2(stdin_copy, 0);
            close(stdin_copy);
        }
        vector_destroy(cmd_splitted);
        if (wait < 0) {
            print_wait_failed();
            exit(1);
        } 
        if (WIFEXITED(status)) {
            // child exit normally: 0 -- exec succeds, 1 -- fails
            int exit_status = WEXITSTATUS(status);
            return exit_status;
        }
    }
    return 1;
}

/**
 * This function executes an input command without operators.
 * 
 * @param
 * 
 * @return 1 if command Succeed and Need to be stored in history,
 *         2 if command Succeed but NO need to be stored  
 *        -1 if command Failed and Need to be stored in history
 *        -2 if command Failed and NO to be stored in history
 * */
int exec_single_cmd(char *command, vector *cmds_history, vector *children_pid, vector *children_cmds, bool is_background, int redir_type, char *path) {

        /** -- Execute command -- **/
        int signal_num = exec_signal(command, children_pid);
        if (signal_num != 0) {
            // return  signal command
            return signal_num;
        }
        int builtin_num = exec_builtin(command, cmds_history, children_pid, children_cmds, redir_type, path);
        // printf("--built-in return: %d--\n", builtin_num);
        if (builtin_num == -2) {
            // command is not built-in, it might be extern
            if (is_background) {
                int len = strlen(command);
                char *original_cmd = malloc(len + 3);
                strcpy(original_cmd, command);
                original_cmd[len] = ' ';
                original_cmd[len + 1] = '&';
                original_cmd[len + 2] = '\0';
                vector_push_back(children_cmds, original_cmd);
                free(original_cmd);
            } else {
                vector_push_back(children_cmds, command);
            }
            // execute foreground extern command: 
            int extern_num = exec_extern(command, children_pid, is_background, redir_type, path);
            // printf("--extern return: %d--\n", extern_num);
            if (extern_num == 0) {
                // succeed, always need to be stored
                return 1;
            } else {
                // failed, always need to be stored
                return -1;
            }
        } else if (builtin_num == -1) {
            // Invalid built-in command but need to be stored
            return -1;
        } else if (builtin_num == 0 || builtin_num == 4) {
            // Succeed and need to be stored
            return 1;
        } else {
            // "!history" "#<n>" "!<pre>" No need to be stored
            return 2;
        }
}

/**
 * This function takes command with / without logical operators,
 *  extract single command and call exec_single_cmd to execute respectively.
 * 
 * */
int take_in_cmd(char *command, vector *cmds_history, vector *children_pid, vector *children_cmds) {
        
        // detect operators &&  ||  ;  in command
        // 0 -- &&      1 -- ||     2 -- ;      3 -- &      
        // 4 -- output >    5 -- append >>      6 -- input <
        int operator_type = get_operator_num(command);
        // printf("##operator type: %d##\n", operator_type);
        
        // printf("##operator index: %d##\n", idx);
        char *x_cmd = NULL;
        char *y_cmd = NULL;
        if (operator_type == -1 || operator_type == 3) {
            // Single command
            int res = 0;
            if (operator_type == -1) {
                // Not a background command
                res = exec_single_cmd(command, cmds_history, children_pid, children_cmds, false, 0, (char *) NULL);
            } else {
                // Alter the tail if it's a background command
                // command[strlen(command) - 2] = '\0';
                int len = strlen(command);
                char background_cmd[len - 1];
                strncpy(background_cmd, command, len - 2);
                background_cmd[len - 2] = '\0';
                // printf("---%s---\n", background_cmd);
                // printf("---%s---\n", command);
                res = exec_single_cmd(background_cmd, cmds_history, children_pid, children_cmds, true, 0, (char *) NULL);
            }
            if (res == 1 || res == -1) {
                // need to be stored then
                vector_push_back(cmds_history, command);
            }
        } 
        
        else {
            // Multiple commands with a logic operator
            int op_idx = get_operator_idx(command);
            sstring *sstr_cmd = cstr_to_sstring(command);
            if (operator_type == 2) {
                // ;
                x_cmd = sstring_slice(sstr_cmd, 0, op_idx);
                y_cmd = sstring_slice(sstr_cmd, op_idx + 2, strlen(command));
            } 
            else if (operator_type == 0 || operator_type == 1) {
                // &&   ||
                x_cmd = sstring_slice(sstr_cmd, 0, op_idx - 1);
                y_cmd = sstring_slice(sstr_cmd, op_idx + 3, strlen(command));
            }
            // 4 -- output >    5 -- append >>      6 -- input <
            else if (operator_type == 4) {
                // 4 -- output >
                x_cmd = sstring_slice(sstr_cmd, 0, op_idx - 1);
                // y_cmd here used as the redirection file path
                y_cmd = sstring_slice(sstr_cmd, op_idx + 2, strlen(command));
            }
            else if (operator_type == 5) {
                // 5 -- append >>
                x_cmd = sstring_slice(sstr_cmd, 0, op_idx - 1);
                // y_cmd here used as the redirection file path
                y_cmd = sstring_slice(sstr_cmd, op_idx + 3, strlen(command));
            }
            else if (operator_type == 6) {
                // 6 -- input <
                x_cmd = sstring_slice(sstr_cmd, 0, op_idx - 1);
                // y_cmd here used as the redirection file path
                y_cmd = sstring_slice(sstr_cmd, op_idx + 2, strlen(command));
            }


            sstring_destroy(sstr_cmd);
        }

        switch (operator_type)
        {
            // exec_single_cmd returns 1 or 2 if command succeed, -1 or -2 if failed
            case 0: {   // &&
                int exec_res = exec_single_cmd(x_cmd, cmds_history, children_pid, children_cmds, false, 0, (char *) NULL);
                if (exec_res == 1 || exec_res == 2) {
                    // execute y if x succeed
                    exec_single_cmd(y_cmd, cmds_history, children_pid, children_cmds, false, 0, (char *) NULL);
                }
                vector_push_back(cmds_history, command);
                break;
            }
            case 1: {   // ||
                // printf("--x:%s--\n", x_cmd);
                // printf("--y:%s--\n", y_cmd);
                int exec_res = exec_single_cmd(x_cmd, cmds_history, children_pid, children_cmds, false, 0, (char *) NULL);
                if (exec_res == -1 || exec_res == -2) {
                    // execute y if x failed
                    exec_single_cmd(y_cmd, cmds_history, children_pid, children_cmds, false, 0, (char *) NULL);
                }
                vector_push_back(cmds_history, command);
                break;
            }
            case 2: {   // ;
                exec_single_cmd(x_cmd, cmds_history, children_pid, children_cmds, false, 0, (char *) NULL);
                exec_single_cmd(y_cmd, cmds_history, children_pid, children_cmds, false, 0, (char *) NULL);
                vector_push_back(cmds_history, command);
                break;
            }
            // 4 -- output >    5 -- append >>      6 -- input <
            case 4: {   // output >
                exec_single_cmd(x_cmd, cmds_history, children_pid, children_cmds, false, 4, y_cmd);
                break;
            }
            case 5: {   // append >>
                exec_single_cmd(x_cmd, cmds_history, children_pid, children_cmds, false, 5, y_cmd);
                break;
            }
            case 6: {   // input <
                exec_single_cmd(x_cmd, cmds_history, children_pid, children_cmds, false, 6, y_cmd);
                break;
            }
            default:
                break;
        }
        if (x_cmd != NULL) {free(x_cmd);}
        if (y_cmd != NULL) {free(y_cmd);}
        if (command != NULL) {free(command);}
        command = NULL;

        return 0;
}

/**
 * This function writes history stored in history vector into the history file,
 *  do nothing if the FILE pointer is NULL,
 *  close the file after writing.
 * 
 * @param fp_history -- 
 * @param cmds_history --
 * */
void store_history(FILE *fp_history, vector *cmds_history) {
    if (fp_history == NULL) {return;}
    for (size_t i = 0; i < vector_size(cmds_history); i++) {
        // printf("--%s--\n", vector_get(cmds_history, i));
        fprintf(fp_history, "%s\n", vector_get(cmds_history, i));
    }
    fclose(fp_history);
}

/**
 * This function cleans all dynamic memory in shell, 
 *  and kills all background processes. 
 * Including: command, 
 *            commands history vector, 
 *            options strings,
 *            -f FILE pointer,
 *            -h FILE pointer.
 * */
void clean_all(char *command, vector *cmds_history, vector *children_pid, vector *children_cmds, char **options, FILE *fp, FILE *fp_history) {
    for (size_t i = 0; i < vector_size(children_pid); i++) {
        pid_t child = *(pid_t *)vector_get(children_pid, i);
        kill(child, SIGINT);
    }
    vector_destroy(cmds_history);
    vector_destroy(children_pid);
    vector_destroy(children_cmds);
    option_destroy(options);
    if (command != NULL) {free(command);}
    if (fp != NULL) {fclose(fp);}
    if (fp_history != NULL) {fclose(fp_history);}
}

/**
 * This function checks if there are terminated children processes,
 *  if so reap them without blocking the current process.
 * */
void clean_zombies() {
    while ( waitpid((pid_t) -1, 0, WNOHANG) > 0 ) {}
}

/**
 * This function accepts a process id and returns the process information
 *  by reading /proc file.
 * 
 * @param pid -- The process id of whom to read
 * 
 * @return A pointer to the process info struct
 * */	
process_info *get_pinfo(pid_t pid, vector *children_cmds, int child_idx) {
    process_info *pinfo = malloc(sizeof(process_info));
    pinfo->start_str = NULL;
    pinfo->time_str = NULL;
    pinfo->command = NULL;
	char path[100];
	char cmd[100];
    char btime_buffer[1024];
	char stat[1024];
	pinfo->pid = pid;		

    // ------------------ read btime -----------------------
    FILE *fp = fopen("/proc/stat", "r");
	if (fp == NULL) {
		// perror("fopen failed!");
		free(pinfo);
		return NULL;
	}
    for (int i = 0; i < 8; i++) {
	    fgets(btime_buffer, 1024, fp);
        // printf("--%d---btime_buffer:%s---\n", i, btime_buffer);
    }
    unsigned long long btime = strtoull(btime_buffer + 6, (char **) NULL, 10);
    // printf("---btime in str:%s---\n", btime_buffer);
    // printf("---btime in llu:%llu---\n", btime);
	fclose(fp);
	fp = NULL;

	// ------------------ read command -----------------------
	sprintf(path, "/proc/%d/cmdline", pid);	
	fp = fopen(path, "r");
	if (fp == NULL) {
		// perror("fopen failed!");
        free(pinfo);
		return NULL;
	}
    if (child_idx == -1) {
	    fgets(cmd, 99, fp);
        int len = strlen(cmd);
        pinfo->command = strndup(cmd, len);
    } else {
	    pinfo->command = strdup((char *) vector_get(children_cmds, child_idx));
    }
    // printf("---command=%s---\n", pinfo->command);
	fclose(fp);
	fp = NULL;
	
	//------------------- read state ---------------------
	sprintf(path, "/proc/%d/stat", pinfo->pid);
	fp = fopen(path, "r");
	if (fp == NULL) {
		free(pinfo->command);
        free(pinfo);
		return NULL;
	}
	fgets(stat, 1023, fp);
	// printf("---stat:\n%s---\n", stat);
	sstring *sstr_stat = cstr_to_sstring(stat);
    vector *fields = sstring_split(sstr_stat, ' ');
    // print_string_vector("stat attributes", vec_attr);
    pinfo->nthreads = strtol((char *) vector_get(fields, 19), (char **) NULL, 10);
    pinfo->vsize =  strtoul(vector_get(fields, 22), (char **) NULL, 10) / 1024;
    pinfo->state = ((char *) vector_get(fields, 2))[0];

    // ------------------ start_str -------------------
    pinfo->start_str = malloc(6);
    unsigned long long starttime = strtoull((char *) vector_get(fields, 21), (char **) NULL, 10);
    time_t start_sum = starttime / sysconf(_SC_CLK_TCK) + btime;
    // printf("---start_sum:%ld---\n", start_sum);
    struct tm ts;
    ts = *localtime(&start_sum);
    strftime(pinfo->start_str, 6, "%H:%M", &ts);
    // printf("---start_str:%s---\n", pinfo->start_str);

    // // ------------------ time_str -------------------
    pinfo->time_str = (char *) malloc(5);
    unsigned long int utime = strtoul((char *) vector_get(fields, 13), (char **) NULL, 10);
    unsigned long int stime = strtoul((char *) vector_get(fields, 14), (char **) NULL, 10);
    unsigned long int cputime = utime + stime;
    unsigned long int time_in_secs = cputime / sysconf(_SC_CLK_TCK);
    // printf("---cputime:%lu---\n", cputime);
    // printf("---cputime/SC:%lu---\n", cputime / sysconf(_SC_CLK_TCK));
    execution_time_to_string(pinfo->time_str, 5, time_in_secs / 60, time_in_secs % 60);
    // printf("---time_str:%s---\n", pinfo->time_str);

    // ---------- other -----------
    // printf("---nthreads:%ld---\n", pinfo->nthreads);
    // printf("---vsize:%zu---\n", pinfo->vsize);
    // printf("---state:%c---\n", pinfo->state);
    // printf("---command:%s---\n", pinfo->command);

    vector_destroy(fields);
	sstring_destroy(sstr_stat);
    fclose(fp);
    return pinfo;
}

/**
 * This function used as ps print all processes information.
 * */
void print_ps(vector *children_pid, vector *children_cmds) {
    print_process_info_header();
    process_info *pinfo = NULL;
    pinfo = get_pinfo(getpid(), children_cmds, -1);
    print_process_info(pinfo);
    pinfo_destroy(pinfo);
    pinfo = NULL;
    for (size_t i = 0; i< vector_size(children_pid); i++) {
        clean_zombies();
        pid_t child = *(pid_t *) vector_get(children_pid, i);
        pinfo = get_pinfo(child, children_cmds, i);
        if (pinfo != NULL) {
            print_process_info(pinfo);
            pinfo_destroy(pinfo);
            pinfo = NULL;
        }
    }
}

/**
 * This function destroies a process info pointer.
 * */
void pinfo_destroy(process_info *pinfo) {
	if (pinfo->start_str) free(pinfo->start_str);
	if (pinfo->time_str) free(pinfo->time_str);
    if (pinfo->command) free(pinfo->command);
	free(pinfo);
}

/**
 * This function accepts a command a vetor of children pid
 *  executes the command if it is a signal command,
 *  and returns 1 if signal execution succeeds,
 *             -1 if signal execution fails,
 *              0 if it is NOT a signal command.
 * */
int exec_signal(char *command, vector *children_pid) {
    char sigcmd[5];
    strncpy(sigcmd, command, 4);
    sigcmd[4] = '\0';
    pid_t pid;
    clean_zombies();
    if (strcmp(sigcmd, "kill") == 0) {
        pid = atoi(command + 5);
        if (pid == 0) {
            print_invalid_command(command);
            return -1;
        }
        for (size_t i = 0; i < vector_size(children_pid); i++) {
            if (pid == *(pid_t *) vector_get(children_pid, i)) {
                kill(pid, SIGTERM);
                print_killed_process(pid, command);
                return 1;
            }
        }
        print_no_process_found(pid);
        return -1;
    }
    else if (strcmp(sigcmd, "stop") == 0) {
        pid = atoi(command + 5);
        if (pid == 0) {
            print_invalid_command(command);
            return -1;
        }
        for (size_t i = 0; i < vector_size(children_pid); i++) {
            if (pid == *(pid_t *) vector_get(children_pid, i)) {
                kill(pid, SIGSTOP);
                print_stopped_process(pid, command);
                return 1;
            }
        }
        print_no_process_found(pid);
        return -1;    
    }
    else if (strcmp(sigcmd, "cont") == 0) {
        pid = atoi(command + 5);
        if (pid == 0) {
            print_invalid_command(command);
            return -1;
        }
        for (size_t i = 0; i < vector_size(children_pid); i++) {
            if (pid == *(pid_t *) vector_get(children_pid, i)) {
                kill(pid, SIGCONT);
                print_continued_process(pid, command);
                return 1;
            }
        }
        print_no_process_found(pid);
        return -1;  
    }
    return 0;
}


/**
 * This function accepts the redirections type number and file path where to redirection, 
 *  returns the new direction file descripton if succeed,
 *          -1 if failed.
 * */
int redirection(int redir_type, char *path) {
	// 4 -- output >    5 -- append >>      6 -- input <
	int fd = -1;
	switch (redir_type)
	{
		case 4: {	// redirection output >
			int flags = O_CREAT | O_TRUNC | O_RDWR;
			mode_t modes = S_IRUSR | S_IWUSR;
			fd = open(path, flags, modes);
			break;
		}
		case 5: {	// redirection append >>
			int flags = O_CREAT | O_APPEND | O_RDWR;
			mode_t modes = S_IRUSR | S_IWUSR;
			fd = open(path, flags, modes);
			break;
		}
		case 6: {	// redirection input <
			int flags = O_RDONLY;
			mode_t modes = S_IRUSR;
			fd = open(path, flags, modes);
		}
		default:
			break;
	}
	return fd;
}



// --------------------- Debug log functions --------------------
// void my_log(char *message1, char *message2) {
//     #ifdef DEBUG
//     perror(message1);
//     perror(message2);
//     #endif
// }

// void print_string_vector(char *message, vector *str_vec) {
//     #ifdef DEBUG
//     perror(message);
//     for (size_t i = 0; i < vector_size(str_vec); i++) {
//         perror((char *) vector_get(str_vec, i));
//     }
//     #endif
// }
// --------------------------------------------------------------