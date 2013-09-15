

from __future__ import print_function
import random

registerMin = 0
registerMax = 7

immediateMin = -16
immediateMax = 15

hexMin = -9
hexMax = 9

opcodeArray = []
opcodeArray.append("AND")
opcodeArray.append("ADD")

#opcodeArray.append("add")
#opcodeArray.append("adD")
#opcodeArray.append("AdD")
#opcodeArray.append("AND")
#opcodeArray.append("aNd")
#opcodeArray.append("anD")
#opcodeArray.append("XOR")
#opcodeArray.append("xOr")
#opcodeArray.append("xOR")
#opcodeArray.append("xor")
#opcodeArray.append("XOr")

branchArray = []
branchArray.append("br")
branchArray.append("brn")
branchArray.append("brp")
branchArray.append("brz")
branchArray.append("brnz")
branchArray.append("brnp")
branchArray.append("brzp")
branchArray.append("brnzp")

ldArray = []
ldArray.append("ldb")
#ldArray.append("ldw")
ldArray.append("stb")
#ldArray.append("stw")

ldMax = 31
ldMin = -32

shfArray = []
shfArray.append("lshf")
shfArray.append("rshfl")
shfArray.append("rshfa")

shfMin = -8
shfMax = 7
shfHexMin = -8
shfHexMax = 7


for x in range (0, 10):
	myFile = open('CurtisXander' + str(x) + '.asm', 'w+')
	
	myFile.write('.orig #4096' +  '\n')
	#myFile.write('.orig #' + str(random.randint(0, 60000)) +  '\n')
	
	# Add, And, Xor
	
	for y in range (0, 50):
		myFile.write(opcodeArray[random.randint(0, 1)] + ' R' + str(random.randint(registerMin, registerMax)) +  ', R' + str(random.randint(registerMin, registerMax)) +  ', R' + str(random.randint(registerMin, registerMax)) + '\n')
		
	for y in range (0, 50):
		myFile.write(opcodeArray[random.randint(0, 1)] + ' R' + str(random.randint(registerMin, registerMax)) +  ', R' + str(random.randint(registerMin, registerMax)) +  ', R' + str(random.randint(registerMin, registerMax)) + '\n' )
		
	for y in range (0, 50):
		myFile.write(opcodeArray[random.randint(0, 1)] + ' R' + str(random.randint(registerMin, registerMax)) +  ', 	R' + str(random.randint(registerMin, registerMax)) +  '	, R' + str(random.randint(registerMin, registerMax)) + '\n' )
		
	for y in range (0, 50):
		myFile.write(opcodeArray[random.randint(0, 1)] + ' R' + str(random.randint(registerMin, registerMax)) +  ', R' + str(random.randint(registerMin, registerMax)) +  ', R' + str(random.randint(registerMin, registerMax)) + '\n' )
		
	for y in range (0, 50):
		myFile.write(opcodeArray[random.randint(0, 1)] + ' R' + str(random.randint(registerMin, registerMax)) +  ', R' + str(random.randint(registerMin, registerMax)) +  ', #' + str(random.randint(immediateMin, immediateMax)) + '\n' )

	for y in range (0, 50):
		myFile.write(opcodeArray[random.randint(0, 1)] + ' R' + str(random.randint(registerMin, registerMax)) +  ', R' + str(random.randint(registerMin, registerMax)) +  ', x' + str(random.randint(hexMin, hexMax)) + '\n' )
		
	# Branch
	
	for y in range (0, 50):
		myFile.write(branchArray[random.randint(0, 7)] + ' A' + '\n' )
		
	# Lea
	
	for y in range (0, 50):
		myFile.write('LEA' + ' R' + str(random.randint(registerMin, registerMax)) + ', A' + '\n' )
		
	myFile.write('A ' + opcodeArray[random.randint(0, 1)] + ' R' + str(random.randint(registerMin, registerMax)) +  ', R' + str(random.randint(registerMin, registerMax)) +  ', #' + str(random.randint(hexMin, hexMax)) + '\n' )
	
	#for y in range (0, 50):
	#	myFile.write('LEA' + ' R' + str(random.randint(registerMin, registerMax)) + ' A' + '\n' )

	for y in range (0, 50):
		myFile.write(branchArray[random.randint(0, 7)] + ' A' + '\n' )
		
	#Jmp
	
	for y in range (0, 50):
		myFile.write('JMP' + ' R' + str(random.randint(registerMin, registerMax)) + '\n' )
		
	#Jsr
	
	for y in range (0, 50):
		myFile.write('JSR' + ' A' + '\n' )
		
	#Jsrr
	
	for y in range (0, 50):
		myFile.write('Jsrr' + ' R' + str(random.randint(registerMin, registerMax)) + '\n' )	
		
	# Ldb, Ldw, Stb, Stw
	
#	for y in range (0, 50):
#		myFile.write(ldArray[random.randint(0, 1)] + ' R' + str(random.randint(registerMin, registerMax)) +  ', R' + str(random.randint(registerMin, registerMax)) +  ', # ' + str(random.randint(ldMin, ldMax)) + '\n' )
		
	# Not
	for y in range (0, 50):
		myFile.write('Not' + ' R' + str(random.randint(registerMin, registerMax)) +  ', R' + str(random.randint(registerMin, registerMax)) + '\n' )
	
	# Ret
	
	myFile.write('Ret' + '\n')
	
	#Rti
	
	myFile.write('rti' + '\n')
	
	#Lshf, Rshfl, Rshfa
	
	#for y in range (0, 50):
	#	myFile.write('Not' + ' R' + str(random.randint(registerMin, registerMax)) +  ', R' + str(random.randint(registerMin, registerMax)) + '\n' )
		
	#for y in range (0, 50):
	#	myFile.write(shfArray[random.randint(0, 2)] + ' R' + str(random.randint(registerMin, registerMax)) +  ', R' + str(random.randint(registerMin, registerMax)) +  ', # ' + str(random.randint(shfMin, shfMax)) + '\n' )
	
	#for y in range (0, 50):
	#	myFile.write(shfArray[random.randint(0, 2)] + ' R' + str(random.randint(registerMin, registerMax)) +  ', R' + str(random.randint(registerMin, registerMax)) +  ', x ' + str(random.randint(shfHexMin, shfHexMax)) + '\n' )
		
	
	myFile.write('.end\n')

