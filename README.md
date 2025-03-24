대한상공회의소 AI 시스템 반도체 SW 개발자 과정 - MINI Project

Atmega128A를 활용한 세탁기 프로젝트

재작기간 : 2025-03-14 ~ 2025-03-17

---

1. FSM
2. 회로도
3. 기능
4. 설계

---

# 1. FSM

![1](https://github.com/user-attachments/assets/291e69b8-cecf-48de-a94f-9967aab6053d)

![2](https://github.com/user-attachments/assets/f611fdf1-c86e-4dfb-9b45-5fa9e85232a6)


---

# 2. 회로도

![image](https://github.com/user-attachments/assets/a0bf6876-bad5-4ed9-95d9-e3cb1e9c6f8d)



![image](https://github.com/user-attachments/assets/637fb4c0-795c-494a-bd9d-9ae1f2cddcfe)


---

# 3. 기능

**Step1** :  세탁기 대기모드 ON, (FND1 : 0000,  FND2 : STOP) 

**Step2** : 세탁 시간 설정 단계, FND2에 “SELT”가 출력된다.

1. 버튼 0 : 대기모드 복귀
2. 버튼  1 :  숫자 UP(10분 단위)
3. 버튼  2 :  숫자 DOWN(10분 단위)
4. 버튼  3 :  시간 저장

세탁 → 헹굼 → 탈수 시간을 저장할 수 있다. 버튼 3을 누르지 않는다면 default값으로 10, 10, 10으로 저장되도록 설정

**Step3** : 세탁 단계, 모터의 회전 방향에 따라 FND2에 애니메이션이 출력된다. FND1에서는 설정된 시간에 따라 시간이 카운트 다운된다.

1. 버튼 0 : 대기모드 복귀
2. 버튼 2 : 세탁 → 헹굼 → 탈수

탈수가 완료됬다면 PIEZO로 알림이 울린다.

## UART 통신

UART로 수신된 명령을 처리한다. 명령이 큐에 들어오면, 이를 읽고 다음과 같은 동작을 수행한다.

1. **"start" 명령**: 세탁을 시작하며, 시간이 0일 경우 기본 세탁 시간으로 설정한다. 모터를 켜고, 시작된 시간 정보를 UART로 전송.
2. **"stop" 명령**: 세탁을 일시 정지하고, 모터를 끄며, 남은 시간을 UART로 전송.

각 명령이 처리될 때마다 명령 큐에서 처리된 명령은 제거

---

# 설계

1. **타이머 0** : 8비트, 1MS 단위 시간 측정을 위해 사용

1ms 마다 overflow 인터럽트 발생 시키기 위해,  분주비(prescaler) =  64, TCNT = 6으로 설정.

1. **타이머 3** : 모터 PWM 제어

모터의 속도 조절을 위해 PWM 신호를 생성하는 담당을 한다.
