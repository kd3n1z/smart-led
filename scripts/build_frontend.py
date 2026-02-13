Import("env")

import os
import gzip
import subprocess
import sys

GUI_FOLDER = os.path.join(env["PROJECT_DIR"], "gui")
DIST_FOLDER = os.path.join(GUI_FOLDER, "dist")
OUTPUT_PATH = os.path.join(env["PROJECT_DIR"], "src/frontend.cpp")

MIME_TYPES = {
    ".html": "text/html",
    ".js": "application/javascript",
    ".css": "text/css",
    ".json": "application/json",
    ".svg": "image/svg+xml",
    ".png": "image/png",
    ".jpg": "image/jpeg",
    ".ico": "image/x-icon",
    ".woff": "font/woff",
    ".woff2": "font/woff2",
}

def file_to_gzip_bytes(path):
    with open(path, "rb") as f:
        return gzip.compress(f.read())

def sanitize_name(path):
    name = path.replace("\\", "_").replace("/", "_").replace(".", "_")
    return "".join(c if c.isalnum() or c == "_" else "_" for c in name)

def get_mime_type(filename):
    _, ext = os.path.splitext(filename)
    return MIME_TYPES.get(ext.lower(), "application/octet-stream")

def run_pnpm_build():
    if not os.path.isdir(GUI_FOLDER):
        print("[frontend] ERROR: folder not found")
        env.Exit(1)

    print("[frontend] running `pnpm run build`")

    try:
        subprocess.check_call(
            ["pnpm", "run", "build"],
            cwd=GUI_FOLDER,
            stdout=sys.stdout,
            stderr=sys.stderr,
        )
    except FileNotFoundError:
        print("[frontend] ERROR: pnpm not found in PATH")
        env.Exit(1)
    except subprocess.CalledProcessError as e:
        print("[frontend] ERROR: pnpm build failed")
        env.Exit(e.returncode)

def build_frontend(source, target, env):
    run_pnpm_build()

    if not os.path.isdir(DIST_FOLDER):
        print("[frontend] ERROR: dist not found after build, aborting")
        env.Exit(1)

    lines = [
        "// Automatically generated, do not touch",
        "",
        "#include <ESP8266WebServer.h>",
        "",
        "extern ESP8266WebServer server;",
        ""
    ]

    route_lines = ["void setupFrontend() {"]

    def append_route(rel_path, mime, var_name):
        route_lines.extend([
            f'    server.on("/{rel_path}", []() {{',
            f'        server.sendHeader("Content-Encoding", "gzip");',
            f'        server.sendHeader("Cache-Control", "max-age=86400");',
            f'        server.send(200, "{mime}", {var_name}, {var_name}_len);',
            f'    }});'
        ])

    for root, _, files in os.walk(DIST_FOLDER):
        for file in files:
            filepath = os.path.join(root, file)
            rel_path = os.path.relpath(filepath, DIST_FOLDER).replace("\\", "/")
            var_name = sanitize_name(rel_path)
            compressed = file_to_gzip_bytes(filepath)
            mime = get_mime_type(file)

            array_lines = []
            for i in range(0, len(compressed), 12):
                chunk = compressed[i:i + 12]
                array_lines.append(", ".join(f"0x{b:02x}" for b in chunk))

            array_content = ",\n    ".join(array_lines)

            lines.append(
                f"const uint8_t {var_name}[] PROGMEM = {{\n    {array_content}\n}};"
            )
            lines.append(
                f"const size_t {var_name}_len = sizeof({var_name});\n"
            )

            if rel_path == "index.html":
                append_route("", mime, var_name)

            append_route(rel_path, mime, var_name)

    route_lines.append("}")

    with open(OUTPUT_PATH, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))
        f.write("\n\n")
        f.write("\n".join(route_lines))

    print(f"[frontend] generated {OUTPUT_PATH}")

env.AddPreAction("$BUILD_DIR/${PROGNAME}.elf", build_frontend)
