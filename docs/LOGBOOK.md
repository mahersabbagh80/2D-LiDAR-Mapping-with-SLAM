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
  - Created `docs/LOGBOOK.md` so the README stays focused on “what/how”, while the logbook captures day-to-day progress.
  - Initialized git, created the initial commit, and connected/pushed the repository to GitHub.
  - Noted SSH vs HTTPS remote choice; attempted switching `origin` to SSH and diagnosed `Permission denied (publickey)` as a missing/unused key file in WSL (deferred; HTTPS is acceptable).
  - Shared how to invite collaborators so the tutor can access the repo.

- **Results**
  - Repository has a clear problem statement and success criteria.
  - Repo is on GitHub and ready for review/collaboration.

- **Evidence**
  - `README.md`
  - `docs/LOGBOOK.md`

- **Blockers**
  - None.

- **Next**
  - Verify JetRover ROS distro and LiDAR model from official docs / device outputs.

---

## 2026-05-02 — Architecture diagram + README corrections

- **Goal**
  - Refine project documentation and capture the end-to-end mapping pipeline at a high level.

- **Work done**
  - Updated `README.md` with details based on the official JetRover Orin Nano mapping tutorial.
  - Corrected the robot/software info: JetRover uses **ROS 2 Humble** (not ROS Noetic).
  - Added a system architecture/pipeline diagram and iterated on it to fix mistakes and wording:
    - `docs/images/pipeline.svg`
    - `docs/images/pipeline.drawio` (source)
  - Verified that GitHub showing an old embedded image can be a browser/CDN cache issue (hard refresh / private window resolves).

- **Results**
  - Documentation and diagram are aligned with the JetRover tutorial and reflect the ROS 2 Humble stack.
  - Diagram assets are stored both as an editable source (`.drawio`) and a rendered export (`.svg`).

- **Evidence**
  - `README.md`
  - `docs/images/pipeline.svg`
  - `docs/images/pipeline.drawio`

- **Blockers**
  - None.

- **Next**
  - Do more background research, validate and collect the papers, and tutorials used for this project.
  - Start validating the pipeline on the actual robot: confirm sensor topics/frames, SLAM package choice, and reproducible “bringup → mapping → save map” steps.

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

