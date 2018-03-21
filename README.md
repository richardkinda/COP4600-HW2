Module:

to "Build": 'make'
to "Install": 'sudo insmod testDev.ko'
to "Remove":  'sudo rmmod testDev'
to "View":  'dmesg'

Testing:
to "Compile": 'gcc -o test test.c'
to "Run": 'sudo ./test'


First, "Build" the module and "Compile" the test file,
Second, "Install" the module and use dmesg to check if it's working
Third, "Run" the test file and hope for the best
Lastly, "Remove" the module
