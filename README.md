# RealTime Log Analyser

## Overview

RealTime Log Analyser is a full-stack observability prototype combining a C++ analytics backend with a React/Vite frontend dashboard. The backend receives JSON log events, processes them using streaming algorithms, stores summary metrics in SQLite, and exposes HTTP analytics APIs. The frontend visualizes visitor trends, top domains, and suspicious IP activity.

## System Architecture

- **Traffic ingestion**: The backend container runs a Python traffic generator (`backend/scripts/generate_traffic.py`) that streams synthetic log events into the C++ processing pipeline.
- **C++ analytics backend**: The backend listens on `0.0.0.0:8080`, accepts log events on `/logs`, and exposes dashboard APIs under `/api/*`.
- **Data storage**: A local SQLite store maintains aggregated metrics and security insights.
- **Frontend dashboard**: The React/Vite app runs on `5173` and fetches analytics data from the backend.
- **Docker Compose**: `docker-compose.yml` orchestrates the backend and frontend containers together.

### Backend responsibilities

- Receives streaming log payloads via `POST /logs`
- Updates in-memory analytics using probabilistic data structures:
  - `BloomFilter` for suspicious IP detection
  - `CountMinSketch` for frequency estimation
  - `HyperLogLog` for approximate unique visitor counting
  - `SpaceSaving` for identifying top domains
- Persists hourly aggregates and top domain data to SQLite
- Serves JSON dashboard endpoints:
  - `GET /api/visitors`
  - `GET /api/domains`
  - `GET /api/suspicious`

### Frontend responsibilities

- Runs a modern Vite development server
- Uses React 19 for UI composition
- Uses Recharts for charting and visualizations
- Fetches analytics data from the backend APIs and renders the dashboard in-browser

## Tech Stack

- Backend
  - C++17
  - CMake build system
  - SQLite3 for local storage
  - `cpp-httplib` for lightweight HTTP server routing
  - `nlohmann::json` for JSON serialization
  - Python 3 for synthetic traffic generation
- Frontend
  - React 19
  - Vite development server
  - Recharts for charts
  - Lucide React for iconography
  - ESLint for code quality
- Containerization
  - Docker
  - Docker Compose

## Getting Started

### Prerequisites

- Docker installed
- Docker Compose available

### Run the full stack

From the repository root:

```bash
docker-compose up --build
```

This will:

1. Build the backend image from `backend/Dockerfile`
2. Build the frontend image from `frontend/Dockerfile`
3. Start the backend service on port `8080`
4. Start the frontend service on port `5173`

### Access the app

- Frontend dashboard: `http://localhost:5173`
- Backend analytics API: `http://localhost:8080`

### Notes

- The backend Docker container launches the Python traffic generator and pipes events into the compiled binary.
- The frontend depends on the backend and is configured to communicate with it over the exposed ports.

## Project Structure

- `backend/`
  - `CMakeLists.txt` — backend build definition
  - `Dockerfile` — backend container build
  - `include/` — analytics and service headers
  - `src/` — C++ implementation files
  - `scripts/generate_traffic.py` — synthetic log producer
  - `third_party/httplib.h` — embedded HTTP server library
- `frontend/`
  - `Dockerfile` — frontend container build
  - `package.json` — NPM dependencies and scripts
  - `src/` — React application source

## Recommended Workflow

- Use `docker-compose up --build` for a full local environment
- Keep the backend running on `8080` and access the dashboard at `5173`
- If you need a fresh rebuild, stop Compose and rerun with `--build`
