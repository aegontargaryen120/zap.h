#ifndef ZAP_H_
#define ZAP_H_

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct {
  char **items;
  size_t count;
  size_t capacity;
} Cmd;

bool zap_rebuild_self(int argc, char **argv, const char *source_path);

void cmd_append(Cmd *cmd, const char *arg);
void cmd_clear(Cmd *cmd);
void cmd_free(Cmd *cmd);
bool cmd_run(Cmd *cmd);
void cmd_dump(Cmd *cmd);

bool mkdir_if_not_exists(const char *path);
bool file_exists(const char *path);
bool write_file(const char *path, const char *contents);

void cc(Cmd *cmd);
void cc_flags(Cmd *cmd);
void cc_output(Cmd *cmd, const char *output);
void cc_inputs(Cmd *cmd, const char *input);

#define ZAP_REBUILD_URSELF(argc, argv)		\
  zap_rebuild_self(argc, argv, __FILE__)

#define info_fmt "INFO: "
#define err_fmt "ERROR: "
#define cmd_fmt "CMD: "
#define zap_fmt "ZAP: "

#ifdef ZAP_IMPLEMENTATION

void cmd_append(Cmd *cmd, const char *arg) {
  if (cmd == NULL || arg == NULL) {
    fprintf(stderr, "ERROR: Invalid argument to cmd_append.\n");
    exit(1);
  }

  if (cmd->capacity < cmd->count + 2) {
    size_t new_cap = cmd->capacity < 10
      ? 10
      : cmd->capacity * 2;

    char **temp = realloc(
			  cmd->items,
			  new_cap * sizeof(*cmd->items)
			 );

    if (temp == NULL) {
      fprintf(stderr, "ERROR: Memory allocation failed.\n");
      exit(1);
    }

    cmd->items = temp;
    cmd->capacity = new_cap;
  }

  char *item = strdup(arg);

  if (item == NULL) {
    fprintf(stderr, "ERROR: strdup failed.\n");
    exit(1);
  }

  cmd->items[cmd->count++] = item;
  cmd->items[cmd->count] = NULL;
}

void cmd_clear(Cmd *cmd) {
  if (cmd == NULL)
    return;

  for (size_t i = 0; i < cmd->count; ++i) {
    free(cmd->items[i]);
  }

  cmd->count = 0;

  if (cmd->items != NULL)
    cmd->items[0] = NULL;
}

void cmd_free(Cmd *cmd) {
  if (cmd == NULL)
    return;

  cmd_clear(cmd);

  free(cmd->items);

  cmd->items = NULL;
  cmd->count = 0;
  cmd->capacity = 0;
}

bool cmd_run(Cmd *cmd) {
  if (cmd == NULL ||
      cmd->items == NULL ||
      cmd->count == 0 ||
      cmd->items[0] == NULL ||
      cmd->items[0][0] == '\0') {
    fprintf(stderr, "ERROR: Invalid command.\n");
    return false;
  }

  if (cmd->items[cmd->count] != NULL) {
    fprintf(stderr, "ERROR: Invalid argument array.\n");
    return false;
  }

  pid_t pid = fork();

  if (pid < 0) {
    perror("fork");
    return false;
  }

  if (pid == 0) {
    execvp(cmd->items[0], cmd->items);

    perror("execvp");
    _exit(127);
  }

  int status;

  while (waitpid(pid, &status, 0) < 0) {
    if (errno == EINTR)
      continue;

    perror("waitpid");
    return false;
  }

  bool success = WIFEXITED(status) && WEXITSTATUS(status) == 0;

  cmd_clear(cmd);

  return success;
}

bool mkdir_if_not_exists(const char *path) {
  if (path == NULL || *path == '\0') {
    return false;
  }

  if (mkdir(path, 0755) == 0) {
    return true;
  }

  if (errno == EEXIST) {
    struct stat st;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
      return true;
    }
  }

  return false;
}

bool file_exists(const char *path) {
  if (path == NULL || *path == '\0') {
    return false;
  }

  struct stat st;
  return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
}

bool write_file(const char *path, const char *contents) {
  if (path == NULL || *path == '\0' || contents == NULL) {
    return false;
  }

  FILE *fp = fopen(path, "w");
  if (fp == NULL) {
    return false;
  }

  size_t len = strlen(contents);
  size_t written = fwrite(contents, 1, len, fp);

  if (fclose(fp) != 0) {
    return false;
  }

  return (written == len);
}

void cc(Cmd *cmd) {
  cmd_append(cmd, "cc");
}

void cc_flags(Cmd *cmd) {
  cmd_append(cmd, "-Wall");
  cmd_append(cmd, "-Wextra");
  cmd_append(cmd, "-std=c99");
  cmd_append(cmd, "-pedantic");
  cmd_append(cmd, "-ggdb");
}

void cc_output(Cmd *cmd, const char *output) {
  cmd_append(cmd, "-o");
  cmd_append(cmd, output);
}

void cc_inputs(Cmd *cmd, const char *input) {
  cmd_append(cmd, input);
}

void cmd_dump(Cmd *cmd) {
  for (size_t i = 0; i < cmd->count; ++i) {
    printf("%s ", cmd->items[i]);
  }
  printf("\b\n");
}

bool zap_rebuild_self(int argc, char **argv, const char *source_path) {
  if (argc <= 0 || argv == NULL || argv[0] == NULL ||
      source_path == NULL || *source_path == '\0') {
    fprintf(stderr, "ERROR: Invalid arguments to zap_rebuild_self.\n");
    return false;
  }

  struct stat self_stat;
  struct stat source_stat;
  struct stat header_stat;

  bool self_exists = stat(argv[0], &self_stat) == 0;

  if (stat(source_path, &source_stat) != 0) {
    perror("stat source");
    return false;
  }

  if (stat(__FILE__, &header_stat) != 0) {
    perror("stat header");
    return false;
  }

  bool rebuild = !self_exists;

  if (self_exists) {
    if (source_stat.st_mtime > self_stat.st_mtime ||
	header_stat.st_mtime > self_stat.st_mtime) {
      rebuild = true;
    }
  }

  if (!rebuild)
    return true;

  fprintf(stderr, zap_fmt"Rebuilding build script...\n");

  size_t tmp_len = strlen(argv[0]) + sizeof(".zap-tmp");
  char *tmp_path = malloc(tmp_len);

  if (tmp_path == NULL) {
    fprintf(stderr, "ERROR: Memory allocation failed.\n");
    return false;
  }

  snprintf(tmp_path, tmp_len, "%s.zap-tmp", argv[0]);

  Cmd cmd = {0};

  cc(&cmd);
  cmd_append(&cmd, "-Wall");
  cmd_append(&cmd, "-Wextra");
  cc_output(&cmd, tmp_path);
  cc_inputs(&cmd, source_path);

  printf(cmd_fmt);
  cmd_dump(&cmd);
  
  if (!cmd_run(&cmd)) {
    cmd_free(&cmd);
    remove(tmp_path);
    free(tmp_path);
    return false;
  }

  cmd_free(&cmd);

  if (rename(tmp_path, argv[0]) != 0) {
    perror("rename");
    remove(tmp_path);
    free(tmp_path);
    return false;
  }

  free(tmp_path);

  execvp(argv[0], argv);

  perror("execvp");
  return false;
}

#endif /* ZAP_IMPLEMENTATION */

#endif /* ZAP_H_ */
