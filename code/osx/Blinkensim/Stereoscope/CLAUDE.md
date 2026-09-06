# Stereoscope (iPhone)

A Blinkenlights installation viewer: renders Toronto City Hall (Nathan Phillips
Square) in OpenGL ES 1.1 and lights its windows from a live Blinkenlights proxy
feed. Originally written 2008–2010 by TheCodingMonkeys, last shipped against the
iOS 4.0 SDK. Restored in September 2026 to build and run on the iOS 26.5 SDK.

## Build and run

```sh
# Simulator
xcodebuild -project Stereoscope.xcodeproj -scheme Blinkenlights \
  -configuration Debug -sdk iphonesimulator \
  -destination 'generic/platform=iOS Simulator' \
  -derivedDataPath /tmp/dd CODE_SIGNING_ALLOWED=NO build

xcrun simctl boot "iPhone 16 Pro"
xcrun simctl install booted /tmp/dd/Build/Products/Debug-iphonesimulator/Stereoscope.app
xcrun simctl launch --console-pty booted de.codingmonkeys.stereoscope
```

- Target: `Blinkenlights` (product name `Stereoscope`), bundle id
  `de.codingmonkeys.stereoscope`.
- Configurations: `Debug`, `Release`, `Distrubtion` (sic — the typo is original).
  All three build clean for simulator and device.
- Deployment target iOS 15.0, arm64. `DEVELOPMENT_TEAM` is deliberately unset;
  set it in Xcode to run on a device.

## Layout

| Path | What |
| --- | --- |
| `Application.mm` | The renderer. Scene setup, camera, skybox, mesh drawing. `CShell` is the engine entry point. |
| `Classes/AppController.mm` | App delegate. Nib outlets, proxy discovery, frame queue, UI glue. |
| `Classes/BlinkenListener.m`, `bprotocol.h` | Blinken proxy UDP protocol client. |
| `Classes/SettingsController.m` | Settings table (feed selection). |
| `Classes/BlinkenhallAppDelegate.m` | Dead code — not in the target. |
| `Classes/RootViewController.m` | Dead code — not in the target. |
| `Engine/` | Oolong engine + PowerVR SDK tools (2008 vintage). |
| `Media/stereoscope.h` | The city hall mesh, exported as a C header by `ModelPODWriteH.cpp`. |
| `Media/*.pvr` | Skybox faces, building textures. |

Memory management is **manual retain/release throughout**. ARC is off and must
stay off — do not enable it piecemeal.

## What the 2026 restoration changed

### Nibs had to be rewritten
`ibtool` refuses IB3-era documents outright: *"Compiling IB documents for earlier
than iOS 7 is no longer supported."* There is no upgrade path in modern Xcode —
bumping `IBDocument.SystemTarget` does not help. All four nibs
(`MainWindow`, `Resources-iPad/MainWindow-iPad`, `Settings`, `EditingView`) were
decoded from the legacy keyed-archive XML and rewritten in the current format,
carrying over every frame, autoresizing mask, color, font, image and connection.

**Gotcha, if you ever touch these:** a free-standing container view in a modern
XIB gets resized to IB's default device (393×852) unless it carries
`<freeformSimulatedSizeMetrics key="simulatedDestinationMetrics"/>`. Without it
the overlay controls silently land at y=809 in a 480pt-tall view and vanish off
screen. Every top-level container view in these nibs has that tag.

The nibs are *design canvases*, not view hierarchies: `AppController` pulls the
labels and buttons out by outlet and re-parents them onto the `EAGLView` in
`applicationDidFinishLaunching:`. The container views themselves are unused.

### 64-bit data layout — the bug that broke rendering
`Media/stereoscope.h` declared all mesh data as `const unsigned long[]`, and
`Texture.mm` had `typedef unsigned long U32` for PVR header parsing. Those are
4 bytes on armv7 and **8 on arm64**, so every array was double-size with every
other word zero. Result: shattered geometry, no textures. Fixed by using
`unsigned int` in `stereoscope.h`, `Texture.mm`, `DisplayTextdat.h` (already
spelled `A32BIT`) and in the POD writer so regenerated headers stay correct.

If you ever re-export the mesh, check the emitted type is 32-bit.

### Other source fixes
- `EAGLView.m` — the `UITouch` category read the private `_locationInWindow`
  ivars, gone since iOS 3.2. Now `[self locationInView:nil]`, which is what they
  held.
- `DisplayTextAPI.cpp` — `new () SDisplayTextAPI` (empty placement args) → `new`.
- `Streaming.h` — `#include "Memory.h"`; clang needs `SafeAlloc` visible at
  template-definition time, GCC did not.
- `AppController.mm` — declares `<NSXMLParserDelegate>`; sets
  `_window.rootViewController`, required by UIKit since iOS 6.
