#include <windows.h>
#include <stdio.h>
#include <iostream>
using namespace std;

#define BUFSIZE 20

HANDLE Write_StartEvent_forThreadA;
HANDLE ReadWrite_StartEvent_forThreadB;
HANDLE Read_Compare_StartEvent_forThreadC;
CRITICAL_SECTION david_CS;// ����ü ��ü ���� ����

namespace David // ���� ������鿡�� ���� ����, ���� ���۵� ����.
{
	char buffer1[BUFSIZE];
	char buffer2[BUFSIZE];
}

DWORD WINAPI  Write_ThreadA_Route(LPVOID arg)
{ 
	//���� ���� �̰� �����.

	DWORD return_value;
	// ���ξ����忡�� �� �̺�Ʈ�� Signal �Ǳ⸦ ��ٸ�. �׷���, ���۵�.
	
	return_value = WaitForSingleObject(Write_StartEvent_forThreadA, INFINITE);
	if (return_value == WAIT_FAILED) return 0;


	cout << "Thread1 proof" << endl;
	//��������1�� ����.

	EnterCriticalSection(&david_CS);
	
	for (int i = 0; i < BUFSIZE; i++)
	{
		David::buffer1[i] = 7;
	}

	LeaveCriticalSection(&david_CS);

	//���� �ϷḦ ������B�� �˸�.
	SetEvent(ReadWrite_StartEvent_forThreadB);


	return 0;
}


DWORD WINAPI ReadWrite_ThreadB_Route(LPVOID arg)
{
	DWORD return_value;

		// �������� 1�� ���� �ϷḦ ��ٸ�
		return_value = WaitForSingleObject(ReadWrite_StartEvent_forThreadB, INFINITE);
		if (return_value == WAIT_FAILED) return 0;

		cout << "Thread2 proof" << endl;

		EnterCriticalSection(&david_CS);

		// �������� 1�� ������ �о�, �������� 2�� ������.
		for (int i = 0; i < BUFSIZE; i++)
		{
			David::buffer2[i] = David::buffer1[i];
		}

		LeaveCriticalSection(&david_CS);
		
		// �̺�Ʈ�� State�� ���������ν�, ���� ����2�� ���� �ϷḦ �˸�.
		SetEvent(Read_Compare_StartEvent_forThreadC);
	
		return 0;
}


DWORD WINAPI Compare_Confirmation_ThreadC_Route(LPVOID arg)
{
	DWORD return_value;

	// �������� 2�� ���� �ϷḦ ��ٸ�
	return_value = WaitForSingleObject(Read_Compare_StartEvent_forThreadC, INFINITE);
	if (return_value == WAIT_FAILED) return 0;

	// ��������1, �������� 2�� ������ ��. ������, TRUE�� ���


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
	//  �Ӱ迵�� ����ü ��ü �ʱ�ȭ
	InitializeCriticalSection(&david_CS);

	//�̺�Ʈ�� ����
	Write_StartEvent_forThreadA = CreateEvent(NULL, FALSE, FALSE, L"WS_TA");//����, �ڵ� ����, ���ȣ ����, �̺�Ʈ �̸� WS_TA .
	ReadWrite_StartEvent_forThreadB = CreateEvent(NULL, FALSE, FALSE, L"RS_TB");//����, �ڵ� ����, ���ȣ ����, �̺�Ʈ �̸� RS_TB.
	Read_Compare_StartEvent_forThreadC = CreateEvent(NULL, FALSE, FALSE, L"RCS_TC");//����, �ڵ� ����, ���ȣ ����, �̺�Ʈ �̸� RCS_TC.

	// ���� ������� ����

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
	// ù��° ��������� ��ŸƮ.
	cout << "Thread Main proof" << endl;
	SetEvent(Write_StartEvent_forThreadA);

	// ���� ������� Route ��� �Ϸ�� ������, ���⿡�� ���.
	WaitForMultipleObjects(3, David_Thread_Handle, true, INFINITE);
	
	// �Ӱ� ���� ����
	DeleteCriticalSection(&david_CS);
	
	//�������ڵ�� ����
	CloseHandle(David_Thread_Handle[0]);
	CloseHandle(David_Thread_Handle[1]);
	CloseHandle(David_Thread_Handle[2]);

	//�̺�Ʈ ����.
	CloseHandle(Write_StartEvent_forThreadA);
	CloseHandle(ReadWrite_StartEvent_forThreadB);
	CloseHandle(Read_Compare_StartEvent_forThreadC);

	return 0;
}