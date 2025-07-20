import requests
import time
import tkinter as _tk

SERVER_URL = "http://192.168.1.28:8080"
NUM_REQUESTS = 40000
WAIT_TIME = 100

WINDOW_WIDTH  = 800
WINDOW_HEIGHT = 600

class Application:
    def __init__(self):
        self.tk = _tk.Tk()
        self.tk.title("CppServer Test")
        self.tk.wm_attributes('-topmost', 1)
        self.tk.geometry("%dx%d" % (WINDOW_WIDTH, WINDOW_HEIGHT))

        # Use grid instead of pack
        self.min_time_label = _tk.Label(self.tk, text="Min time:")
        self.min_time_label.grid(row=0, column=0, padx=10, pady=10)

        self.mid_time_label = _tk.Label(self.tk, text="Average time:")
        self.mid_time_label.grid(row=1, column=0, padx=10, pady=10)

        self.max_time_label = _tk.Label(self.tk, text="Max time:")
        self.max_time_label.grid(row=2, column=0, padx=10, pady=10)

        self.requests_label = _tk.Label(self.tk, text="Num requests:")
        self.requests_label.grid(row=3, column=0, padx=10, pady=10)

        self.min_time = _tk.Label(self.tk, text="0.00 ms")
        self.min_time.grid(row=0, column=1, padx=10, pady=10)

        self.mid_time = _tk.Label(self.tk, text="0.00 ms")
        self.mid_time.grid(row=1, column=1, padx=10, pady=10)

        self.max_time = _tk.Label(self.tk, text="0.00 ms")
        self.max_time.grid(row=2, column=1, padx=10, pady=10)

        self.requests = _tk.Label(self.tk, text="0")
        self.requests.grid(row=3, column=1, padx=10, pady=10)

        self.tk.update()
        self.tk.update_idletasks()
    
    def main(self):
        response_times = []
        min_time = 0
        avg_time = 0
        max_time = 0
        for i in range(NUM_REQUESTS):
            start = time.time()
            try:
                response = requests.get(SERVER_URL)
                response.raise_for_status()
            except Exception:
                continue
            end = time.time()
            elapsed = end - start
            if elapsed < min_time:
                min_time = elapsed
            elif min_time == 0:
                min_time = elapsed
            elif elapsed > max_time:
                max_time = elapsed
            elif avg_time == 0:
                avg_time = elapsed
            else:
                avg_time = (avg_time + elapsed) / 2
            response_times.append(elapsed)
            self.min_time.configure(text=f"{(min_time * 1000) :.2f} ms")
            self.mid_time.configure(text=f"{(avg_time * 1000) :.2f} ms")
            self.max_time.configure(text=f"{(max_time * 1000) :.2f} ms")
            self.requests.configure(text=f"{int(i)}")
            if (i + 1) % 1000 == 0:
                time.sleep(WAIT_TIME / 1000)
            self.tk.update()
            self.tk.update_idletasks()
        if response_times:
            print(f"Minimum response time: {(min_time * 1000) :.2f} milliseconds")
            print(f"Average response time: {(avg_time * 1000) :.2f} milliseconds")
            print(f"Maximum response time: {(max_time * 1000) :.2f} milliseconds")
        else:
            print("No successful responses.")

if __name__ == "__main__":
    app = Application()
    app.main()