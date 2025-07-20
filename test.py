import requests
import time
import tkinter as _tk
import matplotlib.pyplot as plt

SERVER_URL        = "http://192.168.1.28:8080"
NUM_REQUESTS      = 10000
UPDATE_EVERY      = 500      # Update every UPDATE_EVERY requests (To make sure that humans can read the numbers)
PLOT_UPDATE_EVERY = 50
WAIT_TIME         = 100

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
        self.min_time_label.grid(row=0, column=0, padx=10, pady=5)

        self.mid_time_label = _tk.Label(self.tk, text="Average time:")
        self.mid_time_label.grid(row=1, column=0, padx=10, pady=5)

        self.max_time_label = _tk.Label(self.tk, text="Max time:")
        self.max_time_label.grid(row=2, column=0, padx=10, pady=5)

        self.requests_label = _tk.Label(self.tk, text="Num requests:")
        self.requests_label.grid(row=3, column=0, padx=10, pady=5)

        self.min_time = _tk.Label(self.tk, text="0.00 ms")
        self.min_time.grid(row=0, column=1, padx=10, pady=5)

        self.mid_time = _tk.Label(self.tk, text="0.00 ms")
        self.mid_time.grid(row=1, column=1, padx=10, pady=5)

        self.max_time = _tk.Label(self.tk, text="0.00 ms")
        self.max_time.grid(row=2, column=1, padx=10, pady=5)

        self.requests = _tk.Label(self.tk, text="0")
        self.requests.grid(row=3, column=1, padx=10, pady=5)

        self.tk.update()
        self.tk.update_idletasks()
    
    def main(self):
        response_times = []
        min_time = 0.0000001
        avg_time = 0.0000001
        max_time = 0.0000001

        # Setup matplotlib interactive plot
        plt.ion()
        fig, ax = plt.subplots(figsize=(10, 4))
        line, = ax.plot([], [], label="Response Time (s)")
        ax.set_xlim(0, NUM_REQUESTS)
        ax.set_ylim(0, 1)  # This can auto-adjust if you prefer
        ax.set_xlabel("Request Number")
        ax.set_ylabel("Time (s)")
        ax.set_title("Live Response Time Plot")
        ax.grid(True)
        ax.legend()

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
            elif min_time < 0.00001:
                min_time = elapsed
            elif elapsed > max_time:
                max_time = elapsed
            else:
                avg_time = (avg_time + elapsed) / 2

            response_times.append(elapsed * 1000) # Add ms, not sec

            if (i + 1) % UPDATE_EVERY == 0:
                self.min_time.configure(text=f"{(min_time * 1000):.2f} ms ({(1 / min_time):.2f} req/s)")
                self.mid_time.configure(text=f"{(avg_time * 1000):.2f} ms ({(1 / avg_time):.2f} req/s)")
                self.max_time.configure(text=f"{(max_time * 1000):.2f} ms ({(1 / max_time):.2f} req/s)")

            # Update plot
            if (i + 1) % PLOT_UPDATE_EVERY == 0:
                line.set_data(range(len(response_times)), response_times)
                ax.set_xlim(0, len(response_times))
                ax.set_ylim(0, max(response_times) * 1.2)  # Dynamic y-limit
                fig.canvas.draw()
                fig.canvas.flush_events()

            self.requests.configure(text=f"{i + 1} / {NUM_REQUESTS} ({((i + 1) / NUM_REQUESTS) * 100:.2f}%)")
            if (i + 1) % 1000 == 0:
                time.sleep(WAIT_TIME / 1000)

            self.tk.update()
            self.tk.update_idletasks()

        plt.ioff()  # Disable interactive mode after loop
        plt.show()  # Show final plot

        if response_times:
            print(f"Minimum response time: {(min_time * 1000):.2f} ms")
            print(f"Average response time: {(avg_time * 1000):.2f} ms")
            print(f"Maximum response time: {(max_time * 1000):.2f} ms")
        else:
            print("No successful responses.")

if __name__ == "__main__":
    app = Application()
    app.main()