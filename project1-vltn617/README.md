# Project 1 — CFS 스케줄러

xv6 RISC-V의 Round Robin 스케줄러를 가상 실행 시간(`vruntime`) 기반의 단순화한 Completely Fair Scheduler로 교체한 운영체제 과제입니다.

## 핵심 구현

- `kernel/proc.h`: 프로세스별 `vruntime`, `nice`, Red-Black Tree 노드와 실행 시작 시각을 관리합니다.
- `kernel/proc.c`: `scheduler()`가 실행 큐에서 최소 `vruntime` 노드를 선택·제거합니다. `yield()`는 실행 시간을 반영한 후 노드를 재삽입합니다.
- `update_vruntime()`: 실행 tick에 1024를 곱하고 nice에 따라 비트 이동합니다. nice가 1 커지면 같은 실행 시간에 누적되는 가상 시간이 두 배가 됩니다.
- `kfork()`: 부모의 nice와 vruntime을 상속합니다. `wakeup()` 및 잠든 프로세스를 깨우는 `kkill()`에서는 RUNNABLE 최소값, RUNNING 최소값, 0 순서로 vruntime을 정합니다.
- `kernel/rbtree.c`, `kernel/rbtree.h`: 제공된 균형 트리로 실행 큐를 관리합니다. `cfs_lock`은 큐의 동시 변경을 보호합니다.
- `kernel/sysproc.c`, `kernel/syscall.c`, `kernel/syscall.h`, `user/usys.pl`, `user/user.h`: 시스템 콜을 연결합니다.

## API

```c
int set_nice(int nice);
```

현재 프로세스의 nice를 -3부터 2까지 설정합니다. 성공은 0, 범위를 벗어나면 1을 반환합니다. 최초 프로세스의 nice와 vruntime은 0입니다.

## 확인 방법과 구현 경험

xv6 셸에서 `cfstest`를 실행하면 nice 경계값, fork 및 다중 프로세스 스케줄링 관련 테스트를 확인할 수 있습니다. 일반 회귀 검사는 `usertests -q`를 실행합니다. nice 상속 테스트 일부는 실행 결과를 통한 간접 검사이므로, 내부 상태 상속의 완전한 증명으로 해석하지 않습니다.

보고서는 Red-Black Tree를 사용해 빈번한 삽입·삭제와 최소값 탐색을 효율화한 과정과 테스트 통과 결과를 설명합니다. 해당 결과는 과제 당시의 기록이며 이번 문서 작성 중 재실행한 결과와 구분합니다.

## 빌드 및 실행

RISC-V GCC/binutils, QEMU 7.2 이상(`qemu-system-riscv64`), make, 호스트 C 컴파일러와 Perl이 필요합니다. 자동 테스트 도우미에는 Python 3도 필요합니다.

```sh
make qemu CPUS=1
```

다중 코어 실행은 `make qemu CPUS=3`으로 시작합니다. QEMU 종료는 `Ctrl-a`를 누른 뒤 `x`입니다. 도구 접두사를 자동으로 찾지 못하면 `make qemu TOOLPREFIX=riscv64-unknown-elf- CPUS=1`처럼 지정합니다.

기존 `README`는 xv6 원본 소개 및 출처이며, `LICENSE`와 함께 보존했습니다. 이 `README.md`는 과제 가이드라인, 작성한 보고서, 현재 소스를 대조해 정리한 프로젝트 설명입니다.

## 이번 정리 작업의 검증 (2026-10-28)

RISC-V GCC/binutils와 QEMU를 사용해 `make kernel/kernel fs.img` 빌드에 성공했습니다. CPUS=1에서 `cfstest`에서 PASS 20 / FAIL 0 및 ALL TESTS PASSED를 확인했습니다. 전체 `usertests -q`와 다중 코어 회귀 검사는 이번 작업에서는 재실행하지 않았습니다.
