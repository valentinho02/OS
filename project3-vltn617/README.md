# Project 3 — Memory-Mapped Files

파일을 사용자 가상 주소 공간에 매핑하고, 실제 접근 시 페이지를 읽는 지연 할당 방식의 `mmap/munmap`을 구현한 xv6 과제입니다. 다른 프로젝트와 독립된 소스 트리입니다.

## 핵심 흐름

1. `sys_mmap()`이 인자를 검증하고 16개 VMA 슬롯 중 하나에 매핑 정보를 기록합니다. 주소는 TRAPFRAME 아래에서 아래 방향으로 배정하며 현재 힙과 겹치는 요청을 거절합니다. 즉시 물리 페이지를 할당하거나 파일을 읽지는 않습니다.
2. 첫 접근에서 `kernel/trap.c`의 폴트 경로가 `mmapfault()`를 호출합니다. 해당 VMA를 찾아 페이지를 0으로 초기화하고 파일 내용을 읽은 뒤 사용자 페이지 테이블에 연결합니다.
3. `sys_munmap()`은 MAP_SHARED의 실제 매핑된 페이지를 파일에 기록하고 물리 페이지를 반환합니다. VMA 전체 또는 앞/뒤 일부 해제만 지원하며 중간 구멍은 거절합니다.
4. `kfork()`는 VMA 메타데이터와 파일 참조를 복제합니다. 자식의 mmap 페이지는 별도로 지연 로딩합니다. `kexit()`는 남은 VMA를 기록·해제합니다.

주요 파일은 `kernel/sysfile.c`(세 핵심 함수), `kernel/proc.c`(fork/exit), `kernel/proc.h`(VMA), `kernel/trap.c`(폴트 연결), `kernel/fcntl.h`(권한/모드), `user/mmaptest.c`(테스트)입니다.

## API와 범위

```c
char *mmap(int len, int prot, int flags, int fd, int offset);
int munmap(void *addr, int len);
```

- 길이는 양수이며 4096바이트 페이지 단위입니다. offset은 0 이상이며 페이지 단위여야 합니다. munmap의 주소도 페이지 정렬이어야 합니다.
- 권한은 `PROT_READ`, `PROT_WRITE` 또는 둘의 조합입니다. `PROT_NONE`, `PROT_EXEC`는 지원하지 않습니다.
- 모드는 `MAP_SHARED` 또는 `MAP_PRIVATE` 하나를 지정합니다. 오류는 -1이며 mmap 포인터의 실패는 `(char *)-1`과 비교합니다.
- `filedup()`으로 매핑의 파일 참조를 유지하므로 fd를 close해도 매핑은 유지됩니다.
- MAP_PRIVATE 변경은 파일에 기록하지 않습니다. MAP_SHARED는 해제/종료 시 기록하며, 프로세스 사이의 동일 물리 페이지 공유를 구현한 것은 아닙니다.
- dirty bit를 선별하지 않고 현재 존재하는 페이지를 기록합니다. RISC-V 쓰기 PTE에는 읽기 비트도 함께 설정합니다.

## 확인 방법과 구현 경험

xv6 셸에서 `mmaptest`와 `usertests -q`를 각각 실행합니다. 목표 출력은 `mmaptest: all tests succeeded`와 `ALL TESTS PASSED`입니다.

보고서는 이미 매핑된 페이지에서 다시 발생한 폴트를 거절해 remap 패닉을 막는 처리, 해제 범위 오버플로우 검사, 중간 구멍 요청 거절, 부분 해제 시 offset 갱신을 설명합니다. 보고서에는 전체 테스트 통과 결과가 기록되어 있으며, 과제 당시 결과와 이번 재실행 결과는 구분합니다.

## 빌드 및 실행

RISC-V GCC/binutils, QEMU 7.2 이상(`qemu-system-riscv64`), make, 호스트 C 컴파일러와 Perl이 필요합니다. 자동 테스트 도우미에는 Python 3도 필요합니다.

```sh
make qemu CPUS=1
```

다중 코어 실행은 `make qemu CPUS=3`으로 시작합니다. QEMU 종료는 `Ctrl-a`를 누른 뒤 `x`입니다. 도구 접두사를 자동으로 찾지 못하면 `make qemu TOOLPREFIX=riscv64-unknown-elf- CPUS=1`처럼 지정합니다.

기존 `README`는 xv6 원본 소개 및 출처이며, `LICENSE`와 함께 보존했습니다. 이 `README.md`는 과제 가이드라인, 작성한 보고서, 현재 소스를 대조해 정리한 프로젝트 설명입니다.

## 이번 정리 작업의 검증 (2026-10-28)

RISC-V GCC/binutils와 QEMU를 사용해 `make kernel/kernel fs.img` 빌드에 성공했습니다. CPUS=1에서 `mmaptest`에서 mmaptest: all tests succeeded와 셸 복귀를 확인했습니다. 전체 `usertests -q`와 다중 코어 회귀 검사는 이번 작업에서는 재실행하지 않았습니다.
