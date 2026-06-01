
import random
import json

NUM_REQUESTS = 5000
DOMAINS = ["google.com", "github.com", "stackoverflow.com", "wikipedia.org", "reddit.com"]
WEIGHTS = [0.4, 0.35, 0.15, 0.05, 0.05] 
MALICIOUS_IPS = ["192.168.1.50", "10.99.99.99"]
NORMAL_IPS = [f"192.168.1.{i}" for i in range(100, 200)] + [f"10.0.0.{i}" for i in range(1, 50)]

def main():
    for i in range(NUM_REQUESTS):
        is_malicious = random.random() < 0.02
        user_ip = random.choice(MALICIOUS_IPS) if is_malicious else random.choice(NORMAL_IPS)
        domain = random.choices(DOMAINS, weights=WEIGHTS, k=1)[0]
        hour = random.randint(0, 23)
        timestamp = f"2026-05-30T{hour:02d}:00:00Z"

        payload = {
            "website_domain": domain,
            "user_ip": user_ip,
            "timestamp": timestamp
        }
        # Print the raw JSON string directly to standard output
        print(json.dumps(payload))

    # Send a special command at the end to tell C++ to flush the database
    print("FLUSH")

if __name__ == "__main__":
    main()
