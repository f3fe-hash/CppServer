CppServer
---------

CppServer is a fast C++ Server with multithreading and custom memory management via a custom-made memory pool. A Few of the features include:
* Multithreading with a thread pool: This server supports multithreading, using a thread pool to avoid too mush CPU usage, and to not allocate more threads than necessary
* HTTP support: The server supports HTTP, and is capable of sending HTML, JSON, CSS, AVIF, WEBP, PNG, and SVG to clients
* If you run the server, it shows my [website's](f3fe-hash.github.io/CppServer-Website) main page. It also contains files of other types to mess around with


Warnings
--------
* CppServer currently only supports Linux, and has only been tested on Ubuntu 24.04.2 LTS.
* You need top Ctrl-C TWICE to exit, not once.

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

```bash
make     # Compile the code
make run # Run CppServer

# In a separate terminal window...

# Stress test CppServer with C++
g++ test.cpp -o test
./test
```

Usage
-----

To use CppServer, you can add files to the site/ directory, and access them from the web. For example you can load site/index.html by going to '&lt;your ip address&gt;:8080/index.html'. There is already a basic index.html to test