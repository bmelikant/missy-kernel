%ifndef I386_EXCEPTION_INC
%define I386_EXCEPTION_INC

[bits 32]
[section .text]
align 4

[extern i386_exception_set_disp]
[extern i386_system_stop]
[extern str_SystemException]

[extern printf]
[extern puts]

%endif
