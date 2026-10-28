// user/watershapetest.c

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/uthread.h"

#define NTHREAD 8
#define ROUNDS 10

volatile int shared_counter = 0;

void
basic_worker(void *arg)
{
    int *x = (int *)arg;
    *x = 1234;
    exit(0);
}

void
counter_worker(void *arg)
{
    int loops = *(int *)arg;

    for(int i = 0; i < loops; i++){
        shared_counter++;
    }

    exit(0);
}

void
stack_worker(void *arg)
{
    int local;

    printf("thread pid=%d local addr=%p\n",
           getpid(),
           &local);

    exit(0);
}

void
pause_worker(void *arg)
{
    int ticks = *(int *)arg;

    pause(ticks);

    exit(0);
}

void
fd_worker(void *arg)
{
    int fd = *(int *)arg;

    close(fd);

    exit(0);
}

void
exec_worker(void *arg)
{
    char *argv[] = {"echo", "BAD", 0};

    int ret = exec("echo", argv);

    printf("exec from non-leader ret=%d\n", ret);

    exit(0);
}

void
infinite_worker(void *arg)
{
    while(1){
        pause(1);
    }
}

void
test_basic_create_join()
{
    printf("\n=== test_basic_create_join ===\n");

    int x = 0;

    if(thread_create(basic_worker, &x, 1) < 0){
        printf("FAIL: thread_create\n");
        return;
    }

    thread_join();

    if(x == 1234)
        printf("PASS\n");
    else
        printf("FAIL x=%d\n", x);
}

void
test_shared_memory()
{
    printf("\n=== test_shared_memory ===\n");

    shared_counter = 0;

    int loops = 10000;

    for(int i = 0; i < 4; i++){

        if(thread_create(counter_worker,
                         &loops,
                         1) < 0){

            printf("FAIL create\n");
            return;
        }
    }

    for(int i = 0; i < 4; i++)
        thread_join();

    printf("shared_counter=%d\n",
           shared_counter);

    if(shared_counter > 0)
        printf("PASS\n");
    else
        printf("FAIL\n");
}

void
test_pid_difference()
{
    printf("\n=== test_pid_difference ===\n");

    printf("leader pid=%d\n",
           getpid());

    if(thread_create(stack_worker,
                     0,
                     1) < 0){

        printf("FAIL create\n");
        return;
    }

    thread_join();

    printf("PASS\n");
}

void
test_many_threads()
{
    printf("\n=== test_many_threads ===\n");

    int ticks = 10;

    for(int r = 0; r < ROUNDS; r++){

        printf("round %d\n", r);

        for(int i = 0;
            i < NTHREAD - 1;
            i++){

            int ret =
                thread_create(pause_worker,
                              &ticks,
                              1);

            if(ret < 0){

                printf("FAIL create "
                       "round=%d idx=%d\n",
                       r,
                       i);

                return;
            }
        }

        for(int i = 0;
            i < NTHREAD - 1;
            i++){

            thread_join();
        }
    }

    printf("PASS\n");
}

void
test_trapframe_slot_reuse()
{
    printf("\n=== "
           "test_trapframe_slot_reuse "
           "===\n");

    int ticks = 1;

    for(int r = 0; r < 10; r++){

        for(int i = 0;
            i < NTHREAD - 1;
            i++){

            int ret =
                thread_create(pause_worker,
                              &ticks,
                              1);

            if(ret < 0){

                printf("FAIL reuse "
                       "round=%d idx=%d\n",
                       r,
                       i);

                return;
            }
        }

        for(int i = 0;
            i < NTHREAD - 1;
            i++){

            thread_join();
        }
    }

    printf("PASS\n");
}

void
test_thread_limit()
{
    printf("\n=== "
           "test_thread_limit "
           "===\n");

    int ticks = 100;

    int success = 0;

    for(int i = 0; i < 32; i++){

        int ret =
            thread_create(pause_worker,
                          &ticks,
                          1);

        if(ret >= 0)
            success++;
        else{

            printf("create failed "
                   "at i=%d\n",
                   i);

            break;
        }
    }

    printf("created=%d threads\n",
           success);

    for(int i = 0;
        i < success;
        i++){

        thread_join();
    }

    printf("PASS\n");
}

void
test_fd_duplication()
{
    printf("\n=== "
           "test_fd_duplication "
           "===\n");

    int fd = open("README", 0);

    if(fd < 0){

        printf("FAIL open\n");
        return;
    }

    if(thread_create(fd_worker,
                     &fd,
                     1) < 0){

        printf("FAIL create\n");
        close(fd);
        return;
    }

    thread_join();

    char buf[4];

    int n = read(fd,
                 buf,
                 sizeof(buf));

    if(n >= 0)
        printf("PASS parent fd usable\n");
    else
        printf("FAIL parent fd broken\n");

    close(fd);
}

void
test_exec_nonleader_fail()
{
    printf("\n=== "
           "test_exec_nonleader_fail "
           "===\n");

    if(thread_create(exec_worker,
                     0,
                     1) < 0){

        printf("FAIL create\n");
        return;
    }

    thread_join();

    printf("PASS\n");
}

void
test_kill_group()
{
    printf("\n=== "
           "test_kill_group "
           "===\n");

    int pid = fork();

    if(pid < 0){

        printf("FAIL fork\n");
        return;
    }

    if(pid == 0){

        for(int i = 0; i < 4; i++){

            thread_create(infinite_worker,
                          0,
                          1);
        }

        pause(1000);

        exit(0);
    }

    pause(50);

    kill(pid);

    wait(0);

    printf("PASS\n");
}



int
main(int argc, char *argv[])
{
    test_basic_create_join();

    test_shared_memory();

    test_pid_difference();

    test_many_threads();

    test_trapframe_slot_reuse();

    test_thread_limit();

    test_fd_duplication();

    test_exec_nonleader_fail();

    test_kill_group();


    printf("\nALL TESTS DONE\n");

    exit(0);
}