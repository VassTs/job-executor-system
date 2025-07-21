<h1 align="center">Job Executor System: Named Pipes & Multithreaded Network Implementations</h1>

## Overview
This repository contains two implementations of the jobExecutor system:
- `jobExecutor‑named‑pipes`  
  Implements inter-process communication via named pipes and UNIX signals.
- `jobExecutor‑multithreaded`  
  Implements a multithreaded networked server using sockets, threads, mutexes, condition variables, and semaphores, with no busy-waiting.

## Detailed description
Two processes collaborate:
- **jobCommander**: Sends commands to the server.
- **jobExecutorServer**: Receives commands, schedules jobs, and executes them via `fork()`/`exec*()`.


### Named Pipes Implementation
**Supported Commands**:

| Command                        | Description                                       | Server Reply                             |
| ------------------------------ | ------------------------------------------------- | ---------------------------------------- |
| `issueJob <linux command>`     | Submit a job                                      | `<jobID,job,queuePosition>`              |
| `setConcurrency <N>`           | Set max concurrent jobs                           | *(none)*                                 |
| `stop <jobID>`                 | Cancel/terminate job                              | `job_XX removed` or `job_XX terminated`  |
| `poll running`/`poll queued`   | List running/queued jobs                          | One `<jobID,job,queuePosition>` per line |
| `exit`                         | Terminate server                                  | `jobExecutorServer terminated`           |

> Note: jobCommander’s lifecycle ends as soon as it sends a command and (where applicable) receives the server’s response.

**Demo**:
```bash
cd jobExecutor-named-pipes
make quick
./scripts/test_jobExecutor_1.sh
```

### Multithreaded Network Implementation
**Supported Commands**:

| Command                    | Description                                                              | Server Reply                                                                                                                                                |
| -------------------------- | ------------------------------------------------------------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `issueJob <linux command>` | Submit a job.                                                            | `JOB <jobID, job> SUBMITTED` + job output wrapped in start/end markers |
| `setConcurrency <N>`       | Set max worker threads                                                   | `CONCURRENCY SET AT <N>`                                                                                                                                    |
| `stop <jobID>`             | Remove queued job                                                        | `JOB <jobID> REMOVED` or `JOB <jobID> NOTFOUND`                                                                                                             |
| `poll`                     | List queued jobs                                                         | One `<jobID,job>` per line                                                                                                                                  |
| `exit`                     | Graceful shutdown of server                                              | `SERVER TERMINATED`                                                                                                                                         |

> Note: jobCommander’s lifecycle ends once its single command is sent and all necessary responses (including job output for issueJob) are received.

**Demo**:
```bash
# Terminal 1 (Server):
cd jobExecutor-multithreaded
make
./bin/jobExecutorServer 2034 8 5                                    # [port] [buffer] [threads]

# Terminal 2 (Client):
./bin/jobCommander localhost 2034 setConcurrency 2 
```

## License
This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.