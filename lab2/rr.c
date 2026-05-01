#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process
{
  u32 pid;
  u32 arrival_time;
  u32 burst_time;

  TAILQ_ENTRY(process) pointers;

  /* Additional fields here */

  int remaining_time;             //amount of time remaining in the task. <= 0 if completed
  u32 first_schedule_time;        //time at which this process was first given CPU time
  u32 completed_time;             //time at which this process finished its task

  bool has_started;               //true if given any CPU time already, false otherwise
  bool completed;                 //true if time remaining <= 0, false otherwise
  bool found;                     //true if arrival time <= current time and it's been pushed to queue (prevents double pushes)

  /* End of "Additional fields here" */
};

TAILQ_HEAD(process_list, process);

u32 next_int(const char **data, const char *data_end)
{
  u32 current = 0;
  bool started = false;
  while (*data != data_end)
  {
    char c = **data;

    if (c < 0x30 || c > 0x39)
    {
      if (started)
      {
        return current;
      }
    }
    else
    {
      if (!started)
      {
        current = (c - 0x30);
        started = true;
      }
      else
      {
        current *= 10;
        current += (c - 0x30);
      }
    }

    ++(*data);
  }

  printf("Reached end of file while looking for another integer\n");
  exit(EINVAL);
}

u32 next_int_from_c_str(const char *data)
{
  char c;
  u32 i = 0;
  u32 current = 0;
  bool started = false;
  while ((c = data[i++]))
  {
    if (c < 0x30 || c > 0x39)
    {
      exit(EINVAL);
    }
    if (!started)
    {
      current = (c - 0x30);
      started = true;
    }
    else
    {
      current *= 10;
      current += (c - 0x30);
    }
  }
  return current;
}

void init_processes(const char *path,
                    struct process **process_data,
                    u32 *process_size)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1)
  {
    int err = errno;
    perror("open");
    exit(err);
  }

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    int err = errno;
    perror("stat");
    exit(err);
  }

  u32 size = st.st_size;
  const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    int err = errno;
    perror("mmap");
    exit(err);
  }

  const char *data_end = data_start + size;
  const char *data = data_start;

  *process_size = next_int(&data, data_end);

  *process_data = calloc(sizeof(struct process), *process_size);
  if (*process_data == NULL)
  {
    int err = errno;
    perror("calloc");
    exit(err);
  }

  for (u32 i = 0; i < *process_size; ++i)
  {
    (*process_data)[i].pid = next_int(&data, data_end);
    (*process_data)[i].arrival_time = next_int(&data, data_end);
    (*process_data)[i].burst_time = next_int(&data, data_end);
  }

  munmap((void *)data, size);
  close(fd);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    return EINVAL;
  }
  struct process *data;
  u32 size;
  init_processes(argv[1], &data, &size);

  u32 quantum_length = next_int_from_c_str(argv[2]);

  struct process_list list;
  TAILQ_INIT(&list);

  u32 total_waiting_time = 0;
  u32 total_response_time = 0;

  /* Your code here */

  u32 current_time = 0;
  u32 completed_processes = 0;

  //========== 0. INITIALIZE PROCESSES ==========
  for (u32 i = 0; i < size; i++) {
    data[i].remaining_time = data[i].burst_time;
    data[i].has_started = false;
    data[i].completed = false;
    data[i].found = false;
  }

  //iterate until all processes have been completed
  while (completed_processes < size) {
    //========== 1. ARRIVAL CHECK ==========
    //processes may not be in processes.txt in a sorted order-- so we must iterate through all processes every time
    for (u32 i = 0; i < size; i++) {
      if (data[i].arrival_time == current_time && data[i].found == false) {
        data[i].found = true;
        TAILQ_INSERT_TAIL(&list, &data[i], pointers);
      }
    }

    //========== 2. IDLE CHECK (wait for a process to run) ==========
    if (TAILQ_EMPTY(&list)) {
      current_time++;
      continue; 
    }

    //========== 3. DISPATCH ==========
    struct process *p = TAILQ_FIRST(&list);
    TAILQ_REMOVE(&list, p, pointers);

    //========== 4. START PROCESS ==========
    if (!p->has_started) {
      total_response_time += (current_time - p->arrival_time);
      p->first_schedule_time = current_time;
      p->has_started = true;
    }

    //========== 5. EXECUTION ==========
    u32 end_of_execution = current_time + quantum_length;
    while (current_time < end_of_execution) {

      /** Here p would do work on the CPU for one unit of time!! */

      p->remaining_time--;
      current_time++;

      // CHECK FOR ARRIVALS during the execution of this quantum
      for (u32 i = 0; i < size; i++) {
        if (data[i].arrival_time == current_time && data[i].found == false) {
          data[i].found = true;
          TAILQ_INSERT_TAIL(&list, &data[i], pointers);
        }
      }

      //check if our work is done
      if (p->remaining_time <= 0){
        p->completed = true;
        p->completed_time = current_time;
        break;
      }
    }

    //========== 6. COMPLETION OR PREEMPTION ==========
    if (p->completed) { //COMPLETION
      completed_processes++;
      total_waiting_time += (p->completed_time - p->arrival_time - p->burst_time); //time after arrival that was NOT spent on the CPU
    } else {  //PREEMPTION
      TAILQ_INSERT_TAIL(&list, p, pointers); //move to back of queue
    }
  }
  /* End of "Your code here" */

  printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
  printf("Average response time: %.2f\n", (float)total_response_time / (float)size);

  free(data);
  return 0;
}
