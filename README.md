https://github.com/Hyun-hyee/FF7Remake_Dx11_CopyGame_CodeSample

# Final Fantasy VII Remake 모작 팀 프로젝트


DirectX 11을 활용한 게임 모작 팀 프로젝트에서 기여한 영역의 샘플 코드입니다.
<br>
기술의 중요도, 코드 스타일 등의 기준으로 선별했습니다.

<br>

## 💻기술 설명 영상
[![Video Label](http://img.youtube.com/vi/oPwEHSDcpRA/0.jpg)](https://youtu.be/oPwEHSDcpRA)

## 📆제작 기간 & 참여 인원 
*  2024/02/13 ~ 2024/04/15 (2개월)
* 프로그래머 5명

<br>

## 👩‍💻파트 
* 팀장
* 프레임워크
* 플레이어
* AI, 충돌처리
* 렌더링(셰이더)
* 미니게임
* 라이팅
* NPC 

<br>

## 📖기술 스택 및 개발 환경
* DirectX11 SDK
* C++
* ImGui
* Assimp
* PhysX

<br>

## ❗중점 구현 사항
*  PhysX 라이브러리를 사용해 게임 내 모든 충돌 처리 구현
    * Dynamic Actor, Static Actor, Character Controller
    * Kinematic, Trigger 옵션 활용, Ray 충돌 구현
    * Collision Enter/Exit/Stay 함수 연결
    * 충돌 레이어 활용해 최적화
    * 충돌체 Sleep기능과 제거 기능 활용해 최적화
* 다양한 셰이더 구현
    * HBAO+/ 티 샘플링 / HDR / Radial Blur / PBR / Cascade Shadow
Volumetric Light / Tone Mapping / Effect Blur / HSV / Fog
* 플레이어 커맨드 전투 시스템 구현
* 플레이어 스위칭 전투 시스템 구현
* 행동 트리로 플레이어 AI 구현
    * 적의 ActionPower에 따른 패턴 다양화
    * 공격모드, 이동모드
* 댄스 미니게임 구현
* 동물 NPC 구현
* 라이트 배치

</div>
