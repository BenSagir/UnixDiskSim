bensg
Ben Sagir ID 206222200

The following program is a simulation of UNIX file system Disk Management.
The disk is simulated by a file which its size is define to 256 bytes.


How to compile:
	Option 1:
		Open Linux.
		Open the attached folder.
		Right click and chose "Open in Treminal".
		Type in "g++ unixDS.cpp -o unixDS"

	Option 2:
		Open Linux.
		Open Visual Studio Code.
		Open the attached folder in Visual Studio Code.
		Press Ctrl+Shift+B.
		Press Enter twice.


How to run:
	Option 1:
		Open Llinux.
		Open the attached folder in "Files".
		Right click and chose "Open in Treminal".
		On the treminal type in "./unixDS" and press enter.
	
	Option 2:
		Open Linux.
		Open Visual Studio Code.
		Open the attached folder in Visual Studio Code.
		Press Ctrl+F5.
	
	in the program, there is a use in 3 classes:
	(*) fsInode - represent an i-node of a file. saves pointers to its data in the disk.
	(**) FileDescriptor - represent a file that opened during the run.
	(***) fsDisk - represent the disk manager in the file system.
	the program lets you use 8 different methods: file system format, create a file, open and close files,
	write into a file and read from it and delete files
	
	
The folder contain 2 main files:
	finalEx.cpp - contains all the code and all of the functions.
	DISK_SIM_FILE.txt - used as disk in the simulation.
	
Here is a extended list of every function and how to use it:

	function 0: close the program - there is no input and no output. the disk will be deleted and all resources.
	function 1: list all - there is no input. the output is the name and index of every file decriptor. also prints the content of the disk.
	function 2: format disk - the input is 1.block size 2.num of direct blocks. there is no output.
	function 3: create file - the input is a name for a file. the output is the index of the FD in the vector.
	function 4: open file - the input is file name. the output is the index of the opened file.
	function 5: close file - the input is fd index. the output is name of the file that got closed
	function 6: write to file - the input is fd index and a string to write. there is no output.
	function 7: read from file - the input is fd index and amount of char (call it x) to read. the output is the the first x chars in the file.
	function 8: delete file - the input is file name. the output is the fd index of the deleted file.

Errors and how to treat them:
	func 2 : 	"the disk is already formated" 
		this error occure when you are trying to format a formated disk. in order to avoid this, close the program and reopen it, but now with the new values.

	func 3 :	"the disk has not been formated yet"
		you have to format the disk in order to create file.
				"there is a file with the same name. pleae try again"
		you have entered a name of file that already exist in the disk. try another name or delete the current file with that name.

	func 4 :	this func returns -1 if you are trying to open an opened file or a deleted file.
	
	func 5 : 	this func returns -1 when you are trying to close a closed file or closing a file with fd that does not appear in the vector.

	func 6 : 	"the disk hasn't been formated yet"
		you have to format the disk in order to write to file.
				"the file has been deleted"
		you are trying to write into deleted file.
				"this file is closed right now"
		you are trying to write into closed file. use func 4 and then try again.
				"there is not room for *string to write* in the file: *file name*"
		the string you want to write will enlagre the file so much that data will be lost. try write less chars.
				"there is not enogh room for *string to write* in the disk. please delete other files in order to preform this action"
		the disk is so full right now that you cant write this string. if you delete some files you might sucsses.

	func 7 :	"the disk has not been formated yet"
		you have to format the disk in order to read from file.
				"the file is closed now. could not read from the disk"
		you are trying to read from closed file. use func 4 and then try again.

	func 8 :	"there is no file by the name :*file Name*"
		you are trying to delete a file that does not appear in the open FD vector.
	
