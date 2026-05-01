# Project Logbook

This logbook is a chronological record of what I worked on, what I found, and what I will do next.
It complements the `README.md` (which explains what the project is and how to run it).

## How to use this logbook

- Add a new entry for each work session (daily or per meaningful chunk of work).
- Keep entries short and factual.
- Link to evidence where possible (RViz screenshots, TF tree images, `rosbag` names, terminal outputs).
- When you finish a milestone item, reference the exact command/config used so it is reproducible.

## Legend

- **Goal**: What I intended to achieve in this session.
- **Work done**: What I actually did (commands run, files changed, steps taken).
- **Results**: What worked/failed and what I observed (with metrics if available).
- **Evidence**: Screenshots, logs, bag files, links, or filenames.
- **Blockers**: Anything preventing progress.
- **Next**: The very next actions to take.

---

## 2026-05-01 — Project bootstrap

- **Goal**
  - Create repository scaffolding and define a clear plan/milestones.

- **Work done**
  - Created initial `README.md` documenting scope, requirements, and milestones.
  - Set up a TODO list for verifying JetRover ROS/LiDAR details.

- **Results**
  - Repository has a clear problem statement and success criteria.

- **Evidence**
  - `README.md`

- **Blockers**
  - None.

- **Next**
  - Verify JetRover ROS distro and LiDAR model from official docs / device outputs.

---

## Template — copy/paste for new entries

## YYYY-MM-DD — <short title>

- **Goal**
  - ...

- **Context**
  - Robot / environment: ...
  - Software state: ROS distro, branch/commit, machine (Jetson/host): ...

- **Work done**
  - Commands run:
    - `...`
  - Config changes:
    - `...` (file + what changed)
  - Notes:
    - ...

- **Results**
  - Expected:
    - ...
  - Observed:
    - ...

- **Evidence**
  - Screenshots: `docs/images/...`
  - TF frames output: `...`
  - Bag files: `...`
  - Map outputs: `maps/...`

- **Blockers / Issues**
  - ...

- **Next**
  - ...

