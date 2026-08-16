# 과제 보고서

## 환경 의존성

**필수**

| 도구 | 최소 버전 | 검증 환경 | 비고 |
|---|---|---|---|
| C++ 컴파일러 | GCC 11 | GCC 13.3.0 | C++17 사용. GCC 11은 Ubuntu 22.04 LTS 기본값 |
| CMake | 3.22 | 3.29.5 | Ubuntu 22.04 LTS 기본값 |
| GNU Make | — | 4.3 | 표준 기능만 사용 |
| Git | — | 2.43.0 | 아래 참조 |

기준선은 **Ubuntu 22.04 LTS 기본 도구 체인**입니다. 언어 표준은 C++17이며,
코드가 쓰는 가장 최신 기능은 `std::unordered_map::extract`(C++17)입니다.

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
