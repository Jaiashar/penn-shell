#include "linked_list.h"
#include <sys/types.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

Node *createNode(int job_id, pid_t *pids, pid_t pgid, int commandCount, char *pipeline, int status)
{
    // allocate memory and initialize variables
    Node *node = malloc(sizeof(Node));
    node->job_id = job_id;
    node->pids = malloc(commandCount * sizeof(pid_t));
    for (int i = 0; i < commandCount; i++)
        node->pids[i] = pids[i];
    node->pgid = pgid;
    node->pipeline = pipeline;
    node->next = node->prev = NULL;
    node->commandCount = commandCount;
    node->finished = 0;
    node->changed = false;
    node->status = status;
    return node;
}

void appendToEnd(Node **head, Node **tail, int job_id, pid_t *pids, pid_t pgid, int commandCount, char *pipeline, int status)
{
    // add a job to the end of the list
    Node *node = createNode(job_id, pids, pgid, commandCount, pipeline, status);
    if (!*head)
    {
        // create new lsit if empty
        *head = *tail = node;
        return;
    }
    else
    {
        (*tail)->next = node;
        node->prev = *tail;
        (*tail) = (*tail)->next;
        return;
    }
}

Node *removeNode(Node **head, Node **tail, Node *node)
{
    // remove a specific node from the linked list, updating the 
    // references in the neighboring nodes as well
    // we do not free the node, and leave it to the caller to 
    // free the node appropriately
    if (node->prev)
    {
        node->prev->next = node->next;
    }
    if (node->next)
    {
        node->next->prev = node->prev;
    }
    if (node == *head)
    {
        *head = node->next;
    }
    if (node == *tail)
    {
        *tail = node->prev;
    }
    node->next = node->prev = NULL;
    return node;
}

bool isPidInJob(Node *node, pid_t pid)
{
    // return true if the given pid is among the pids in the job
    pid_t *arr = node->pids;
    int cmd_count = node->commandCount;
    for (int i = 0; i < cmd_count; i++)
    {
        if (pid == arr[i])
            return true;
    }
    return false;
}

Node *getCurrentJob(Node *head, Node *tail)
{
    // If there are stopped jobs, the current job is the most recently
    // stopped one. Otherwise, it is the most recently created background job

    if (!head)
        return NULL;
    Node *node = tail;
    Node *candidate = NULL;
    while (node)
    {
        if (WIFSTOPPED(node->status))
        {
            //return last stopped job
            return node;
        }
        else
        {
            // hold on to last background job
            if (!candidate)
                candidate = node;
        }
        node = node->prev;
    }
    // if no stopped job, return most recent background job
    return candidate;
}

Node *findJob(Node *head, int job_id)
{
    // look for a specific job
    Node *node = head;
    while (node && node->job_id <= job_id)
    {
        if (node->job_id == job_id)
            return node;
        node = node->next;
    }
    return NULL;
}

void printChangedStatuses(Node **head, Node **tail)
{
    // print status if changed
    Node *node = *head;
    while (node)
    {
        Node *next = node->next;
        if (node->changed)
        {
            if (node->finished == node->commandCount)
            {
                fprintf(stderr, "Finished: %s\n", node->pipeline);
                Node *removed_node = removeNode(head, tail, node);
                free(removed_node->pids);
                free(removed_node->pipeline);
                free(removed_node);
            }
        }
        node = next;
    }
}

void printJobs(Node *head, Node *tail)
{
    // print statuses of jobs in the queue
    Node *node = head;
    char stopped[] = "stopped";
    char running[] = "running";
    while (node)
    {
        char *statusString = WIFSTOPPED(node->status) ? stopped : running;
        fprintf(stderr, "[%d] %s (%s)\n", node->job_id, node->pipeline, statusString);
        node = node->next;
    }
}

void updateStatus(Node **head, Node **tail, pid_t pid, int status)
{
    // update the job status in the list
    Node *node = *head;
    while (node)
    {
        if (isPidInJob(node, pid))
        {
            node->changed = true;
            if WIFEXITED (status)
            {
                node->finished++;
            }
            else
            {
                node->status = status;
            }
            break;
        }
        else
        {
            node = node->next;
        }
    }
}