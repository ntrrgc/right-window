//
// Created by ntrrgc on 24/01/16.
//

#include "subprocess.h"
#include <stdexcept>
#include <unistd.h>
#include <sstream>
#include <sys/wait.h>

// NOTE: This implementation is UNIX specific but it's currently only needed for BSPWM.

using namespace std;

namespace subprocess {

string check_output(std::vector<string> args) {
    int pipe_fd[2];
    if (pipe(pipe_fd) != 0) {
        perror("pipe");
        exit(2);
    }

    int pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(2);
    }
    if (pid == 0) {
        // I'm the child
        close(pipe_fd[0]);

        char **args_raw = new char *[args.size() + 1];
        for (size_t i = 0; i < args.size(); ++i) {
            args_raw[i] = const_cast<char *>(args[i].c_str());
        }
        args_raw[args.size()] = nullptr;

        // Close parent files and substitute stdout with an end of the pipe
        close(STDIN_FILENO);
        close(STDERR_FILENO);
        close(STDOUT_FILENO);
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);

        execvp(args[0].c_str(), args_raw);
        throw std::runtime_error("Could not launch subprocess");
    } else {
        // I'm the parent
        close(pipe_fd[1]);

        vector<char> output;
        char buf[4096];
        ssize_t ret;
        while (0 != (ret = read(pipe_fd[0], buf, sizeof(buf)))) {
            if (ret < 0 && errno != EINTR) {
                perror("read");
                exit(2);
            } else if (ret > 0) {
                // Append to output
                output.insert(output.end(), buf, buf + ret);
            }
        }

        close(pipe_fd[0]);
        close(pipe_fd[1]);

        int status;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            exit(2);
        }

        if (WEXITSTATUS(status) == 0) {
            // Successful exit
            output.push_back('\0');
            return string(output.begin(), output.end());
        } else {
            throw std::runtime_error("Subprocess failed");
        }
    }
}

void call(std::vector<std::string> args) {
    check_output(args);
}

}