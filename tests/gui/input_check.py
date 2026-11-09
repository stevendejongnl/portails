#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Drives a real portails window with xdotool and checks scrolling, text selection and the clipboard.

It starts portails with its own data folder and a tab that shows a local test page, so the
tabs of the user are not touched. The page reports what it saw to this script over HTTP.

Usage: tests/gui/input_check.py [path/to/portails]   (needs X11, xdotool, python3)
"""
import http.server
import json
import os
import subprocess
import sys
import tempfile
import threading
import time

SIDEBAR_WIDTH = 220
PAGE = """<!doctype html><meta charset=utf-8><style>
body{margin:0;background:#fff;color:#000;font:24px sans-serif;height:6000px}
#copy{position:absolute;left:20px;top:20px;width:200px;height:50px}
#text{position:absolute;left:20px;top:120px;width:600px}
</style>
<button id=copy>Copy link</button>
<p id=text>alpha bravo charlie delta echo foxtrot golf hotel india juliet kilo lima</p>
<script>
const report = (what, value) => fetch("/report", {method: "POST", body: JSON.stringify({what, value})});
document.getElementById("copy").onclick = async () => {
  try { await navigator.clipboard.writeText("copied-by-page"); report("copy", "ok"); }
  catch (error) { report("copy", "failed: " + error.name); }
};
const textBox = document.getElementById("text").getBoundingClientRect();
report("textbox", [textBox.left, textBox.top, textBox.width, textBox.height].join(","));
addEventListener("mousedown", (event) => report("mousedown", event.clientX + "," + event.clientY));
addEventListener("scroll", () => report("scroll", scrollY));
document.addEventListener("selectionchange", () => report("selection", String(getSelection())));
report("loaded", innerWidth + "x" + innerHeight);
</script>"""

reports = []


class Handler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-Type", "text/html")
        self.end_headers()
        self.wfile.write(PAGE.encode())

    def do_POST(self):
        reports.append(json.loads(self.rfile.read(int(self.headers["Content-Length"]))))
        self.send_response(204)
        self.end_headers()

    def log_message(self, *_):
        pass


def xdo(*args):
    return subprocess.run(["xdotool", *args], capture_output=True, text=True, check=True).stdout.strip()


def last(what):
    values = [r["value"] for r in reports if r["what"] == what]
    return values[-1] if values else None


def wait_for(what, timeout=20):
    end = time.time() + timeout
    while time.time() < end:
        if last(what) is not None:
            return last(what)
        time.sleep(0.2)
    return None


def main():
    binary = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else "build/portails")
    server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), Handler)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    url = f"http://127.0.0.1:{server.server_port}/"

    home = tempfile.mkdtemp(prefix="portails-gui-")
    data = os.path.join(home, "data", "portails")
    os.makedirs(data)
    tabs = {"tabs": [{"id": "t1", "name": "test", "platform_id": "local-test", "url": url}], "active_id": "t1"}
    with open(os.path.join(data, "tabs.json"), "w") as file:
        json.dump(tabs, file)
    env = dict(os.environ, XDG_DATA_HOME=os.path.join(home, "data"), XDG_RUNTIME_DIR=home)
    app = subprocess.Popen([binary], env=env, stderr=subprocess.DEVNULL)
    results = {}
    try:
        if wait_for("loaded") is None:
            print("FAIL: the test page never loaded")
            return 1
        time.sleep(2)  # the window must be mapped and settled before it can be focused
        window = xdo("search", "--sync", "--onlyvisible", "--pid", str(app.pid)).splitlines()[0]
        subprocess.run(["xdotool", "windowfocus", window], check=False)
        time.sleep(1)

        origin = [SIDEBAR_WIDTH, 0]

        def at(x, y):
            xdo("mousemove", "--window", window, str(int(origin[0] + x)), str(int(origin[1] + y)))

        textbox = last("textbox")
        # Learn the offset between window coordinates and page coordinates with one click.
        at(300, 300)
        xdo("click", "1")
        time.sleep(0.5)
        seen_x, seen_y = (float(v) for v in last("mousedown").split(","))
        offset_x, offset_y = SIDEBAR_WIDTH + 300 - seen_x, 300 - seen_y
        print(f"page origin is at window ({offset_x:.0f}, {offset_y:.0f})")
        origin[:] = [offset_x, offset_y]

        # Scrolling: five wheel notches over the page.
        at(300, 300)
        for _ in range(5):
            xdo("click", "5")
            time.sleep(0.1)
        time.sleep(1)
        scrolled = float(last("scroll") or 0)
        results["scroll"] = (scrolled > 0, f"scrollY={scrolled}")
        for _ in range(20):
            xdo("click", "4")
            time.sleep(0.1)
        time.sleep(2)
        results["scrolls back to top"] = (float(last("scroll") or 0) == 0, f"scrollY={last('scroll')}")
        text_left, text_top = (float(v) for v in textbox.split(",")[:2])
        line_y = text_top + 15

        # Selection: a double click selects a word, a drag selects a range.
        reports.clear()
        at(text_left + 20, line_y)
        xdo("click", "--repeat", "2", "--delay", "80", "1")
        time.sleep(0.5)
        results["double-click selects a word"] = (bool(last("selection")), repr(last("selection")))
        reports.clear()
        at(text_left + 400, text_top + 150)  # blank spot: clears the selection, so the drag starts a new one
        xdo("click", "1")
        time.sleep(0.3)
        at(text_left + 5, line_y)
        xdo("mousedown", "1")
        for step in range(1, 11):
            at(text_left + 5 + step * 40, line_y)
            time.sleep(0.05)
        xdo("mouseup", "1")
        time.sleep(0.5)
        selection = last("selection") or ""
        results["drag selects text"] = (len(selection) > 5, repr(selection))

        # Clipboard: the button calls navigator.clipboard.writeText.
        reports.clear()
        at(100, 45)
        xdo("click", "1")
        time.sleep(1)
        results["copy button"] = (last("copy") == "ok", str(last("copy")))

        # Developer tools: F12 opens a window of its own, and it is the same window the second time.
        def tools_windows():
            found = subprocess.run(["xdotool", "search", "--onlyvisible", "--name", "Developer tools"],
                                   capture_output=True, text=True)
            return found.stdout.split()

        at(300, 300)
        xdo("click", "1")
        xdo("key", "F12")
        time.sleep(3)
        results["F12 opens developer tools"] = (len(tools_windows()) == 1, f"windows={tools_windows()}")
        xdo("key", "--window", window, "ctrl+shift+i")
        time.sleep(1)
        results["Ctrl+Shift+I reuses the window"] = (len(tools_windows()) == 1, f"windows={tools_windows()}")
    finally:
        app.terminate()
        app.wait(15)
        server.shutdown()
    failed = False
    for name, (passed, detail) in results.items():
        print(f"{'PASS' if passed else 'FAIL'}  {name}: {detail}")
        failed |= not passed
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
