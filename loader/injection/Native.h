#include <Windows.h>
#include <winternl.h>

#pragma once

using NtCreateSection = NTSTATUS(NTAPI*)(
	OUT PHANDLE SectionHandle,
	IN ULONG DesiredAccess,
	IN OPTIONAL POBJECT_ATTRIBUTES ObjectAttributes,
	IN OPTIONAL PLARGE_INTEGER MaximumSize,
	IN ULONG PageAttributess,
	IN ULONG SectionAttributes,
	IN OPTIONAL HANDLE FileHandle);

using NtMapViewOfSection = NTSTATUS(NTAPI*)(
	IN HANDLE SectionHandle,
	IN HANDLE ProcessHandle,
	IN OUT PVOID* BaseAddress,
	IN ULONG_PTR ZeroBits,
	IN SIZE_T CommitSize,
	IN OUT OPTIONAL PLARGE_INTEGER SectionOffset,
	IN OUT PSIZE_T ViewSize,
	IN DWORD InheritDisposition,
	IN ULONG AllocationType,
	IN ULONG Win32Protect);

using NtUnmapViewOfSection = NTSTATUS(NTAPI*)(
	IN HANDLE ProcessHandle,
	IN PVOID BaseAddress OPTIONAL);

typedef enum _SECTION_INHERIT : DWORD {
	ViewShare = 1,
	ViewUnmap = 2
} SECTION_INHERIT, * PSECTION_INHERIT;