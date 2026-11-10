import http.server
import socketserver
import webbrowser
import os
import sys

PORT = 8080
BUILD_WEB_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "build-web")
DIRECTORY = BUILD_WEB_DIR if os.path.exists(BUILD_WEB_DIR) else os.path.join(os.path.dirname(os.path.abspath(__file__)), "build_web")

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

    def end_headers(self):
        # Enable SharedArrayBuffer and cross-origin isolation if needed
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate')
        super().end_headers()

def main():
    if not os.path.exists(DIRECTORY):
        print(f"[Error] Directory '{DIRECTORY}' does not exist.")
        print("Please build the WebAssembly version first by running build_web.bat.")
        input("Press Enter to exit...")
        sys.exit(1)

    html_file = os.path.join(DIRECTORY, "DungeonReborn.html")
    if not os.path.exists(html_file):
        print(f"[Warning] 'DungeonReborn.html' not found in '{DIRECTORY}'.")
        print("Make sure compilation completed successfully.")

    Handler.extensions_map.update({
        '.wasm': 'application/wasm',
        '.js':   'application/javascript',
        '.data': 'application/octet-stream',
        '.html': 'text/html',
    })

    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        url = f"http://localhost:{PORT}/DungeonReborn.html"
        print("========================================================")
        print("   DUNGEON REBORN - LOCAL WEBASSEMBLY SERVER ACTIVE      ")
        print("========================================================")
        print(f" Serving at: {url}")
        print(" Press Ctrl+C in this terminal to stop the server.")
        print("========================================================")
        try:
            webbrowser.open(url)
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n[Server] Shutting down cleanly.")

if __name__ == "__main__":
    main()

