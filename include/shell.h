/**
 * @file shell.h
 * @brief NIMBU shell interface
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Defines the minimal command-line shell interface with built-in commands:
 * help, ls, echo, clear, mem, ps, reboot, uname.
 */

#ifndef SHELL_H
#define SHELL_H

#include "types.h"

/* Maximum command line length */
#define SHELL_MAX_CMD_LEN   256

/* Maximum number of arguments */
#define SHELL_MAX_ARGS      16

/**
 * @brief Shell command structure
 * 
 * Defines a built-in command and its handler.
 */
typedef struct {
    const char *name;           /* Command name */
    const char *description;    /* Short description */
    void (*handler)(int argc, char *argv[]);  /* Command handler function */
} shell_command_t;

/**
 * @brief Initialize shell
 * 
 * Loads built-in command table and displays welcome message.
 * 
 * @return 0 on success, -1 on failure
 */
int shell_init(void);

/**
 * @brief Start interactive shell
 * 
 * Displays prompt and processes user commands in an infinite loop.
 * Returns only on system shutdown commands.
 */
void shell_run(void);

/**
 * @brief Execute single command
 * 
 * Parses command line and dispatches to appropriate handler.
 * 
 * @param cmdline Command line string
 * @return 0 on success, -1 on failure
 */
int shell_execute(const char *cmdline);

/**
 * @brief Parse command line into arguments
 * 
 * Splits command line string into argc/argv format.
 * 
 * @param cmdline Command line to parse
 * @param argc Output: argument count
 * @param argv Output: argument vector (array of strings)
 * @return 0 on success, -1 on failure (too many arguments)
 */
int shell_parse_args(const char *cmdline, int *argc, char **argv);

/**
 * @brief Display shell prompt and read command
 * 
 * Shows "nimbu@localhost:~$ " prompt and reads user input from keyboard.
 * 
 * @param buffer Output buffer for command line
 * @param max_len Maximum characters to read
 * @return Number of characters read
 */
int shell_read_line(char *buffer, size_t max_len);

/* Built-in commands */

/**
 * @brief help - Display available commands
 */
void cmd_help(int argc, char *argv[]);

/**
 * @brief echo - Echo arguments to screen
 */
void cmd_echo(int argc, char *argv[]);

/**
 * @brief clear - Clear screen
 */
void cmd_clear(int argc, char *argv[]);

/**
 * @brief mem - Display memory statistics
 */
void cmd_mem(int argc, char *argv[]);

/**
 * @brief ps - List running processes
 */
void cmd_ps(int argc, char *argv[]);

/**
 * @brief uname - Display system information
 */
void cmd_uname(int argc, char *argv[]);

/**
 * @brief reboot - Reboot system
 */
void cmd_reboot(int argc, char *argv[]);

/**
 * @brief ls - List filesystem (stub for now)
 */
void cmd_ls(int argc, char *argv[]);

/**
 * @brief info - Display system info
 */
void cmd_info(int argc, char *argv[]);

#endif /* SHELL_H */
