#include "pch.h"

#include <stdio.h>
#include <windows.h>

const DWORD sleep_time_for_writer = 50; // milliseconds for writer 
const DWORD sleep_time_for_reader = 30; // milliseconds for reader 

const int readers_count = 5;
const int writers_count = 3;
const int iterations = 5;

HANDLE mutex; // void *, дескриптор объекта
HANDLE can_read;
HANDLE can_write;

HANDLE writers[writers_count];
HANDLE readers[readers_count];

volatile LONG active_readers = 0;
// информирует компилятор, что значение переменной может меняться извне. 
// Это может произойти под управлением операционной системы, 
// аппаратных средств или другого потока. 
bool writing = false;

int value = 0;

void start_read()
{
	// WaitForSingleObject - Waits until the specified object is in the signaled state or the time-out interval elapses.
	// hHandle, dwMilliseconds
	// WAIT_OBJECT_0 - в случае успеха возвращается WAIT_OBJECT_0, если мы дождались перехода объекта в сигнальное состояние
	if (writing || WaitForSingleObject(can_write, 0) == WAIT_OBJECT_0) 
	{
		WaitForSingleObject(can_read, INFINITE);
	}

	//WaitForSingleObject(mutex, INFINITE);
	InterlockedIncrement(&active_readers); // Increments (increases by one) the value of the specified 32-bit variable as an atomic operation.
	SetEvent(can_read); // Waits until the specified object is in the signaled state or 
	// the time-out intervaSets the specified event object to the signaled state.l elapses.
}

void stop_read()
{
	InterlockedDecrement(&active_readers); // Decrements (decreases by one) the value of the specified 32-bit variable as an atomic operation.

	if (active_readers == 0) 
	{
		SetEvent(can_write); // можно писать
	}

	//ReleaseMutex(mutex);
}

void start_write()
{
	WaitForSingleObject(mutex, INFINITE);

	if (writing || active_readers > 0) {
		WaitForSingleObject(can_write, INFINITE);
	}

	writing = true;

	ReleaseMutex(mutex);
}

void stop_write()
{
	writing = false;

	if (WaitForSingleObject(can_read, 0) == WAIT_OBJECT_0) 
	{
		SetEvent(can_read);
	}
	else 
	{
		SetEvent(can_write);
	}
}

DWORD WINAPI reader(LPVOID)
{
	while (value < writers_count * iterations) {
		start_read();
		//Sleep(sleep_time_for_reader);
		printf("\tReader #%ld <---- %d\n", GetCurrentThreadId(), value);
		stop_read();
		Sleep(sleep_time_for_reader);
	}

	return 0; 
}

DWORD WINAPI writer(LPVOID) // LPVOID - A pointer to any type.
	// Функция потока должна соответствовать следующему прототипу
{
	for (int i = 0; i < iterations; i++) {
		start_write();
		//Sleep(sleep_time_for_writer);
		printf("Writer #%ld ----> %ld\n", GetCurrentThreadId(), ++value);
		stop_write();
		Sleep(sleep_time_for_writer);
	}

	return 0;
}

