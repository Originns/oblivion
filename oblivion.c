#include <winternl.h>
#include <stdio.h>
#include "oblivion.h"

EXTERN_C VOID instrumentation();
EXTERN_C LPVOID KiUserExceptionDispatcher = NULL;
EXTERN_C VOID InstrumentationCallbackInternal(PEXCEPTION_RECORD ExceptionRecord, PCONTEXT ContextRecord)
{
	printf("Caught Exception at : 0x%p\n", ExceptionRecord->ExceptionAddress);

	ContextRecord->Rip++;

	RtlRestoreContext(ContextRecord, NULL);
}

int main()
{
	printf("Hello World!\n");

	HMODULE hntdll = GetModuleHandleA("ntdll.dll");

	if (hntdll == NULL)
	{
		printf("Failed to load ntdll.dll\n");
		return 0;
	}

	KiUserExceptionDispatcher = GetProcAddress(hntdll, "KiUserExceptionDispatcher");

	if (KiUserExceptionDispatcher == NULL)
	{
		printf("Failed to get KiUserExceptionDispatcher\n");
		return 0;
	}

	printf("KiUserExceptionDispatcher: %p\n", KiUserExceptionDispatcher);

	pNtSetInformationProcess NtSetInformationProcess = (pNtSetInformationProcess)GetProcAddress(hntdll, "NtSetInformationProcess");

	if (NtSetInformationProcess == NULL)
	{
		printf("Failed to get NtSetInformationProcess\n");
		return 0;
	}

	PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION nirvana;
	nirvana.Callback = instrumentation;
	nirvana.Reserved = 0; // always 0
	nirvana.Version = 0;  // 0 for x64, 1 for x86

	if (!NT_SUCCESS(NtSetInformationProcess(
			(HANDLE)-1,
			(PROCESS_INFORMATION_CLASS)0x28,
			&nirvana,
			sizeof(nirvana))))
	{
		printf("Failed to set instrumentation callback\n");
		return 0;
	}

	printf("Generating exception...\n");

	__debugbreak();

	printf("Done\n");

	return 0;
}
