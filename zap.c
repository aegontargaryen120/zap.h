#define ZAP_IMPLEMENTATION
#include "./zap.h"

int main(int argc, char **argv) {
  if (!ZAP_REBUILD_URSELF(argc, argv)) {
    fprintf(stderr, err_fmt"Rebuild failed.\n");
    return 1;
  }
  
  Cmd cmd = {0};

  printf(info_fmt"Creating directory `bin`...\n");
  
  if (!mkdir_if_not_exists("bin")) {
    fprintf(stderr, err_fmt"Could not create directory.\n");
    return 1;
  }
  
  cc(&cmd);
  cc_flags(&cmd);
  cc_output(&cmd, "bin/main");
  cc_inputs(&cmd, "main.c");

  printf(cmd_fmt);
  cmd_dump(&cmd);
  
  if (!cmd_run(&cmd)) {
    fprintf(stderr, err_fmt"Compilation failed.\n");
    return 1;
  }

  cmd_append(&cmd, "./bin/main");

  printf(cmd_fmt);
  cmd_dump(&cmd);
  
  if (!cmd_run(&cmd)) {
    fprintf(stderr, err_fmt"Could not execute binary file.\n");
    return 1;
  }

  cmd_append(&cmd, "rm");
  cmd_append(&cmd, "-rf");
  cmd_append(&cmd, "./bin/main");
  cmd_append(&cmd, "./bin/main.dSYM");

  printf(cmd_fmt);
  cmd_dump(&cmd);
  
  if (!cmd_run(&cmd)) {
    fprintf(stderr, err_fmt"Could not remove files.\n");
    return 1;
  }
  
  return 0;
}
