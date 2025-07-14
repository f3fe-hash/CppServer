import requests
import time

SERVER_URL = "http://192.168.1.28:8080"
NUM_REQUESTS = 10000

def main():
    response_times = []
    for i in range(NUM_REQUESTS):
        start = time.time()
        try:
            response = requests.get(SERVER_URL)
            response.raise_for_status()
        except Exception as e:
            print(f"Request {i+1} failed: {e}")
            continue
        end = time.time()
        elapsed = end - start
        response_times.append(elapsed)
        print(f"Request {i+1}: {(elapsed * 1000):.2f} milliseconds")
    if response_times:
        avg = sum(response_times) / len(response_times)
        print(f"Average response time: {(avg * 1000):.2f} milliseconds")
    else:
        print("No successful responses.")

if __name__ == "__main__":
    main()
