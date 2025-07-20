CppServer
---------

CppServer is a fast C++ Server with multithreading and custom memory management via a custom-made memory pool. A Few of the features include:
* Multithreading with a thread pool: This server supports multithreading, using a thread pool to avoid too mush CPU usage, and to not allocate more threads than necessary
* Custom memory pool: A Custom memory pool to speed up allocations, by default using 64 MiB (Approx. 67.1 MB)
* HTTP support: The server supports HTTP, and is capable of sending HTML to clients (Not currently, but will be later)

*Sidenote: CppServer currently only supports Linux, and has only been tested on Ubuntu 24.04.2 LTS

Setup
-----
To setup CppServer, There are a couple things to do:
1. Enter src/main.cpp with your favorite text editor (vim, vi, vscode, etc.)
2. Change 'new Server(8080, "192.168.1.28")' to 'new Server(8080, "&lt;your ip address&gt;")', replacing &lt;your ip address&gt; with your machine's actual IP
3. Type make to compile the code, then make run to run it

(Optional) Stress test CppServer
1. Enter test.py, changing 'SERVER_URL = "http://192.168.1.28:8080"' to 'SERVER_URL = "http://&lt;your ip address&gt;:8080"', replacing &lt;your ip address&gt; with your machine's actual IP
2. Exit, then create a python virtual environment with 'python3 -m venv .venv'
3. Run the virtual environment with 'source .venv/bin/activate'
4. Install packages 'pip install requests matplotlib'
5. Run it 'python3 test.py'
6. To exit the virtual environment, type 'deactivate

```bash
make     # Compile the code
make run # Run CppServer

# In a separate terminal window...

python3 -m venv .venv
source .venv/bin/activate
pip install requests matplotlib
python3 test.py

# To exit
deactivate
```