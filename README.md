CppServer
---------

CppServer is a fast C++ Server with multithreading and custom memory management via a custom-made memory pool. A Few of the features include:
* Multithreading with a thread pool: This server supports multithreading, using a thread pool to avoid too mush CPU usage, and to not allocate more threads than necessary
* Custom memory pool: A Custom memory pool to speed up allocations, by default using 64 MiB (Approx. 67.1 MB)
* HTTP support: The server supports HTTP, and is capable of sending HTML pages (CSS has to be incorporated into the HTML) to clients

*Sidenote: CppServer currently only supports Linux, and has only been tested on Ubuntu 24.04.2 LTS

Setup
-----
To setup CppServer, There are a couple things to do:
1. Enter src/main.cpp with your favorite text editor (vim, vi, vscode, etc.)
2. Change 'new Server(8080, "192.168.1.30")' to 'new Server(8080, "&lt;your ip address&gt;")', replacing &lt;your ip address&gt; with your machine's actual IP
3. Type make to compile the code, then make run to run it

(Optional) Stress test CppServer
1. In test.cpp, change the Server ip & port to the correct values
2. Compile the test with 'g++ test.cpp -o test'
3. Run the test './test'

*Sidenote: The test might say it took 3 seconds to send 30,000 requests, when it felt like 5. It probably did. There are just a ton of timers, so it is slower, but the results are still somewhat accurate.

```bash
make     # Compile the code
make run # Run CppServer

# In a separate terminal window...

# Stress test CppServer with C
g++ test.cpp -o test
./test
```

Usage
-----

To use CppServer, you can add files to the site/ directory, and access them from the web. For example you can load site/index.html by going to '<your ip address>:8080/index.html'. There is already a basic index.html to test