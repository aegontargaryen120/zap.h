#define ZAP_IMPLEMENTATION
#include "./zap.h"

int setup(void) {
  if (!mkdir_if_not_exists("bin")) {
    return 0;
  }
  
  return 1;
}

int hello(void) {
  if (!setup()) {
    return 1;
  }

  Cmd cmd = {0};

  cc(&cmd);
  cc_flags(&cmd);
  cc_output(&cmd, "bin/hello");
  cc_inputs(&cmd, "hello.c");

  printf(cmd_fmt);
  cmd_dump(&cmd);
  
  if (!cmd_run(&cmd)) {
    fprintf(stderr, err_fmt"Compilation failed.\n");
    return 1;
  }

  cmd_append(&cmd, "./bin/hello");
  
  printf(cmd_fmt);
  cmd_dump(&cmd);
  
  if (!cmd_run(&cmd)) {
    fprintf(stderr, err_fmt"Execution failed.\n");
    return 1;
  }

  cmd_append(&cmd, "rm");
  cmd_append(&cmd, "-rf");
  cmd_append(&cmd, "bin/hello");
  cmd_append(&cmd, "bin/hello.dSYM");
  
  printf(cmd_fmt);
  cmd_dump(&cmd);
  
  if (!cmd_run(&cmd)) {
    fprintf(stderr, err_fmt"Deletion failed.\n");
    return 1;
  }

  return 0;
}

int counter(void) {
  if (!setup()) {
    return 1;
  }

    Cmd cmd = {0};

  cc(&cmd);
  cc_flags(&cmd);
  cc_output(&cmd, "bin/counter");
  cc_inputs(&cmd, "counter.c");

  printf(cmd_fmt);
  cmd_dump(&cmd);
  
  if (!cmd_run(&cmd)) {
    fprintf(stderr, err_fmt"Compilation failed.\n");
    return 1;
  }

  cmd_append(&cmd, "./bin/counter");
  
  printf(cmd_fmt);
  cmd_dump(&cmd);
  
  if (!cmd_run(&cmd)) {
    fprintf(stderr, err_fmt"Execution failed.\n");
    return 1;
  }

  cmd_append(&cmd, "rm");
  cmd_append(&cmd, "-rf");
  cmd_append(&cmd, "bin/counter");
  cmd_append(&cmd, "bin/counter.dSYM");
  
  printf(cmd_fmt);
  cmd_dump(&cmd);
  
  if (!cmd_run(&cmd)) {
    fprintf(stderr, err_fmt"Deletion failed.\n");
    return 1;
  }

  return 0;
}

int main(int argc, char **argv) {
  if (!ZAP_REBUILD_URSELF(argc, argv)) {
    fprintf(stderr, err_fmt"Could not rebuild file.\n");
    return 1;
  }
  
  Args args = args_parse(argc, argv);

  const char *target = args_option(&args, "--build");

  if (target != NULL && *target != '\0') {
    if (strcmp(target, "hello") == 0) {
      return hello();
    } else if (strcmp(target, "counter") == 0) {
      return counter();
    } else {
      fprintf(stderr, err_fmt"Invalid argument.\n");
      return 1;
    }
  } else {
    fprintf(stderr, "Usage: ./zap --build [OPTIONS]\n");
    return 1;
  }

  return 0;
}
