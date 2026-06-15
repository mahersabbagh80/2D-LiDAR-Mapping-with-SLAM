# Cross-Machine ROS 2 DDS Communication

## Setup

| Machine | Role | IP | OS | ROS |
|---|---|---|---|---|
| PC | Operator (RViz, CLI) | 192.168.2.102 | Ubuntu 22.04 | Humble |
| JetRover (Jetson Orin Nano) | Robot (all nodes) | 192.168.2.138 | Ubuntu 22.04 | Humble |

Both machines are on the same WiFi network. ROS 2 middleware: FastDDS (default for Humble).

---

## The Problem

ROS 2 uses DDS (Data Distribution Service) as its communication layer. By default, DDS discovers other participants on the network using **UDP multicast** — it broadcasts "I exist!" to the entire subnet and waits for replies.

**The router blocks multicast traffic between WiFi clients.** This is common on consumer and institutional routers. As a result:

- `ros2 node list` on the PC shows nothing from the Jetson
- `ros2 topic list` on the PC shows nothing
- RViz cannot see the map, scan, or any robot data

---

## Root Causes (in order of discovery)

### 1. Multicast is blocked

The router does not forward DDS multicast packets between WiFi clients. Without multicast, nodes on different machines are completely invisible to each other.

**Fix:** Configure FastDDS to use **unicast** discovery by specifying the other machine's IP in `initialPeersList`.

### 2. FastDDS only probes a small participant ID range by default

Each ROS 2 process gets a DDS **participant ID**, and listens for discovery on a specific UDP port:

```
metatraffic unicast port = 7410 + (2 × participant_ID)
```

So participant 0 → port 7410, participant 1 → port 7412, and so on.

When you add a unicast peer (just an IP address, no port), FastDDS only probes participant IDs **0–3** by default — ports 7410 to 7416. With 13 nodes running on the Jetson, most get IDs above 3 and are completely invisible.

**This is why you can see some nodes but not all** — only the first 4 participants (lowest IDs) are discovered.

**Fix:** Explicitly list all ports you want to probe in `initialPeersList`, covering participant IDs 0–99.

### 3. SEDP matching delay

After SPDP (participant discovery) completes, DDS must run SEDP (endpoint discovery) to match publishers with subscribers. This SEDP exchange takes 5–15 seconds after nodes start.

**This is why `ros2 topic echo /scan --once` with a short timeout appears to hang** — it's not that data doesn't flow, it just hasn't matched yet.

---

## The Solution

### FastDDS XML Profile

Each machine needs a `~/fastdds.xml` file. The profile disables multicast reliance and explicitly probes 100 participant IDs on the other machine.

**Jetson** (`~/fastdds.xml`):
```xml
<?xml version="1.0" encoding="UTF-8" ?>
<profiles xmlns="http://www.eprosima.com/XMLSchemas/fastRTPS_Profiles">
    <participant profile_name="unicast_connection" is_default_profile="true">
        <rtps>
            <builtin>
                <initialPeersList>
                    <locator><udpv4><address>192.168.2.102</address><port>7410</port></udpv4></locator>
                    <locator><udpv4><address>192.168.2.102</address><port>7412</port></udpv4></locator>
                    <!-- ... ports 7414 through 7608 (participant IDs 0–99) ... -->
                </initialPeersList>
            </builtin>
        </rtps>
    </participant>
</profiles>
```

**PC** (`~/fastdds.xml`) — same structure, with `192.168.2.138` as the address.

The full files (with all 100 ports) are already deployed on both machines. To regenerate them:

```python
def gen_xml(target_ip, max_id=100):
    locs = ''.join(
        f'<locator><udpv4><address>{target_ip}</address><port>{7410+2*i}</port></udpv4></locator>\n'
        for i in range(max_id)
    )
    return f"""<?xml version="1.0" encoding="UTF-8" ?>
<profiles xmlns="http://www.eprosima.com/XMLSchemas/fastRTPS_Profiles">
    <participant profile_name="unicast_connection" is_default_profile="true">
        <rtps>
            <builtin>
                <initialPeersList>
                    {locs}
                </initialPeersList>
            </builtin>
        </rtps>
    </participant>
</profiles>"""

# Jetson: target_ip = 192.168.2.102 (PC)
# PC:     target_ip = 192.168.2.138 (Jetson)
```

