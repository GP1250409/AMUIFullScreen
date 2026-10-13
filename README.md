# AMUIFullScreen

A jailbreak tweak that makes the system **"Automation Running"** overlay fill the whole screen on
iPhone Plus models (6/7/8 Plus, 414×736 pt).

## Problem

On an iPhone 8 Plus (iOS 16.7) the XCUITest automation-mode overlay drawn by
`AutomationModeUI.app` (`com.apple.dt.AutomationModeUI`) covers only 360×640 pt of the
414×736 pt screen. It is a system process, so it is unaffected by the automation tool's own
Info.plist (adding `UILaunchScreen` to the runner changes nothing).

Cause, measured with a probe tweak: the overlay root layer is sized and centred for the
1080×1920 physical panel instead of the 1242×2208 render size:

| | observed | should be |
|---|---|---|
| layer bounds | 360 × 640 | 414 × 736 |
| layer position | (540, 960) | (621, 1104) |

## What it does

Injected only into `com.apple.dt.AutomationModeUI`, it hooks `-[CALayer setBounds:]`,
`-[CALayer setFrame:]` and `-[CALayer setPosition:]`, and replaces exactly those wrong
values with the real display size. Anything else is passed through untouched.

## Install (rootless, Dopamine / ElleKit)

```sh
ldid -S AMUIFullScreen.dylib
cp AMUIFullScreen.dylib AMUIFullScreen.plist /var/jb/Library/MobileSubstrate/DynamicLibraries/
# restart the overlay process; launchd relaunches it on demand
kill -9 $(pgrep AutomationModeUI)
```

Uninstall: delete the two files from `DynamicLibraries/` and restart the process.
The sealed system volume is never modified.

## Notes

- Tested on iPhone 8 Plus, iOS 16.7.16, Dopamine. Overlay is full-screen after the fix.
- Constants assume a 3× Plus panel (1080×1920 panel, 1242×2208 render). Other models
  are not affected, but are untested.
- Killing `AutomationModeUI` while an automation session is running can drop that session;
  restart the automation tool afterwards.
