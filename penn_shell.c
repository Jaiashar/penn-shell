#include "interrupt_handlers.h"
#include "parsejob.h"
#include "error_handlers.h"
#include "input_reader.h"
#include "linked_list.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define DELIMITER " \t"

typedef struct
{
  int fd[2];
} pipe_fds;

bool is_interactive()
{
  bool interactive = false;
  if (isatty(STDIN_FILENO) == 1)
  {
    interactive = true;
  }
  else if (errno == EBADF)
  {
    perror("isatty");
    exit(EXIT_FAILURE);
  }
  return interactive;
}

bool is_really_background()
{
  return isBackgroundJob() && is_interactive();
}

void install_signal_handlers(bool interactive)
{
  /* 
  * Add signal handlers for the shell
  * For SIGINT, the handler added is shell_sigint_handler
  * 
  * error handling is added for cases
  * where installation of the handler fails.
  * 
  * TODO: Add handlers for non-interactive
  * */

  if (interactive)
  {
    if (signal(SIGINT, shell_sigint_handler) == SIG_ERR)
    {
      perror("signal");
      exit(EXIT_FAILURE);
    }
    if (signal(SIGTSTP, shell_sigint_handler) == SIG_ERR)
    {
      perror("signal");
      exit(EXIT_FAILURE);
    }
    if (signal(SIGTTOU, shell_sigint_handler) == SIG_ERR)
    {
      perror("signal");
      exit(EXIT_FAILURE);
    }
    if (signal(SIGTTIN, shell_sigint_handler) == SIG_ERR)
    {
      perror("signal");
      exit(EXIT_FAILURE);
    }
    if (signal(SIGQUIT, shell_sigint_handler) == SIG_ERR)
    {
      perror("signal");
      exit(EXIT_FAILURE);
    }
  }
}

