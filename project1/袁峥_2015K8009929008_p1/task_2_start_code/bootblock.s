	.text
	.globl main

main:
	# check the offset of main
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	#need add code
	# call printstr print a string
	li $8,0xbfe48000
	la $9,string
loop:
	lb $10,($9)
	BEQZ $10,finish
	sb $10,($8)
	nop
	addiu $9,$9,1
	j loop

finish:
	jr $31
	
	
	
	.data
	string: .ascii  "Welcome to OS!"
	