- `MathTable.h` is untouched: its 2's-complement hex trig tables trip C++11
  narrowing rules, suppressed project-wide with
  `OTHER_CPLUSPLUSFLAGS = -Wno-c++11-narrowing`.

### Project settings
`SDKROOT iphoneos4.0` → `iphoneos`; dropped `ARCHS_STANDARD_32_BIT`, the private
`GraphicsServices.framework` link, the `PrivateFrameworks` search path, and
`PREBINDING` / `GCC_THUMB_SUPPORT` / `GCC_ENABLE_FIX_AND_CONTINUE`.

### Info.plist
ATS exceptions for `blinkenlights.net` / `.de` (the stream list is plain HTTP),
and `UISupportedInterfaceOrientations` = portrait, restoring the non-rotating
behaviour iOS 2 gave for free.

## Known issues

### Skybox renders only half of each face
**Confirmed symptom, cause NOT found.** Reported as "the background box flickers
its triangles on manual rotation". Flat-colouring the six faces shows what is
actually happening: **every skybox quad rasterises only one of its two
triangles**, and the survivor is `(v0, v2, v3)` — the shape you would get if
vertex 1 read back as vertex 0. As the camera turns, the resulting wedge of
missing sky sweeps across the frame.

Ruled out, each with its own rebuild and screenshot:

| Hypothesis | Result |
| --- | --- |
| Uninitialised Back face — verts/UVs 8–11 are commented out in `CreateSkybox()` while `DrawSkybox()` draws all six | Uncommenting them changed nothing visible. Still worth fixing on its own merits (it really is a read of uninitialised `new[]` memory) but it is **not** this bug. |
| Backface culling / winding | `glDisable(GL_CULL_FACE)` gave a pixel-identical frame. |
| `GL_TRIANGLE_STRIP` being mishandled | `glDrawElements(GL_TRIANGLES, 6, …)` with explicit indices `{0,1,2, 2,1,3}`, and a static 6-vertex triangle list via `glDrawArrays(GL_TRIANGLES, 0, 6)`, both render identically. |
| Vertex stride | `0` and `sizeof(VERTTYPE)*3` behave the same. |
| Bad geometry or camera | Dumped both at runtime: a correct closed box (x,z ∈ ±150, y ∈ 0…300) with the camera at (0, 2, 40), provably inside it. |
| The `myglTranslate(-vCameraPosition…)` sign | Flipping it to `+` (the PowerVR original, and the conventional skybox idiom) made coverage *worse*. Reverted. |

Leading suspicion is the Simulator's OpenGL ES 1.1 emulation (GL over Metal).
The city hall mesh uses the same client-side vertex arrays and renders perfectly,
but it has no triangles spanning far outside the frustum the way the 300-unit
skybox walls do. **Try it on a real device before spending more time here** —
that single data point decides whether this is a real bug or a simulator
artifact.

Diagnostic recipe that made it legible, worth repeating: in `DrawSkybox()`,
`glDisable(GL_TEXTURE_2D)` and `glColor4f` a distinct colour per face, then
comment out `DrawMesh()` in `RenderScene()` so nothing else is on screen.

### Runs letterboxed at 320×480
There is no launch storyboard, so iOS runs the app in compatibility mode and
scales a 320×480 canvas up with black bars. This is deliberate and currently the
*correct* choice: `CAM_ASPECT` is a hardcoded `WIDTH/HEIGHT` (320/480) and the
overlay controls are positioned in that space. Going full-screen means reworking
the projection and re-laying-out the overlay — a real project, not a flag.

### The stream list endpoint is gone
`http://www.blinkenlights.net/config/blinkenstreams.xml` no longer resolves, so
the app sits on "Please Choose a Proxy". Use **ⓘ → Manual Feed Address → Other…**
to point it at a proxy directly. A copy of the old feed format is in
`blinkenstreams.xml` at the repo root.

### Deprecated-but-compiling API still in place
`UIAlertView` (`AppController.mm` ~862), `NSURLConnection`, `openURL:`,
`presentModalViewController:`, `beginAnimations:`. All still compile. The
`UIAlertView` path only fires on a `<message>` element from the server, so it
could not be exercised — it may well not display on modern iOS.

## Verified working

Splash, 3D scene with skybox and textures, overlay controls (status label, live
label, camera button, ⓘ), the Settings screen (banner header, grouped table,
Autoselect switch, Done button), and the EditingView nib including its
`navigationItem` outlet. No exceptions on launch.

## Odds and ends

- Stale `.svn/` directories from the original checkout are still present.
- `Engine/Bullet/` is vendored but not compiled into the target.
- Do not let `xcodebuild` default its `SYMROOT` into the repo — pass
  `-derivedDataPath` or you get an untracked `build/` directory.
