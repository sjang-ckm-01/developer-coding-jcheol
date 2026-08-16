# 과제 보고서

용량이 고정된 LRU 저장소를 `get`/`put` 평균 O(1)로 구현한 정적 라이브러리입니다.
기존 LRU 구현을 상속·래핑하지 않고 해시맵과 이중 연결 리스트를 직접 조합했습니다.

## 환경 의존성

**필수**

| 도구 | 최소 버전 | 검증 환경 | 비고 |
|---|---|---|---|
| C++ 컴파일러 | GCC 11+ | GCC 13.3.0 | CMake가 `cxx_std_23`을 요구 |
| CMake | 3.22 | 3.29.5 | Ubuntu 22.04 LTS 기본값에 맞춤 |
| GNU Make | — | 4.3 | 표준 기능만 사용 |
| Git | — | 2.43.0 | 아래 참조 |

컴파일러 하한이 GCC 11인 것은 CMake가 `cxx_std_23`을 요구하기 때문입니다.
**실제 코드가 쓰는 최신 기능은 `std::unordered_map::extract`(C++17)까지**로,
`-std=c++17`로도 라이브러리와 테스트가 모두 컴파일됩니다. `cxx_std_17`로 낮추면
GCC 7+까지 지원 범위가 넓어집니다.

**최초 빌드 시 네트워크 필요.** CMake FetchContent가 GoogleTest(v1.17.0)와
Google Benchmark(v1.9.5)를 GitHub에서 내려받습니다. 라이브러리 자체는 외부
의존성이 없으며, 테스트·벤치마크를 빌드할 때만 필요합니다.

**선택**

| 도구 | 검증 환경 | 용도 | 없을 때 |
|---|---|---|---|
| gcovr | 7.0 | `make coverage` | 커버리지 측정만 불가 |
| clang-format | 22.1.8 | `make fmt`, `make fmt-check` | 포맷 검사만 불가 |

## make 명령

| 명령 | 동작 |
|---|---|
| `make` | 라이브러리·테스트·벤치마크 빌드 (Release) |
| `make test` | 빌드 후 ctest로 테스트 실행 |
| `make test-asan` | AddressSanitizer + UBSan 빌드로 테스트 실행 |
| `make coverage` | 커버리지 계측 빌드 후 라인/분기 커버리지 리포트 |
| `make bench` | 벤치마크 실행 |
| `make fmt` | 소스 포맷 적용 |
| `make fmt-check` | 포맷 위반 시 실패 (트리 변경 없음) |
| `make clean` | 빌드 디렉터리 삭제 |

## 프로젝트 구조

```
storagecache/
  include/storagecache/storagecache.h   공개 헤더
  src/storagecache.cpp                  구현
  src/storagecache_internal.h           내부 자료구조 (비공개)
tests/                                  GoogleTest 테스트
bench/                                  Google Benchmark 벤치마크
```

공개 헤더는 불완전 타입 `StorageCache`만 노출하고 내부 자료구조는 감춥니다.
소비자는 `#include <storagecache/storagecache.h>`로 사용하며, CMake에서는
`add_subdirectory` 후 `storagecache::storagecache`를 링크하면 됩니다. 이때
테스트·벤치마크와 그 의존성은 빌드되지 않습니다.

## 설계

`std::unordered_map<int, iterator>`와 `std::list<std::pair<int,int>>`를 함께 씁니다.
맵이 키에서 리스트 노드로 바로 이어주고, 리스트는 사용 순서를 유지합니다.
리스트 앞이 가장 오래된 항목(축출 대상), 뒤가 가장 최근 항목입니다.

O(1)의 근거는 **리스트 노드를 재배치할 때 `splice`만 쓴다**는 점입니다. `splice`는
포인터만 다시 걸므로 할당이 없고, 표준이 이터레이터 무효화를 보장하지 않으므로
맵에 저장된 이터레이터를 그대로 재사용할 수 있습니다. 덕분에 조회·갱신 경로에서
할당이 0회이고, 축출 경로도 `unordered_map::extract`로 맵 노드까지 재활용해 0회입니다.

`put`의 세 경로(갱신·축출·신규 삽입)는 모두 던질 수 있는 연산을 먼저 끝낸 뒤
실패하지 않는 `splice`로만 리스트를 건드립니다. 할당 실패가 나도 자료구조가
깨지지 않습니다.

## 검증 결과

| 항목 | 결과 |
|---|---|
| 테스트 | 17개 전부 통과 |
| AddressSanitizer + UBSan | 오류 없음 |
| 커버리지 | 라인 96.6%, 함수 100%, 분기 96.8% |
| 컴파일 경고 | `-Wall -Wextra -Werror`로 전 타깃 빌드 |

커버리지에서 빠진 2줄은 `createCache`의 할당 실패 처리 블록입니다. 스위트 안에서
할당 실패를 재현하려면 전역 `operator new`를 교체해야 하는데, 그러면 GoogleTest
내부 할당과 짝이 어긋나 ASan이 중단됩니다. ASan 검사를 끄는 대신 이 경로는
별도 프로그램으로 확인했습니다.
