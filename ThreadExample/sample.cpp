#include <windows.h>
#include <stdio.h>
#include <iostream>
using namespace std;

#define BUFSIZE 20

HANDLE Write_StartEvent_forThreadA;
HANDLE ReadWrite_StartEvent_forThreadB;
HANDLE Read_Compare_StartEvent_forThreadC;
CRITICAL_SECTION david_CS;// 구조체 객체 전역 선언

namespace David // 여러 스레드들에서 쓰기 위해, 전역 버퍼들 선언.
{
	char buffer1[BUFSIZE];
	char buffer2[BUFSIZE];
}

DWORD WINAPI  Write_ThreadA_Route(LPVOID arg)
{ 
	//가장 먼저 이게 실행됨.

	DWORD return_value;
	// 메인쓰레드에서 이 이벤트가 Signal 되기를 기다림. 그러면, 시작됨.
	
	return_value = WaitForSingleObject(Write_StartEvent_forThreadA, INFINITE);
	if (return_value == WAIT_FAILED) return 0;


	cout << "Thread1 proof" << endl;
	//공유버퍼1에 쓰기.

	EnterCriticalSection(&david_CS);
	
	for (int i = 0; i < BUFSIZE; i++)
	{
		David::buffer1[i] = 7;
	}

	LeaveCriticalSection(&david_CS);

	//쓰기 완료를 쓰레드B에 알림.
	SetEvent(ReadWrite_StartEvent_forThreadB);


	return 0;
}


DWORD WINAPI ReadWrite_ThreadB_Route(LPVOID arg)
{
	DWORD return_value;

		// 공유버퍼 1에 쓰기 완료를 기다림
		return_value = WaitForSingleObject(ReadWrite_StartEvent_forThreadB, INFINITE);
		if (return_value == WAIT_FAILED) return 0;

		cout << "Thread2 proof" << endl;

		EnterCriticalSection(&david_CS);

		// 공유버퍼 1의 내용을 읽어, 공유버퍼 2에 복사함.
		for (int i = 0; i < BUFSIZE; i++)
		{
			David::buffer2[i] = David::buffer1[i];
		}

		LeaveCriticalSection(&david_CS);
		
		// 이벤트의 State를 변경함으로써, 공유 버퍼2에 쓰기 완료를 알림.
		SetEvent(Read_Compare_StartEvent_forThreadC);
	
		return 0;
}


DWORD WINAPI Compare_Confirmation_ThreadC_Route(LPVOID arg)
{
	DWORD return_value;

	// 공유버퍼 2에 쓰기 완료를 기다림
	return_value = WaitForSingleObject(Read_Compare_StartEvent_forThreadC, INFINITE);
	if (return_value == WAIT_FAILED) return 0;

	// 공유버퍼1, 공유버퍼 2의 내용을 비교. 같으면, TRUE를 출력


	int sum_for_confirm = 0;

	cout << "Thread3 proof" << endl;
	EnterCriticalSection(&david_CS);
	
	for (int i = 0; i < BUFSIZE; i++)
	{
		if (David::buffer1[i] == David::buffer2[i])
		{
			sum_for_confirm++;
		}
	}

	LeaveCriticalSection(&david_CS);

	if (sum_for_confirm == 20)
	{
		cout << "TURE" << endl;
	}
	else
	{
		cout << "FALSE" << endl;
	}

	return 0;
}

int main()
{
	//  임계영역 구조체 객체 초기화
	InitializeCriticalSection(&david_CS);

	//이벤트들 생성
	Write_StartEvent_forThreadA = CreateEvent(NULL, FALSE, FALSE, L"WS_TA");//보안, 자동 리셋, 비신호 상태, 이벤트 이름 WS_TA .
	ReadWrite_StartEvent_forThreadB = CreateEvent(NULL, FALSE, FALSE, L"RS_TB");//보안, 자동 리셋, 비신호 상태, 이벤트 이름 RS_TB.
	Read_Compare_StartEvent_forThreadC = CreateEvent(NULL, FALSE, FALSE, L"RCS_TC");//보안, 자동 리셋, 비신호 상태, 이벤트 이름 RCS_TC.

	// 세개 스레드들 생성

	HANDLE David_Thread_Handle[3];
	DWORD David_Thread_Handle_Id[3];

	for (int i = 0; i < 3; i++)
	{
		switch (i)
		{
		case 0 :
			David_Thread_Handle[i] = CreateThread(NULL, 0, Write_ThreadA_Route, NULL, 0, &David_Thread_Handle_Id[i]);
			break;
		case 1:
			David_Thread_Handle[i] = CreateThread(NULL, 0, ReadWrite_ThreadB_Route, NULL, 0, &David_Thread_Handle_Id[i]);
			break;
		case 2:
			David_Thread_Handle[i] = CreateThread(NULL, 0, Compare_Confirmation_ThreadC_Route, NULL, 0, &David_Thread_Handle_Id[i]);
			break;
		}
	}
	// 첫번째 스레드부터 스타트.
	cout << "Thread Main proof" << endl;
	SetEvent(Write_StartEvent_forThreadA);

	// 세개 스레드들 Route 모두 완료될 때까지, 여기에서 대기.
	WaitForMultipleObjects(3, David_Thread_Handle, true, INFINITE);
	
	// 임계 영역 제거
	DeleteCriticalSection(&david_CS);
	
	//쓰레드핸들들 제거
	CloseHandle(David_Thread_Handle[0]);
	CloseHandle(David_Thread_Handle[1]);
	CloseHandle(David_Thread_Handle[2]);

	//이벤트 제거.
	CloseHandle(Write_StartEvent_forThreadA);
	CloseHandle(ReadWrite_StartEvent_forThreadB);
	CloseHandle(Read_Compare_StartEvent_forThreadC);

	return 0;
}