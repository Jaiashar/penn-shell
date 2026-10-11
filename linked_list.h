#ifndef LINKED_LIST_H
#define LINKED_LIST_H
#include "linked_list.h"
#include <sys/types.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

// definition of a node in the doubly linked list that has references to the head and tail
typedef struct Node
{
  char *pipeline;
  int job_id;
  pid_t pgid;
  pid_t *pids;
  int commandCount;
  int finished, status;
  bool changed;
  struct Node *next, *prev;
} Node;

Node *createNode(int job_id, pid_t *pids, pid_t pgid, int commandCount, char *pipeline, int status);

void appendToEnd(Node **head, Node **tail, int job_id, pid_t *pids, pid_t pgid, int commandCount, char *pipeline, int status);

Node * removeNode(Node **head, Node **tail, Node *node);

bool isPidInJob(Node *node, pid_t pid);

Node * getCurrentJob(Node *head, Node *tail) ; ;
Node * findJob(Node * head, int job_id) ;
 
void printChangedStatuses(Node **head, Node **tail);
void printJobs(Node *head, Node *tail) ;

void updateStatus(Node **head, Node **tail, pid_t pid, int status);
#endif