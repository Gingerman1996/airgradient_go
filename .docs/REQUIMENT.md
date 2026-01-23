@Nick @Samuel @Altair Irfan @guide 

Below my ideas for the firmware features for the AirGradient Go.

Goals

Not dependent on phone. Key functionality can be controlled on the moniyor, e.g. recording + GPS sessions + tagging + settings.

The Go is setup like a normal monitor, and could be continuously deployed stationary. At the same time, a user initiated route can be started which then enables higher frequency recoding and tagging possibilities. So from the data model, it would record like a normal monitor on the dashboard, opening the detailed view of the monitor would allow to see saved routes (sessions) on a map with details for that session (with sharing possibilities from the app).

1) UI

A) Main dashboard (single screen)

Show all values at once:

PM2.5 (large) - display mode: µg/m³ or USAQI

CO2 (ppm)

VOC (Index)

Temp (°C/°F)

RH (%)

Pressure (hPa)

No PM1/PM10, no trends, no graphs.

B) Status bar (top row, always visible)

1st Row: Left to right:

BLE icon (connected/not)

WiFi icon (off/connecting/connected)

GPS icon (off/search/fix)

Battery + % (bolt if charging)

2nd Row:

Recording: ● REC / PAUSE

Interval: e.g. 10s

Time HH:MM (or --:--)

! alert indicator (any fault/warm-up/storage/time issue)

2) Buttons (3 capacitive)

Left/Right: move focus between tiles (PM2.5, CO2, VOC, Temp, RH, Pressure). Focus means inverted section (white text, dark background)

Middle short: Quick Menu

Middle long: Start/Stop Session (confirm on stop)

Right long: Touch lock / unlock

Auto lock after 20s of inactivity

Quick Menu (navigate with buttons, middle selects):

Add Tag

Settings

Pause/Resume Recording

About

3) Recording (autonomous)

Local ring-buffer logging.

Interval options (MVP): 1s, 10s, 30s, 60s, 5m, 15m, 1h (default 5m).

Record fields: timestamp, PM2.5, CO2, VOC index, temp, RH, pressure, battery, power mode, sensor flags, GPS fix/accuracy (if enabled).

Ring buffer overwrites oldest when full. If storage high watermark hit - set !.

Implementation notes

UI refresh follows the recording interval (default 10s) after the test screen. Refresh should not be faster than needed on battery.

If interval is 1s, be mindful of storage + battery; still supported.

4) Sessions (GPS routes, autonomous)

Start/stop on device (Middle long).

During session:

sensors + GPS records at recording interval (same as global setting)

GPS points at 10s default (option: 1s / 10s / 30s / 60s)

After stop: show summary ~5-10s:

duration

PM2.5 avg/peak

CO2 avg/peak

tags count

GPS time sets RTC when fix available (default ON).

GPS modes (Settings):

OFF

SESSIONS ONLY (default)

HOME DETECT (for WiFi home sync)

ALWAYS

5) Tagging (on-device notes)

No free text. One tag per action. Store with timestamp + snapshot + session ID (if active) + GPS pos (if fix).

Categories (MVP)

A) Activity

run, walk, bike, commute, cooking, sleep, workout, working

B) Context

indoor, outdoor, car, bus, train/metro, motorbike, bicycle, office, home

C) Source (expanded)

traffic

smoke

wildfire smoke

incense

cooking

grilling/BBQ

cleaning products

paint/solvents

perfume/fragrance

candles

vaping/smoking

dust/construction

road dust

generator/exhaust

factory/industrial

garbage burning

fireworks

pesticides/spraying

humidity/steam (shower)

unknown

UI flow

Quick Menu -> Add Tag

Choose category -> choose value -> confirm

Optional: allow “last used” shortcut at top of list.

6) LEDs (simplified MVP - mostly off)

Rules

BAT mode: LEDs OFF normally.

LEDs on only for:

warnings (brief)

optional “touch preview” 2-3s

PWR mode: LEDs may be ON continuously (setting), otherwise warning-only.

Patterns (only 2)

Warning: short pulse (all LEDs) every 60s while condition persists

Critical: faster pulse (all LEDs) every 15s while condition persists

Warning triggers (fixed, not user-editable in MVP)

PM2.5: Warning > 35 µg/m³ (or USAQI > 100), Critical > 75 µg/m³ (or USAQI > 150)

CO2: Warning > 1200 ppm, Critical > 2000 ppm

VOC Index: Warning > 200, Critical > 400

Battery: Warning < 20%, Critical < 10%
If any critical active -> show Critical pattern.

7) Settings (on-device)

Minimal settings set (MVP):

Recording: On/Off, interval: 1s, 10s, 30s, 60s, 5m, 15m, 1h

Units: °C/°F

PM display: µg/m³ / USAQI

GPS mode: OFF / SESSIONS ONLY / HOME DETECT / ALWAYS

Session GPS interval: 1s / 10s / 30s / 60s

WiFi: On/Off

Auto-sync: Off / Home Zone only / Home Zone + PWR only (default: Home Zone + PWR only)

Home Zone radius: 200m / 500m / 1km (default 200m)

Data: storage used, clear data (confirm)

About: FW version, device ID, sensor status

Use existing PUT requests in fw?

WiFi credentials + Home Zone center are set via the app over BLE (MVP).

8) Home Zone WiFi auto-sync (AFTER MVP)

Home Zone = lat/lon + radius (set by app).

Auto-sync runs when:

WiFi enabled

Auto-sync enabled

inside Home Zone (GPS fix ok)

and (if configured) only on PWR

Upload incremental data since last success:

records, sessions, tags, session GPS points

On BAT, WiFi is duty-cycled (connect, upload, disconnect; timeout if slow).

Show in Settings:

last sync time + last result (OK/Fail).

9) BLE (MVP)

BLE must support:

device info + FW version

settings read/write (WiFi creds + home zone via app)

data sync (records since timestamp/cursor, sessions + tags)

time sync from app (fallback if GPS off)

Acceptance checklist

Dashboard + status bar as specified.

Interval list includes: 1s, 10s, 30s, 60s, 5m, 15m, 1h.

Expanded Source tags present and persisted.

Recording ring buffer works.

Sessions record GPS route and show summary.

LEDs off on battery except warning pulses; only 2 patterns.

WiFi auto-sync in Home Zone works and respects PWR/BAT setting.

BLE supports provisioning + data sync.
