#The Prime Number Producer - Consumer Problem with Faulty Producer Threads

##Contents of Directory:
####Makefile:
File that compiles the .c source code and creates the osproj3 executable
####README:
File specifying contents of directory and solutions to problems.
####osproj3.c:
Source code, successfully performs The Prime Number Producer - Consumer Problem with Faulty Producer Threads and Bounded Buffer.

##Solution to problems:
The program took me some time, probably 16+ hours. Something that took me long to solve was the getopt method, which was giving me wrong values for the arguments until I realized that I was declaring optarg, which is not necessary. Another problem I had was when sending the data to the thread, it would consider all threads to have the same number, I solved it by changing the way I reference the thread data when sending it to the thread through thread_create. Messing with the threads, mutexes and semaphores didn't seem too bad for me, since I had a clear idea in my mind on how to use them.
