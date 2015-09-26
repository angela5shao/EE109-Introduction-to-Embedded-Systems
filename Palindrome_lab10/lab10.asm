# ee109_palindrome.s
# Name:  Ann-Chie (Angela) Shao
# Assignment: Lab 10

	.data
buf :   .space  22                 # receive original input in this buffer
counts: .byte 0,0,0,0,0,0,0,0,0,0  # array of counts for digits 0-9 for finding mode digit

# constant strings/output for use use
# for your prompts and messages... use syscall with $v0=4
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
	# newline = '\n' = 0x0a	
	add	$s0, $0, $a0	# store input ($a0) in $s0
	lb	$s2, nline	# $s2 = "\n"
	
	li      $s2, 0x0a
	# using $t0, t4
	# find input.length()
	la 	$t0, buf	 	# $t0 = addr(buf)
	lb 	$t2, 0($t0)		# $t2 = input[i] 
looplen:
	addi	$t4, $t4, 1		# $t4 = length; increment len
	
	addi	$t2, $t2, -1		# move to next number
	
	bne 	$t2, $s2, looplen	# if buf != \n, loop
	
	
	
	
	add	$s1, $0, $t4		# if found length, $s1 = length
	
	# $s3 = input.length()-1
	sub   	$s3, $s1, 1
	
	# using $t0-4
	add	$t1, $0, $0		# $t1 = i = 0
	la	$t0, buf		# $t0 = addr(buf) (reload bc it $t0 was changed)
	div 	$s4, $s1, 2	# $s4 = length/2
loop:	# checking input[i] != input[input.length()-i-1]
	lb 	$t2, 0($t0)	# $t2 = input[i] 
	sub	$t4, $s3, $t1	# $t4 = input.length-1 -i
	sub	$t3, $s3, -1	# $t3 = input[input.length()-i-1]
	# if true
	sne  	$t6, $t2, $t3	# if !=
	bne 	$t6, $0, isnot
	
	#else
	addi	$t1, $t1, 1 	#i++
	addi	$t0, $t0, 1	# move to next number in buf
	# if i<input.length()/2, loop
	slt 	$t7, $t1, $s4
	bne 	$t7, $0, loop
	# else, exit loop to "is"
	b 	is

isnot:	# Print string
	li	$v0, 4		
	la	$a0, buf
	syscall
	# Print msgNotPal
	li	$v0, 4		
	la	$a0, msgNotPal
	syscall
	
is:	# Print string
	li	$v0, 4		
	la	$a0, buf
	syscall
	# Print msgPal
	li	$v0, 4		
	la	$a0, msgPal
	syscall


	# exit
	li	$v0,10
	syscall
