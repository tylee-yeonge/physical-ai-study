# 코드 스타일 (공통)

## 네이밍 (Google Style 기준)
- 변수, 함수: snake_case
- 클래스, 구조체, 타입: CamelCase
- 상수: kConstantName
- 멤버 변수: trailing underscore (member_var_)

## 주석
- 코드 주석은 한국어로 작성
- 코드가 자명하더라도 주석을 작성 — 내용을 모르는 사람도 주석만 보고 파악할 수 있도록
- Python: Google style docstring (Args, Returns, Raises) 필수
- C++ 헤더: Doxygen 형식 주석 필수 (@brief, @param, @return)

## 파일 관리
- 새 파일 생성은 최소한으로 — 기존 파일 수정 우선
- README/문서 파일은 명시적 요청 시에만 생성