int init_handles()
{
	mutex = CreateMutex(NULL, FALSE, NULL); // атрибут безопасности, флаг начального владельца, имя
	// A pointer to a SECURITY_ATTRIBUTES structure. 
	// If this parameter is NULL, the handle cannot be inherited by child processes.

	// Если флаг начального владельца = TRUE, и вызывающий объект создал мьютекс, вызывающий поток 
	// получает первоначальное владение объектом мьютекса. 
	// В противном случае вызывающий поток не получает права собственности на мьютекс. 
	if (mutex == NULL) 
	{
		perror("Can't create mutex. Stop.\n");
		return EXIT_FAILURE;
	}

	can_read = CreateEvent(NULL, FALSE, TRUE, NULL);
	can_write = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (can_read == NULL || can_write == NULL)
	{
		perror("Can't create event can read. Stop.\n");
		return EXIT_FAILURE;
	}
	// Описание парметров CreateEvent по порядку
	// атрибут безопасности

	// bManualReset - если этот параметр имеет значение TRUE, функция создает объект события с ручным сбросом, 
	// для которого требуется использовать функцию ResetEvent,
	// чтобы установить состояние события как «без сигнала». 
	// Если этот параметр имеет значение FALSE, 
	// функция создает объект события с автосбросом,
	// и система автоматически сбрасывает состояние события в 
	// состояние без сигнала после освобождения одного ожидающего потока.

	// bInitialState - If this parameter is TRUE, the initial state of the event object is signaled;
	// otherwise, it is nonsignaled.

	// имя

	// Приложения могут использовать объекты событий в ряде ситуаций, 
	// чтобы уведомить ожидающий поток о возникновении события. 

	return EXIT_SUCCESS;
}

int create_threads()
{
	// CreateThread creates a thread to execute within the virtual address space of the calling process.
	// атрибут безопасности 
	
	// dwStackSize - Начальный размер стека в байтах. Система округляет это значение до ближайшей страницы. 
	// Если этот параметр равен нулю, новый поток использует размер по умолчанию для исполняемого файла.
	
	// lpStartAddress - Указатель на определяемую приложением функцию, которая будет выполняться потоком.
	// Этот указатель представляет начальный адрес потока. 

	// lpParameter - Указатель на переменную для передачи в поток.

	// dwCreationFlags - Флаги, которые управляют созданием потока.
	// 0 - The thread runs immediately after creation. 
	// CREATE_SUSPENDED - Поток создается в приостановленном состоянии и не запускается до тех пор, 
	//					  пока не будет вызвана функция ResumeThread.
	// STACK_SIZE_PARAM_IS_A_RESERVATION - The dwStackSize parameter specifies the initial reserve size of the stack. 
	//									   If this flag is not specified, dwStackSize specifies the commit size. 

	// lpThreadId - Указатель на переменную, которая получает идентификатор потока. 
	// Если этот параметр равен NULL, идентификатор потока не возвращается. 
	for (int i = 0; i < writers_count; i++) 
	{
		writers[i] = CreateThread(NULL, 0, writer, NULL, 0, NULL);
		if (writers[i] == NULL) 
		{

			perror("Can't create writer. Stop.\n");
			return EXIT_FAILURE;
		}
	}

	for (int i = 0; i < readers_count; i++) 
	{
		readers[i] = CreateThread(NULL, 0, reader, NULL, 0, NULL);
		if (readers[i] == NULL) 
		{
			perror("Can't create reader. Stop.\n");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

int main()
{
	int code = EXIT_SUCCESS;

	if ((code = init_handles()) != EXIT_SUCCESS) {
		return code;
	}

	if ((code = create_threads()) != EXIT_SUCCESS) {
		return code;
	}
	
	// Waits until one or all of the specified objects are in the signaled state or the time-out interval elapses.
	// nCount - The number of object handles in the array pointed to by lpHandles. 
	//			The maximum number of object handles is MAXIMUM_WAIT_OBJECTS. This parameter cannot be zero.
	// lpHandles - An array of object handles.
	// bWaitAll - If this parameter is TRUE, the function returns when the state of all objects in the lpHandles array is signaled. 
	//			  If FALSE, the function returns when the state of any one of the objects is set to signaled. 
	// dwMilliseconds - The time-out interval, in milliseconds. If a nonzero value is specified, 
	//					the function waits until the specified objects are signaled or the interval elapses.
	WaitForMultipleObjects(writers_count, writers, TRUE, INFINITE); 
	WaitForMultipleObjects(readers_count, readers, TRUE, INFINITE);

	CloseHandle(mutex); // Closes an open object handle.
	CloseHandle(can_read);
	CloseHandle(can_write);

	system("pause");

	return code;
}
