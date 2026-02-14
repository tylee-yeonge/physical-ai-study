# Git 워크플로우

## 커밋
- 커밋 메시지는 영어로 작성
- Conventional Commits 형식: feat:, fix:, docs:, refactor:, test:
- 명시적 요청 없이 커밋하지 않기
- git push도 명시적 요청 없이 실행하지 않기

## 안전
- force push, reset --hard 등 파괴적 명령은 반드시 확인 후 실행
- .env, credentials 등 민감 파일은 커밋하지 않기
- amend 대신 새 커밋 생성 (명시적 요청 제외)
