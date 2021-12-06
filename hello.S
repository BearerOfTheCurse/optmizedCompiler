	.section .rodata
sHelloMsg:  .string "hello, world\n"
sPromptMsg: .string "Enter an integer (negative to exit): "
sInputFmt:  .string "%ld"
sOutputMsg: .string "You entered: %ld\n"
	.section .text
	.globl main
main:
	subq $8, %rsp
	movq $sHelloMsg, %rdi
	call printf
.L0:
	movq $sPromptMsg, %rdi
	call printf
	movq $sInputFmt, %rdi
	leaq 0(%rsp), %rsi
	call scanf
	movq 0(%rsp), %r10
	cmpq $0, %r10
	jl .L1
	movq $sOutputMsg, %rdi
	movq 0(%rsp), %rsi
	call printf
	jmp .L0
.L1:
	addq $8, %rsp
	movl $0, %eax
	ret
