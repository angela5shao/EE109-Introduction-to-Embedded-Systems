# ee109_palindrome.s
# Name:  Ann-Chie (Angela) Shao
# Assignment: Lab 10

	.data
buf :   .space  22                 # receive original input in this buffer
counts: .byte 0,0,0,0,0,0,0,0,0,0  # array of counts for digits 0-9 (to find mode)

# constant strings/output for use
msgIn:     .asciiz "Enter up to 20 digits without spaces: "
msgString: .asciiz "Your string: "
msgPal:    .asciiz " IS a palindrome\n"
msgNotPal: .asciiz " IS NOT a palindrome\n"
msgMode:   .asciiz "The mode digit is: "

# print this string for a newline character
nline:  .asciiz "\n"


	.text
main:
	li	$v0, 4		# Print the prompt
	la	$a0, msgIn
	syscall
	
	li	$v0, 8		# Read the string from the user
	la	$a0, buf
	li	$a1, 22
	syscall
	# this syscall will keep the newline character on the end of the string
	# so we will know we're at the end of a string when we hit the newline
	
	add	$s0, $0, $a0	# $s0 = buf
	
	li      $s2, 0x0a
	la 	$t0, buf	 
	# find addr of last digit/char
looplen:lb	$t3, 0($t0)	# load digit
	addi	$t0, $t0, 1	# move to next digit
	
	bne	$t3, $s2, looplen # if reached end '\n'
	
	subi	$t0, $t0, 2	# find addr of last digit ($t0 is one past \n)



loop:	# compare digits from each end of buf
	lb	$t2, 0($s0)
	lb 	$t3, 0($t0)
	# if digits not equal, branch to isnot
	bne 	$t2, $t3, isnot
	
	# update digits and exit if $a0 !< $t0
	addi	$s0, $s0, 1
	addi	$t0, $t0, -1
	bge 	$s0, $t0, is
	b 	loop


isnot:	# Print string
	li	$v0, 4		
	la	$a0, msgString
	syscall
	li	$v0, 4		
	la	$a0, buf
	syscall
	# Print msgNotPal
	li	$v0, 4		
	la	$a0, msgNotPal
	syscall
	b	cnt1
	
is:	# Print string
	li	$v0, 4		
	la	$a0, msgString
	syscall
	li	$v0, 4		
	la	$a0, buf
	syscall
	# Print msgPal
	li	$v0, 4		
	la	$a0, msgPal
	syscall




cnt1:	la	$t1, counts	# $t1 = count
	la	$t0, buf	# $t0 = buf
	
cnt:	# $t2 = offset from $t1 (count)
	lb	$t6, 0($t0)	# load digit in buf
	subi	$t2, $t6, 0x30
		
	add	$t2, $t2, $t1
	lb	$t5, 0($t2)
	addi	$t5, $t5, 1
	sb	$t5, 0($t2)
	
	addi	$t0, $t0, 1 	# move to next digit in buf
	bne	$t6, $s2, cnt


 	# starting with count of 9, find greatest count
	la	$t0, counts
	addi	$t1, $t0, 9
	lb 	$s0, 0($t1)	# $s0 = greatest count (right now, count of 9's)
mode:	beq	$t0, $t1, printM # loop until $t0 == $t1
	subi	$t1, $t1, 1	# decrement addr of count 
	lb	$t2, 0($t1)
	
	#sgt 	$t3, $t2, $s0	# if $t2 > $s0
	ble  	$t2, $s0, noUpdM
	add	$s0, $0, $t2	
	add	$t4, $0, $t1	# $t4 = addr of greatest count
		
noUpdM:	b	mode		# loop till end of count array
	

printM:	# Print mode
	li	$v0, 4		
	la	$a0, msgMode
	syscall
	
	sub	$t4, $t4, $t0

	li	$v0, 1		
	add	$a0, $0, $t4
	syscall
	
	# exit
	li	$v0,10
	syscall
