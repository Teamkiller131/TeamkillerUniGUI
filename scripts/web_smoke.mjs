// Headless-browser render smoke for the wasm web_demo artifacts (roadmap H4a).
//
// Loads web_demo.html in headless Chromium (Playwright), waits for the app to boot,
// screenshots the page, and asserts the UI actually DREW pixels — the browser twin of
// the native UNIGUI_RENDER_VERIFY gate (the 4.3.1 black screen passed every log-only
// signal; pixels don't lie). Also fails on page errors and wasm aborts.
//
// Usage:
//   node scripts/web_smoke.mjs --dir build_web/examples/web_demo [--webgpu]
//                              [--out smoke.png] [--timeout 45000]
//
// Deps (installed --no-save in CI; not part of the library):
//   npm install --no-save playwright pngjs && npx playwright install --with-deps chromium
//
// Exit codes: 0 = drew; 1 = blank/aborted/failed to boot; 2 = bad invocation.

import http from "node:http";
import { readFile } from "node:fs/promises";
import { extname, join, resolve } from "node:path";
import { createRequire } from "node:module";

const require = createRequire(import.meta.url);
const { chromium } = require("playwright");
const { PNG } = require("pngjs");

// ── args ─────────────────────────────────────────────────────────────────────
const args = process.argv.slice(2);
const getArg = (name, dflt) => {
    const i = args.indexOf(name);
    return i >= 0 && i + 1 < args.length ? args[i + 1] : dflt;
};
const dir = getArg("--dir", null);
const out = getArg("--out", "web_smoke.png");
const timeoutMs = parseInt(getArg("--timeout", "45000"), 10);
const webgpu = args.includes("--webgpu");
if (!dir) {
    console.error("usage: node scripts/web_smoke.mjs --dir <artifact dir> [--webgpu] [--out f.png]");
    process.exit(2);
}

// ── tiny static server (wasm needs the right MIME for streaming compile) ─────
const MIME = {
    ".html": "text/html; charset=utf-8",
    ".js": "text/javascript; charset=utf-8",
    ".wasm": "application/wasm",
    ".data": "application/octet-stream",
};
const root = resolve(dir);
const server = http.createServer(async (req, res) => {
    try {
        const path = join(root, decodeURIComponent(new URL(req.url, "http://x").pathname));
        const body = await readFile(path);
        res.writeHead(200, { "Content-Type": MIME[extname(path)] ?? "application/octet-stream" });
        res.end(body);
    } catch {
        res.writeHead(404);
        res.end("not found");
    }
});
await new Promise((ok) => server.listen(0, "127.0.0.1", ok));
const url = `http://127.0.0.1:${server.address().port}/web_demo.html`;
console.log(`[smoke] serving ${root} at ${url} (webgpu=${webgpu})`);

// ── browser ──────────────────────────────────────────────────────────────────
// SwiftShader ANGLE is the deterministic software-GL path: headless Chromium's default
// GL virtualization produced cross-context INVALID_OPERATION errors + a black canvas on
// Windows while the same artifact rendered fine headed — swiftshader renders identically
// to a real GPU everywhere, which is exactly what a CI gate needs.
const browserArgs = webgpu
    ? ["--enable-unsafe-webgpu", "--use-webgpu-adapter=swiftshader", "--enable-features=Vulkan"]
    : ["--use-angle=swiftshader"];
// Extra chromium switches for environment tuning (e.g. headless GL backends):
//   SMOKE_EXTRA_ARGS="--use-angle=swiftshader" node scripts/web_smoke.mjs ...
if (process.env.SMOKE_EXTRA_ARGS)
    browserArgs.push(...process.env.SMOKE_EXTRA_ARGS.split(/\s+/).filter(Boolean));
const browser = await chromium.launch({ args: browserArgs, headless: !args.includes("--headed") });
const page = await browser.newPage({ viewport: { width: 1280, height: 720 } });

let fatal = null;
let booted = false;
page.on("console", (msg) => {
    const text = msg.text();
    console.log(`[console:${msg.type()}] ${text.slice(0, 300)}`);
    if (/Init complete|render-verify|Active backend/.test(text)) booted = true;
    if (/Aborted\(|RuntimeError|out of memory|failed to (compile|instantiate)/i.test(text))
        fatal = `wasm abort in console: ${text.slice(0, 200)}`;
});
page.on("pageerror", (err) => (fatal = `pageerror: ${String(err).slice(0, 200)}`));

let verdictFail = null;
try {
    await page.goto(url, { waitUntil: "domcontentloaded", timeout: timeoutMs });
    // Wait for the app to report it booted (spdlog → console), then let frames settle.
    const deadline = Date.now() + timeoutMs;
    while (!booted && !fatal && Date.now() < deadline)
        await page.waitForTimeout(250);
    if (!booted && !fatal)
        console.log("[smoke] no boot log seen — proceeding to pixel check anyway");
    await page.waitForTimeout(3000); // several RAF frames

    if (fatal) throw new Error(fatal);

    // ── pixel verdict on the CANVAS ELEMENT ONLY (the emscripten shell's page chrome
    //    — logo, checkboxes, Fullscreen button — must not be able to fake a "drawn"
    //    verdict; the first local run proved a full-page screenshot passes on a black
    //    canvas). Dominant sampled color = background; count what differs. ──
    const canvas = page.locator("#canvas");
    const shot = await canvas.screenshot({ path: out });
    const png = PNG.sync.read(shot);
    const counts = new Map();
    const samples = [];
    const step = 8;
    for (let y = 0; y < png.height; y += step) {
        for (let x = 0; x < png.width; x += step) {
            const i = (png.width * y + x) << 2;
            // Quantize to 4 bits/channel so AA/gradients cluster with their base color.
            const key =
                ((png.data[i] >> 4) << 8) | ((png.data[i + 1] >> 4) << 4) | (png.data[i + 2] >> 4);
            counts.set(key, (counts.get(key) ?? 0) + 1);
            samples.push(key);
        }
    }
    let bgKey = -1, bgCount = -1;
    for (const [k, c] of counts) if (c > bgCount) { bgKey = k; bgCount = c; }
    const total = samples.length;
    const nonBg = total - bgCount;
    const distinct = counts.size;
    const frac = nonBg / total;
    console.log(
        `[smoke] samples=${total} background=0x${bgKey.toString(16)} nonBackground=${nonBg} ` +
        `(${(frac * 100).toFixed(1)}%) distinctColors=${distinct}`);

    // A rendered widget-gallery UI produces plenty of non-background structure; a black
    // screen / bare clear produces almost none. Thresholds are deliberately generous.
    if (nonBg < 200 || frac < 0.02 || distinct < 8)
        verdictFail = `canvas looks blank (nonBackground=${nonBg}, ${(frac * 100).toFixed(1)}%, distinct=${distinct})`;
} catch (e) {
    verdictFail = String(e && e.message ? e.message : e);
} finally {
    await browser.close();
    server.close();
}

if (verdictFail) {
    console.error(`[smoke] FAIL: ${verdictFail} (screenshot: ${out})`);
    process.exit(1);
}
console.log(`[smoke] PASS: the wasm UI rendered real pixels (screenshot: ${out})`);
process.exit(0);