void execute_shell(bool async)
{
  int job_id = 1;
  Node *current_job = NULL;
  bool interactive = is_interactive();
  install_signal_handlers(interactive);
  Node *bg_head = NULL, *bg_tail = NULL;
  int bg_count = 0;
  while (1)
  {

    int fg_count = 0;
    char *line;
    if (interactive)
    {
      // print jobs whose status has changed since the last time
      printChangedStatuses(&bg_head, &bg_tail);
      write_err_check(PROMPT);
      line = read_input_from_prompt();
    }
    else
    {
      line = read_input_from_file();
    }
    // poll job statuses
    while (true)
    {
      int status;
      int cpid = waitpid(-1, &status, WNOHANG | WUNTRACED);
      if (cpid > 0)
      {
        updateStatus(&bg_head, &bg_tail, cpid, status);
        bg_count -= 1;
        if (bg_count == 0)
        {
          job_id = 1;
        }
      }
      else
        break;
    }

    bool is_valid = parseJob(line, interactive);
    free(line);
    if (!is_valid)
    {
      continue;
    }
    char *pipeline = getJobPipeline();
    char ***job_commands = getJobCommands();
    int command_count = getCommandCount();
    char *infile = getJobStdin();
    char *outfile = getJobStdout();
    bool append = isAppendingStdout();
    pid_t *child_pids = malloc(command_count * sizeof(pid_t));
    pipe_fds *pipes = NULL;
    if (command_count > 1)
    {
      pipes = malloc((command_count - 1) * sizeof(pipe_fds));
    }
    // 1. Create array  fds[n-1][2] for pipes, and call pipe n-1 times
    for (int i = 0; i < command_count - 1; i++)
    {
      pipe(pipes[i].fd);
    }

    if (!is_really_background())
    {
      fg_count = command_count;
    }
    // handle case command is the builtin jobs
    if (interactive && strcmp(job_commands[0][0], "jobs") == 0)
    {
      free(pipeline);
      freeJobCommands(job_commands);
      free(infile);
      free(outfile);
      free(child_pids);
      printJobs(bg_head, bg_tail);
      continue;
    }

    // handle case command is the builtin fg/bg
    else if (interactive && ((strcmp(job_commands[0][0], "bg") == 0) || (strcmp(job_commands[0][0], "fg") == 0)))
    {
      // find the current job to execute if no job_id is specified
      free(child_pids);
      bool isBackground = strcmp(job_commands[0][0], "bg") == 0;
      Node *job;
      if (job_commands[0][1] == NULL)
      {
        job = getCurrentJob(bg_head, bg_tail);
        if (!job)
        {
          fprintf(stderr, "No background/stopped jobs found!\n");
          free(pipeline);
          freeJobCommands(job_commands);
          free(infile);
          free(outfile);
          continue;
        }
      }
      // find the requested job_id
      else
      {
        int req_job_id = atoi(job_commands[0][1]);
        job = findJob(bg_head, req_job_id);
        if (!job)
        {
          fprintf(stderr, "No job with id %d found!\n", req_job_id);
          free(pipeline);
          freeJobCommands(job_commands);
          free(infile);
          free(outfile);
          continue;
        }
      }
      // print appropriate job status message
      if (WIFSTOPPED(job->status) && isBackground)
        fprintf(stderr, "Running: %s\n", job->pipeline);
      if (WIFSTOPPED(job->status) && !isBackground)
        fprintf(stderr, "Restarting: %s\n", job->pipeline);
      if (!WIFSTOPPED(job->status) && !isBackground)
        fprintf(stderr, "%s\n", job->pipeline);
      
      // reset job status
      job->status = -1;

      // send SIGCONT to the selected job
      kill(-job->pgid, SIGCONT);

      // if command is fg, give terminal control to the running process
      if (!isBackground)
      {
        removeNode(&bg_head, &bg_tail, job);
        tcsetpgrp(STDIN_FILENO, job->pgid);
        signal(SIGTTOU, SIG_IGN);
        free(pipeline);
        child_pids = job->pids;
        fg_count = job->commandCount;
        pipeline = job->pipeline;
        free(job);
      }
      else
      { 
        // otherwise free used memory
        free(pipeline);
        freeJobCommands(job_commands);
        free(infile);
        free(outfile);
        continue;
      }
    }
    else
    {
      for (int i = 0; i < command_count; i++)
      {

        child_pids[i] = fork();
        if (child_pids[i] < 0)
        {
          perror("fork");
          exit(EXIT_FAILURE);
        }

        if (child_pids[i] == 0)
        {
          if (interactive)
          {
            setpgid(child_pids[i], child_pids[0]);
          }
          // if it is the first child (i==0) redirect stdin to infile's fd, if specified
          // otherwise redirect stdin to fd[i-1][0] (read end)
          if (i == 0)
          {
            int new_stdin = infile ? open(infile, O_RDONLY) : STDIN_FILENO;
            dup2(new_stdin, STDIN_FILENO);
          }

          // If last child, (i == command_count - 1) redirect stdout to outfile's fd, if specified
          // otherwise, redirect stdout to fd[i][1] (write end) 
          if (i == command_count - 1)
          {
            int flags = append ? O_CREAT | O_APPEND : O_CREAT | O_TRUNC | O_WRONLY;
            int new_stdout = outfile ? open(outfile, flags, 0644) : STDOUT_FILENO;
            dup2(new_stdout, STDOUT_FILENO);
          }
          if (command_count > 1)
          {
            if (i > 0)
            {
              dup2(pipes[i - 1].fd[0], STDIN_FILENO);
            }

            if (i < command_count - 1)
            {
              dup2(pipes[i].fd[1], STDOUT_FILENO);
            }
          }

          // once redirections are complete, close all pipe fds
          for (int j = 0; j < command_count - 1; j++)
          {
            close(pipes[j].fd[0]);
            close(pipes[j].fd[1]);
          }

          // execute user command and catch error
          execvp(job_commands[i][0], job_commands[i]);
          perror("execvp");
          exit(EXIT_FAILURE);
        }
        else
        {
          // in parent, give terminal control to child, if it is running as a foreground process
          if (interactive)
          {
            setpgid(child_pids[i], child_pids[0]);
            if (!isBackgroundJob())
            {
              tcsetpgrp(STDIN_FILENO, child_pids[0]);
              signal(SIGTTOU, SIG_IGN);
            }
          }
        }
      }
    }
    // in parent, close unused fds and wait for all children to terminate
    int status = -1;
    if (child_pids[0] > 0)
    {
      if (is_really_background())
      {
        // if a background job, append it to the linked list
        bg_count++;
        fprintf(stderr, "Running: %s\n", pipeline);
        appendToEnd(&bg_head, &bg_tail, job_id++, child_pids, child_pids[0], command_count, pipeline, 0);
      }

      // close all pipes
      for (int i = 0; i < command_count - 1; i++)
      {
        close(pipes[i].fd[0]);
        close(pipes[i].fd[1]);
      }
      free(pipes);
      for (int i = 0; i < fg_count; i++)
      {
        int pid = waitpid(-child_pids[0], &status, WUNTRACED);

        if (pid < 0)
        {
          perror("waitpid");
        }
      }

      if (WIFSTOPPED(status))
      {
        // if job was stopped, add it to the linked list to be retrieved  
        // later by fg/bg
        fprintf(stderr, "\nStopped: %s\n", pipeline);
        appendToEnd(&bg_head, &bg_tail, job_id++, child_pids, child_pids[0], fg_count, pipeline, status);
        current_job = bg_tail;
      }
      else if (WIFSIGNALED(status))
      {
        // if received a signal, print a newline
        fprintf(stderr, "\n");
        free(pipeline);
      }
      else if WIFEXITED (status)
      {
        // do nothing if child exited normally
        free(pipeline);
      }
      tcsetpgrp(STDIN_FILENO, getpgid(0));
    }
    // free all used memory
    free(child_pids);
    freeJobCommands(job_commands);
    free(infile);
    free(outfile);
  }
}

int main(int argc, char **argv)
{
  bool isasync = false;
  if ((argc > 2) || (argc == 2 && (strcmp(argv[1], "--async") != 0)))
  {
    write_err_check("USAGE: ./penn-shell [--async]\n");
    exit(EXIT_FAILURE);
  }

  if (argv[1])
  {
    isasync = true;
  }
  execute_shell(isasync);
  return 0;
}
