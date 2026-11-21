#!/usr/bin/env python3
"""
Нагрузочный тест локального API.
Использование:
    python3 loadtest.py --requests 10000 --concurrency 200
    python3 loadtest.py --duration 30 --concurrency 200
"""
import argparse
import http.client
import json
import time
import statistics
from concurrent.futures import ThreadPoolExecutor, as_completed


HOST = "localhost"
PORT = 8080
PATH = "/h1/task/"
BODY = json.dumps({"left": 2, "right": 8, "operation": "^"}).encode()
HEADERS = {"Content-Type": "application/json"}


def one_request():
    """Один POST. Возвращает (status, latency_ms, size)."""
    start = time.perf_counter()
    try:
        conn = http.client.HTTPConnection(HOST, PORT, timeout=10)
        conn.request("POST", PATH, body=BODY, headers=HEADERS)
        resp = conn.getresponse()
        data = resp.read()
        conn.close()
        latency = (time.perf_counter() - start) * 1000
        return resp.status, latency, len(data)
    except Exception as e:
        latency = (time.perf_counter() - start) * 1000
        return f"ERR:{type(e).__name__}", latency, 0


def run(n_requests, concurrency):
    latencies = []
    statuses = {}
    errors = 0
    t0 = time.perf_counter()

    with ThreadPoolExecutor(max_workers=concurrency) as pool:
        futures = [pool.submit(one_request) for _ in range(n_requests)]
        done = 0
        for fut in as_completed(futures):
            status, latency, _size = fut.result()
            latencies.append(latency)
            if isinstance(status, str):
                errors += 1
                statuses[status] = statuses.get(status, 0) + 1
            else:
                statuses[status] = statuses.get(status, 0) + 1
            done += 1
            if done % 500 == 0:
                print(f"  ... {done}/{n_requests}", flush=True)

    elapsed = time.perf_counter() - t0
    latencies.sort()

    print("\n===== RESULT =====")
    print(f"Requests:      {n_requests}")
    print(f"Concurrency:   {concurrency}")
    print(f"Errors:        {errors}")
    print(f"Elapsed:       {elapsed:.2f} s")
    print(f"RPS:           {n_requests / elapsed:.1f}")
    print(f"Latency min:   {latencies[0]:.1f} ms")
    print(f"Latency p50:   {latencies[len(latencies)//2]:.1f} ms")
    print(f"Latency p95:   {latencies[int(len(latencies)*0.95)]:.1f} ms")
    print(f"Latency p99:   {latencies[int(len(latencies)*0.99)]:.1f} ms")
    print(f"Latency max:   {latencies[-1]:.1f} ms")
    print(f"Mean:          {statistics.mean(latencies):.1f} ms")
    print(f"Status codes:  {statuses}")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--requests", type=int, default=5000)
    ap.add_argument("--concurrency", type=int, default=100)
    args = ap.parse_args()
    run(args.requests, args.concurrency)