#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <errno.h>

int main(int argc, char *argv[])
{
    if (argc <= 1) exit(EINVAL);

    pid_t pids[argc];
	pids[0] = getpid(); //argv[0] = "./pipe"

    int pipe_fds[2];	//this will be written over on every iter
    int prev_pipe_read_fd = 0;	//first prgm should read from stdin

	//should read (argc - 1) programs and start at argv[1], since argv[0] = "./pipe"
    for (int i = 1; i < argc; i++) {
        if (i != argc -1) {
			//create a pipe between this program and next program
            if (pipe(pipe_fds) < 0) {
				perror("pipe");
				exit(errno);
			}
        }

        pids[i] = fork();
        if (pids[i] == 0) {
            // CHILD PROCESS
			// hook up to previous pipe and this pipe, then start with execlp()

			// if not first program, route stdin to prev pipe read
			// otherwise, program will automatically read from stdin
            if (i > 1) {
                dup2(prev_pipe_read_fd, 0);
                close(prev_pipe_read_fd);
            }

			//if not last program, route stdout to pipe write
			//otherwise, program will automatically write to stdout
            if (i != argc - 1) {
                dup2(pipe_fds[1], 1);
				//pipe is only created/opened if not last program
                close(pipe_fds[0]);
                close(pipe_fds[1]);
            }
            
            execlp(argv[i], argv[i], (char*)NULL);

			//if code gets here, then execlp() failed..
            perror("exec failed");
            exit(1);
        } else if (pids[i] > 0) {
			// PARENT PROCESS
			// close pipes that child needed pointers to, but parent doesnt need anymore

			//if a previous pipe exists (other than stdin for first program)
			if (i > 1) {
				//parent doesn't ever care about read ends, but we wait
				//until our next child in the iteration can use it
				//(this is bc child copies parent's FDs from process table)
				close(prev_pipe_read_fd);
			}

			//if this pipe needs to be routed to the next program, set pointer to pipe read fd
			//(last program exempted, just sends to stdout)
			if (i != argc - 1) {
				close(pipe_fds[1]); //close so that reader sees EOF eventually (after program finishes)
				prev_pipe_read_fd = pipe_fds[0];
			}

		} else {
			perror("fork");
			exit(1);
		}
	}

    //all programs are running and reading/writing from their pipe buffers, wait for finish
    int status;
    for (int i = 1; i < argc; i++) {
        if (waitpid(pids[i], &status, 0) < 0){
			perror("waitpid");
			exit(1);
		}
        if (!WIFEXITED(status) || WEXITSTATUS(status)){
			perror(argv[i]);
			exit(1);
		}
    }

    return 0;
}