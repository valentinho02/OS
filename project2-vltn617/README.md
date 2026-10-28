# Project 2 — xv6 커널 스레드

Linux의 clone 모델을 단순화해, 실행 단위와 주소 공간을 분리하고 여러 커널 스레드가 하나의 주소 공간을 공유하도록 구현한 과제입니다. Project 1과는 독립된 xv6 소스 트리입니다.

## 설계와 구현

| 위치 | 역할 |
| --- | --- |
| `kernel/proc.h` | 제공된 `mm_struct`, 그룹 리더, tgid, 개별 trapframe 가상 주소 정의 |
| `kernel/proc.c` | `mm_get/mm_put`, trapframe 슬롯 탐색, `kclone/kjoin`, 그룹 종료와 자원 회수 |
| `kernel/exec.c` | 리더만 exec 허용, 형제 스레드 정리 후 주소 공간 교체 |
| `user/uthread.c`, `user/uthread.h` | 사용자 스택 할당·반환 및 스레드 라이브러리 |
| `kernel/trampoline.S`, `kernel/trap.c` | 제공된 sscratch 기반 개별 trapframe 전환 경로 |

`CLONE_VM`이 있으면 부모의 `mm_struct`를 공유하고 참조 수를 늘립니다. 각 실행 단위에는 별도 trapframe 슬롯을 매핑합니다. 플래그가 없는 clone 경로는 주소 공간을 복사해 새 프로세스를 만듭니다. `mm_put()`의 마지막 참조가 사라지면 공유 주소 공간을 회수합니다.

`join()`은 같은 그룹에서 종료된 스레드를 회수하고 사용자 스택의 시작 주소를 돌려줍니다. 라이브러리는 이 주소로 `free()`를 호출합니다. 리더의 exit/exec는 `drain_siblings()`로 형제 스레드를 종료·회수합니다. kill은 그룹 전체에 적용하고, wait는 자식 그룹 리더만 대상으로 합니다. 비리더의 exec는 실패합니다.

## API와 사용 예

```c
int clone(void (*fn)(void *), void *arg, void *stack, int n_pages, int flags);
int join(void **stack);
int thread_create(void (*fn)(void *), void *arg, int n_pages);
int thread_join(void);
```

`user/uthread.h`를 포함해 `thread_create(worker, arg, 1)`로 생성하고 `thread_join()`으로 종료된 스레드를 회수합니다. worker는 작업을 끝낸 뒤 `exit(0)`으로 종료해야 합니다. 생성 실패는 -1입니다. 각 스레드는 고유 PID를 가지며, 파일 디스크립터 테이블과 cwd는 별도로 복제됩니다. 주소 공간 공유가 POSIX pthread의 모든 동작을 의미하지는 않습니다.

## 확인 방법과 구현 경험

xv6 셸에서 `threadtest`를 실행합니다. 생성/join, 공유 메모리, PID 차이, trapframe 슬롯 재사용, 스레드 수 제한, fd 복제, 비리더 exec 실패, 그룹 kill을 확인합니다. 출력의 개별 PASS/FAIL을 확인해야 하며 `ALL TESTS DONE`은 테스트 종료 표시입니다. 일반 회귀 검사는 `usertests -q`입니다. CPUS=1과 CPUS=3 양쪽에서 확인하는 것이 좋습니다.

보고서는 trapframe 매핑 해제로 메모리 누수를 해결한 과정과, 단일 코어에서만 동작하던 락 순서 문제를 정리한 과정을 기록합니다. 보고서의 실행 화면은 과제 당시 기록이며 이번 문서 작성 중 재검증 결과와 구분합니다.

## 빌드 및 실행

RISC-V GCC/binutils, QEMU 7.2 이상(`qemu-system-riscv64`), make, 호스트 C 컴파일러와 Perl이 필요합니다. 자동 테스트 도우미에는 Python 3도 필요합니다.

```sh
make qemu CPUS=1
```

다중 코어 실행은 `make qemu CPUS=3`으로 시작합니다. QEMU 종료는 `Ctrl-a`를 누른 뒤 `x`입니다. 도구 접두사를 자동으로 찾지 못하면 `make qemu TOOLPREFIX=riscv64-unknown-elf- CPUS=1`처럼 지정합니다.

기존 `README`는 xv6 원본 소개 및 출처이며, `LICENSE`와 함께 보존했습니다. 이 `README.md`는 과제 가이드라인, 작성한 보고서, 현재 소스를 대조해 정리한 프로젝트 설명입니다.

## 이번 정리 작업의 검증 (2026-10-28)

RISC-V GCC/binutils와 QEMU를 사용해 `make kernel/kernel fs.img` 빌드에 성공했습니다. CPUS=1에서 `threadtest`에서 개별 FAIL 없이 ALL TESTS DONE과 셸 복귀를 확인했습니다. 전체 `usertests -q`와 다중 코어 회귀 검사는 이번 작업에서는 재실행하지 않았습니다.
