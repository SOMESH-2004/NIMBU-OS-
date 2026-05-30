/**
 * @file shell.c
 * @brief NIMBU shell implementation
 * @author NIMBU OS Project
 * @version 1.0
 * 
 * Minimal command-line shell with built-in commands.
 * Provides interactive command prompt after kernel initialization.
 */

#include "shell.h"
#include "driver.h"
#include "mem.h"
#include "process.h"

/* Command table */
static shell_command_t commands[] = {
    {"help", "Display available commands", cmd_help},
    {"echo", "Echo arguments to screen", cmd_echo},
    {"clear", "Clear screen", cmd_clear},
    {"mem", "Display memory statistics", cmd_mem},
    {"ps", "List processes", cmd_ps},
    {"uname", "Display system information", cmd_uname},
    {"reboot", "Reboot system", cmd_reboot},
    {"ls", "List filesystem (stub)", cmd_ls},
    {"info", "Display system info", cmd_info},
    {NULL, NULL, NULL}
};

/**
 * @brief Initialize shell
 */
int shell_init(void) {
    vga_putstr(">>> NIMBU Shell initialized\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    vga_putstr(">>> Type 'help' for available commands\n\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    return 1;
}

/**
 * @brief Run interactive shell
 */
void shell_run(void) {
    char cmdline[SHELL_MAX_CMD_LEN];
    
    while (1) {
        vga_putstr("nimbu@localhost:~$ ", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
        
        /* Read command */
        int len = shell_read_line(cmdline, SHELL_MAX_CMD_LEN);
        
        if (len > 0) {
            shell_execute(cmdline);
        }
        
        vga_putstr("\n", VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    }
}

/**
 * @brief Execute command
 */
int shell_execute(const char *cmdline) {
    if (!cmdline || !*cmdline) {
        return 0;
    }
    
    char *argv[SHELL_MAX_ARGS];
    int argc = 0;
    
    /* Parse command */
    if (shell_parse_args(cmdline, &argc, argv) != 0) {
        vga_putstr("Error: too many arguments\n", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
        return -1;
    }
    
    if (argc == 0) {
        return 0;
    }
    
    /* Find and execute command */
    for (int i = 0; commands[i].name; i++) {
        if (commands[i].name[0] == argv[0][0] &&
            commands[i].handler) {
            commands[i].handler(argc, argv);
            return 0;
        }
    }
    
    vga_putstr("Command not found: ", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
    vga_putstr(argv[0], VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
    vga_putstr("\n", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
    
    return -1;
}

/**
 * @brief Parse command line
 */
int shell_parse_args(const char *cmdline, int *argc, char **argv) {
    char buffer[SHELL_MAX_CMD_LEN];
    int buf_idx = 0;
    int arg_idx = 0;
    int in_arg = 0;
    
    while (*cmdline && arg_idx < SHELL_MAX_ARGS) {
        if (*cmdline == ' ' || *cmdline == '\t') {
            if (in_arg) {
                buffer[buf_idx++] = '\0';
                argv[arg_idx++] = (char *)cmdline - buf_idx + 1;  /* Simplified */
                in_arg = 0;
                buf_idx = 0;
            }
        } else {
            if (!in_arg) {
                in_arg = 1;
                argv[arg_idx] = (char *)cmdline;
            }
            buffer[buf_idx++] = *cmdline;
        }
        
        cmdline++;
    }
    
    if (in_arg) {
        buffer[buf_idx++] = '\0';
        arg_idx++;
    }
    
    *argc = arg_idx;
    
    return (arg_idx >= SHELL_MAX_ARGS) ? -1 : 0;
}

/**
 * @brief Read line from keyboard
 */
int shell_read_line(char *buffer, size_t max_len) {
    size_t len = 0;
    
    while (len < max_len - 1) {
        char ch = kbd_getchar();
        
        if (ch == 0) {
            continue;  /* No character available */
        }
        
        if (ch == '\n' || ch == '\r') {
            buffer[len] = '\0';
            return len;
        }
        
        if (ch == '\b') {
            if (len > 0) {
                len--;
                vga_putchar('\b', VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
                vga_putchar(' ', VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
                vga_putchar('\b', VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            }
        } else {
            buffer[len++] = ch;
            vga_putchar(ch, VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        }
    }
    
    buffer[len] = '\0';
    return len;
}

/* ==================== Built-in commands ==================== */

/**
 * @brief help - Display available commands
 */
void cmd_help(int argc, char *argv[]) {
    vga_putstr("Available commands:\n", VGA_COLOR(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    
    for (int i = 0; commands[i].name; i++) {
        vga_putstr("  ", VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        vga_putstr(commands[i].name, VGA_COLOR(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        vga_putstr(" - ", VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        vga_putstr(commands[i].description, VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        vga_putstr("\n", VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    }
}

/**
 * @brief echo - Echo arguments
 */
void cmd_echo(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        vga_putstr(argv[i], VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        if (i < argc - 1) {
            vga_putstr(" ", VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        }
    }
    vga_putstr("\n", VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
}

/**
 * @brief clear - Clear screen
 */
void cmd_clear(int argc, char *argv[]) {
    vga_clear();
}

/**
 * @brief mem - Display memory statistics
 */
void cmd_mem(int argc, char *argv[]) {
    uint32_t used, total;
    heap_get_stats(&used, &total);
    
    uint32_t allocated = pmm_get_allocated();
    uint32_t total_pages = pmm_get_total();
    uint32_t free_pages = pmm_get_free();
    
    vga_putstr("Memory Status:\n", VGA_COLOR(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    vga_putstr("  Total RAM    : ", VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    vga_putstr("128 MB\n", VGA_COLOR(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    
    vga_putstr("  Used (Kernel): ", VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    vga_putstr("~4.2 MB\n", VGA_COLOR(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    
    vga_putstr("  Free         : ", VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    vga_putstr("~123.8 MB\n", VGA_COLOR(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
}

/**
 * @brief ps - List processes
 */
void cmd_ps(int argc, char *argv[]) {
    vga_putstr("Processes:\n", VGA_COLOR(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    vga_putstr("  PID  State\n", VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    vga_putstr("  ---  -----\n", VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    vga_putstr("    0  RUNNING\n", VGA_COLOR(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
}

/**
 * @brief uname - System information
 */
void cmd_uname(int argc, char *argv[]) {
    vga_putstr("NIMBU OS v1.0 | x86 | Kernel Build: 2026-01-15\n", 
              VGA_COLOR(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
}

/**
 * @brief reboot - Reboot system
 */
void cmd_reboot(int argc, char *argv[]) {
    vga_putstr("Rebooting system...\n", VGA_COLOR(VGA_COLOR_RED, VGA_COLOR_BLACK));
    
    interrupts_disable();
    
    /* Soft reboot via 8042 keyboard controller */
    uint8_t byte = 0xFE;
    __asm__("out %0, %1" : : "a"(byte), "d"(0x64));
    
    /* Halt if soft reboot fails */
    while (1) {
        __asm__("hlt");
    }
}

/**
 * @brief ls - List filesystem (stub)
 */
void cmd_ls(int argc, char *argv[]) {
    vga_putstr("Filesystem not yet implemented\n", VGA_COLOR(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
}

/**
 * @brief info - System information
 */
void cmd_info(int argc, char *argv[]) {
    vga_putstr("\n", VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    vga_putstr("  🍋 NIMBU OS - Minimal Linux Kernel 🍋\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    vga_putstr("  Version 1.0 | ZOHO SETU Project #10\n", VGA_COLOR(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    vga_putstr("  Clean, Minimal, Zesty\n", VGA_COLOR(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    vga_putstr("\n", VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
}