### Environment Variables

Both machines need these set when any ROS 2 process starts:

```bash
export FASTRTPS_DEFAULT_PROFILES_FILE=~/fastdds.xml
export ROS_DOMAIN_ID=0
```

On the **Jetson**: `scripts/run_mapping.sh` exports both before launching the mapping stack; keep the shell startup file synchronized if you run ad-hoc ROS CLI commands outside the script.
On the **PC**: this is set in `~/.zshrc`.

---

## Usage

On the **Jetson** (open a shell after building and sourcing `/home/ubuntu/my_projects/mapping_ws/install/setup.zsh`):

```bash
zsh "$(ros2 pkg prefix two_d_lidar_mapping_with_slam)/share/two_d_lidar_mapping_with_slam/scripts/run_mapping.sh"
```

The run script exports `FASTRTPS_DEFAULT_PROFILES_FILE=~/fastdds.xml`, stops the HiWonder auto-start service, clears stale ROS nodes from previous sessions, sources the vendor and mapping workspaces, restarts the ROS 2 daemon so it uses the FastDDS profile, and launches `mapping.launch.py`.

On the **PC** (open a terminal after setting `FASTRTPS_DEFAULT_PROFILES_FILE=~/fastdds.xml` and `ROS_DOMAIN_ID=0`):

```bash
# If this package is built and sourced on the PC:
rviz2 -d "$(ros2 pkg prefix two_d_lidar_mapping_with_slam)/share/two_d_lidar_mapping_with_slam/config/rviz/mapping.rviz"

# Or, from a checkout of this repository:
rviz2 -d config/rviz/mapping.rviz
```

RViz should discover all topics within ~15 seconds. The committed layout already has:

- fixed frame `map`
- RobotModel from `/robot_description` with Transient Local durability
- Map display on `/map` with updates from `/map_updates`
- LaserScan display on `/scan` with Best Effort reliability

If RViz starts before SLAM publishes `map`, it may temporarily report `Frame [map] does not exist`; this clears once `slam_toolbox` publishes the first map transform.

---

## Diagnostics

**Check if PC sees Jetson's topics:**
```bash
ros2 topic list
```

**Check if scan data is flowing:**
```bash
timeout 15 ros2 topic echo /scan --once
```
(Use at least 8 seconds — SEDP matching takes time on first connection.)

**Check if the robot model is available to late-joining RViz:**
```bash
timeout 15 ros2 topic echo /robot_description --once
```

**Check subscription count for a topic:**
```bash
ros2 topic info /scan --verbose
```
Subscription count should increment when a new subscriber starts.

**Capture DDS traffic on Jetson:**
```bash
sudo /usr/bin/tcpdump -i wlan0 -n 'udp and src host 192.168.2.102' | head -20
```

---

## What Does NOT Work

| Approach | Why it fails |
|---|---|
| Default DDS (no profile) | Router blocks multicast |
| `initialPeersList` with IP only (no port) | Only probes IDs 0–3, misses most nodes |
| `useBuiltinTransports=false` | Removes standard RTPS port binding; discovery breaks |
| `metatrafficUnicastLocatorList` without a port | Advertises port 0 (invalid), SEDP never reaches peers |
| `initialAnnouncements` XML tag | Invalid in FastDDS 2.6 — silently invalidates the entire profile |
| Short timeouts when testing (< 6s) | SEDP matching takes 5–15s; looks like failure but isn't |

---

## Why 100 Participants?

With 13 Jetson nodes, participant IDs reach up to ~14. The 100-participant range (ports 7410–7608) covers this 6× over.

To exceed 100 participants you'd need 100 simultaneously running ROS 2 processes on one machine, which is far beyond any realistic robotics project. If it ever becomes necessary, regenerate the `fastdds.xml` files with a larger `max_id`.
